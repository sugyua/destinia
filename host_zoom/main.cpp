#include "../common/safe_save.h"
#include "../common/virtual_controls.h"
#include "../common/ui_renderer.h"
#include "../common/drop_names.h"
#include "../common/loot_qol_runtime.h"
static destinia::LootQolRuntime lootQol;
#include "../common/boss_mage_runtime.h"
static destinia::BossMageRuntime bossMage;
#include "../common/difficulty_arm.h"
static destinia::DifficultyArmRuntime difficultyArm;
#include "../common/boss_rush_ui.h"
static destinia::BossRushUI bossRushUI;
static destinia::DropNamesRenderer dropNamesRenderer;
static destinia::UiRenderer uiRenderer;
// destinia_host - executa a libdestinia_jni.so ORIGINAL (ARM 32 bits) no PC.
//
// Papel: faz o que o Java (GameThread/GameMainView) fazia no Android.
//   - carrega a .so em um emulador de CPU (Unicorn)
//   - fornece as funcoes de C que ela importa (malloc, fopen, sprintf...)
//   - fornece um "JNI falso" (ler assets, tocar som...)
//   - chama initGame uma vez e jniRun a cada quadro; mostra o framebuffer
//
// Tudo que foi CONFIRMADO no APK esta marcado com [OK]; o que e suposicao
// esta marcado com [?] para voce poder procurar depois.
#define SDL_MAIN_HANDLED
#include <SDL.h>
#ifdef HAS_SDL_MIXER
#include <SDL_mixer.h>
#endif
#include <unicorn/unicorn.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include "../common/runtime_support.h"
#include "../common/elf_image.h"
#include "../common/player_controls.h"
using destinia::hostReport;

namespace fs = std::filesystem;
using u32 = uint32_t;

// ---------------------------------------------------------------- constantes
// [OK] resolucao e formato: GameThread passa 400x240 e um buffer de 192000
// bytes (400*240*2 = RGB565 little-endian) para initGame/jniRun.
// Zoom out: mais area do mapa e UI original proporcionalmente menor.
static const int SCREEN_W = 800, SCREEN_H = 480;
static const int WINDOW_W = 800, WINDOW_H = 480;
static const u32 FB_BYTES = SCREEN_W * SCREEN_H * 2;

// mapa de memoria do "celular" emulado
static const u32 SO_BASE    = 0x10000000;  // onde a .so e carregada
static const u32 HEAP_BASE  = 0x20000000;  // malloc
static const u32 HEAP_SIZE  = 0x04000000;  // 64 MB
static const u32 STUB_BASE  = 0x30000000;  // funcoes que o host implementa
static const u32 STUB_COUNT = 0x4000;      // 4 bytes cada
static const u32 MAGIC_RET  = 0x30100000;  // endereco de retorno das chamadas
static const u32 DATA_BASE  = 0x40000000;  // JNIEnv, JavaVM, stack guard
static const u32 DATA_SIZE  = 0x10000;
static const u32 STACK_BASE = 0x7F000000;
static const u32 STACK_SIZE = 0x00200000;

static const u32 ENV_OBJ = DATA_BASE + 0x0000, ENV_TAB = DATA_BASE + 0x0100;
static const u32 VM_OBJ  = DATA_BASE + 0x1000, VM_TAB  = DATA_BASE + 0x1100;
static const u32 GUARD   = DATA_BASE + 0x2000;

static const u32 ID_LIBC = 0x000, ID_JNI = 0x400, ID_VM = 0x800, ID_MISSING = 0xC00;
static const int JNI_FUNCS = 235;

// ---------------------------------------------------------------- estado
static uc_engine* uc = nullptr;
static bool verbose = false, fatal = false, hitMagic = false, executionFailed = false;
static destinia::FileLock saveLock;
static destinia::Preferences preferences;
static bool audioFocused=true, audioPaused=false;
static int nativeMusicVolume=5, nativeEffectsVolume=5;
static std::string gameDir = "../game", soPath, assetsDir, saveDir;
static u32 loadedSoSpan = 0;

static std::vector<std::function<void()>> handlers(STUB_COUNT);
static std::vector<std::string> stubName(STUB_COUNT);
static std::map<std::string, u32> importId;  // nome -> id do stub
static u32 nextMissing = ID_MISSING;
static std::vector<std::string> ring;        // ultimas chamadas (para debug)

static void note(const std::string& s) {
  ring.push_back(s);
  if (ring.size() > 40) ring.erase(ring.begin());
  destinia::sessionLog.write("EVENTO "+s);
  if (verbose) hostReport("[host] %s\n", s.c_str());
}

// ---------------------------------------------------------------- registradores
static const int REGS[13] = {UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2,  UC_ARM_REG_R3,  UC_ARM_REG_R4,
                             UC_ARM_REG_R5, UC_ARM_REG_R6, UC_ARM_REG_R7,  UC_ARM_REG_R8,  UC_ARM_REG_R9,
                             UC_ARM_REG_R10, UC_ARM_REG_R11, UC_ARM_REG_R12};
static u32 R(int i) { u32 v = 0; uc_reg_read(uc, REGS[i], &v); return v; }
static void W(int i, u32 v) { uc_reg_write(uc, REGS[i], &v); }
static u32 regSP() { u32 v = 0; uc_reg_read(uc, UC_ARM_REG_SP, &v); return v; }
static u32 regLR() { u32 v = 0; uc_reg_read(uc, UC_ARM_REG_LR, &v); return v; }
static u32 regPC() { u32 v = 0; uc_reg_read(uc, UC_ARM_REG_PC, &v); return v; }

static bool callFn(u32 addr, std::initializer_list<u32> args, u32* ret = nullptr);
static bool rd(u32 a, void* b, size_t n) { return n == 0 || uc_mem_read(uc, a, b, n) == UC_ERR_OK; }
static bool wr(u32 a, const void* b, size_t n) { return n == 0 || uc_mem_write(uc, a, b, n) == UC_ERR_OK; }
static u32 rd32(u32 a) { u32 v = 0; rd(a, &v, 4); return v; }
static void wr32(u32 a, u32 v) { wr(a, &v, 4); }
static void wr16(u32 a, uint16_t v) { wr(a, &v, 2); }

// n-esimo argumento de uma chamada (AAPCS: r0-r3, depois pilha)
static u32 arg(int n) { return n < 4 ? R(n) : rd32(regSP() + 4 * (n - 4)); }

static std::string cstr(u32 a, size_t max = 65536) {
  std::string s; char c;
  while (s.size() < max && rd(a + (u32)s.size(), &c, 1) && c) s.push_back(c);
  return s;
}

// ---------------------------------------------------------------- SONDA (diagnostico de toque)
// So liga com --probe. Nao altera o comportamento do jogo: apenas LE memoria e imprime.
// Os enderecos de MMain/Menu/Grp sao descobertos quando o proprio jogo zera essas estruturas
// com memset (tamanhos 0x3928, 0x214 e 0x7c; cada um aparece em um unico lugar do codigo).
static bool probe = false, muteAudio = false, profile = false;
// Diagnostico passivo de viewport. So e ativado por --trace-viewport; nao escreve
// na memoria emulada nem altera argumentos/retornos da .so.
static bool traceViewport = false;
static bool traceViewportRecording = false;
static long traceViewportFrames = 300;
static long traceViewportFrame = -1;  // -1 = initGame, antes do primeiro jniRun
static long traceViewportStartFrame = -1;
static const uint64_t TRACE_VIEWPORT_MAX_EVENTS_PER_FUNCTION = 4000;
static const uint64_t TRACE_VIEWPORT_MAX_SAMPLED_EVENTS_PER_FRAME = 8;
static const char* const TRACE_VIEWPORT_FUNCTIONS[] = {
    "Map_setMapView", "Map_loadData", "Map_freeData",
    "GameCanvas_mapCheck", "mapMoveCheck2", "Map_loadTile", "Grp_setValidRect"};
static std::string traceViewportOutDir = "trace_viewport";
static std::ofstream traceViewportLog;
static std::vector<uc_hook> traceViewportHooks;
struct TraceViewportContext {
  u32 sp = 0, r0 = 0, r1 = 0, r2 = 0, r3 = 0, stack0 = 0, stack1 = 0;
  bool thumb = false;
  bool operator==(const TraceViewportContext& other) const {
    return sp == other.sp && r0 == other.r0 && r1 == other.r1 && r2 == other.r2 &&
           r3 == other.r3 && stack0 == other.stack0 && stack1 == other.stack1 &&
           thumb == other.thumb;
  }
};
struct TraceViewportCounts {
  uint64_t calls = 0, recorded = 0, filtered = 0, limitDropped = 0;
};
struct TraceViewportFrameCounts {
  TraceViewportCounts counts;
  bool hasLastContext = false;
  TraceViewportContext lastContext;
};
static std::map<std::string, TraceViewportCounts> traceViewportTotals;
static std::map<long, std::map<std::string, TraceViewportFrameCounts>> traceViewportFrameCounts;
static std::map<std::string, bool> traceViewportHookInstalled;
static u32 gMMain = 0, gMenu = 0, gGrp = 0;
static int autoFrame = -1, autoX = 0, autoY = 0;   // --autotap FRAME,X,Y
static int autoFrame2 = -1, autoX2 = 0, autoY2 = 0; // --autotap2 FRAME,X,Y
static u32 fnLogoPress = 0, fnMenuPress = 0, fnMenuRelease = 0, fnGamePress = 0, fnGameRelease = 0;
static u32 fnGetCash = 0, fnSetCash = 0;
static u32 fnProcCommMsg = 0;
static bool pendingBuyCash = false, purchaseCancelTest = false, audioReady = false, fastForward = false;
static bool portugueseTextTest = false;
static u32 pendingCashAmount = 0;
static int currentBgTrack = -1, analogDir = 0, rightStickKey = 0;
#ifdef HAS_SDL_MIXER
static std::map<int, Mix_Chunk*> sfxCache;
static std::map<int, Mix_Music*> musCache;
#endif
static u32 rd16(u32 a) { uint16_t v = 0; rd(a, &v, 2); return v; }
static int menuState() { return gMenu ? (int)rd32(gMenu + 156) : 0; }

static std::string soundFile(int id) {
  char nm[32];
  if (id < 100) snprintf(nm, sizeof nm, "eff%02d.ogg", id);
  else snprintf(nm, sizeof nm, "bg%03d.mp3", id);
  return gameDir + "/res/raw/" + nm;
}
static void playSoundId(int id) {
#ifdef HAS_SDL_MIXER
  if (muteAudio || !audioReady) return;
  if (id >= 100) {
    if (id == currentBgTrack) return;
    auto it = musCache.find(id);
    if (it == musCache.end()) {
      Mix_Music* m = Mix_LoadMUS(soundFile(id).c_str());
      if (!m) { note(std::string("playSound ") + std::to_string(id) + " falhou: " + Mix_GetError()); return; }
      it = musCache.emplace(id, m).first;
    }
    Mix_HaltMusic();
    Mix_PlayMusic(it->second, -1);
    currentBgTrack = id;
    note("playSound " + std::to_string(id) + " (musica)");
  } else {
    auto it = sfxCache.find(id);
    if (it == sfxCache.end()) {
      Mix_Chunk* c = Mix_LoadWAV(soundFile(id).c_str());
      if (!c) { note(std::string("playSound ") + std::to_string(id) + " falhou: " + Mix_GetError()); return; }
      it = sfxCache.emplace(id, c).first;
    }
    Mix_PlayChannel(-1, it->second, 0);
    note("playSound " + std::to_string(id) + " (efeito)");
  }
#else
  (void)id;
  if (!muteAudio && !audioReady) note("playSound ignorado: host compilado sem SDL_mixer");
#endif
}
static void applyAudioPreferences() {
#ifdef HAS_SDL_MIXER
  if(!audioReady) return;
  const bool muted=!audioFocused && preferences.muteUnfocused;
  Mix_VolumeMusic(destinia::scaledVolume(nativeMusicVolume,preferences.music,muted));
  Mix_Volume(-1,destinia::scaledVolume(nativeEffectsVolume,preferences.effects,muted));
#endif
}
static void setSoundVol(int ch, int level) {
  if(ch==1) nativeMusicVolume=std::clamp(level,0,5);
  else nativeEffectsVolume=std::clamp(level,0,5);
  applyAudioPreferences();
}
static void updateAudioState(bool focused,bool paused) {
  audioFocused=focused;
  applyAudioPreferences();
#ifdef HAS_SDL_MIXER
  if(audioReady && paused!=audioPaused) {
    if(paused) { Mix_PauseMusic(); Mix_Pause(-1); }
    else { Mix_ResumeMusic(); Mix_Resume(-1); }
  }
#endif
  audioPaused=paused;
}

// Teclas WIPI usadas pelo port Vita: 3/9/5/7 = cima/baixo/esq/dir, 17 = confirmar, 10 = voltar, 27 = menu, 1 = start.
static void sendKey(u32 pressFn, u32 releaseFn, int key, bool down) {
  u32 fn = down ? pressFn : releaseFn;
  if (!fn) return;
  callFn(fn, {(u32)key});
}
static bool difficultyMenuKey(int key,bool down);
static bool codexMenuKey(int key,bool down);
static bool inventoryMenuKey(int key,bool down);
static bool fcMenuKey(int key,bool down);
static void gameKey(int key, bool down) {
  if(difficultyMenuKey(key,down) || bossRushUI.handleKey(key,down) || inventoryMenuKey(key,down) || fcMenuKey(key,down) || codexMenuKey(key,down)) return;
  destinia::sessionLog.write("ENTRADA tecla="+std::to_string(key)+(down?" pressionada":" solta"));
  int st = menuState();
  if (st <= 0) { if (down && fnLogoPress) callFn(fnLogoPress, {(u32)key}); return; }
  if (st == 1) sendKey(fnMenuPress, fnMenuRelease, key, down);
  else sendKey(fnGamePress, fnGameRelease, key, down);
}
static void probeState(const char* tag) {
  if (!probe) return;
  if (!gMMain || !gMenu) { hostReport("[sonda] %s: enderecos ainda nao descobertos (MMain=%08x Menu=%08x Grp=%08x)\n", tag, gMMain, gMenu, gGrp); return; }
  hostReport("[sonda] %s: rede=%u (0=livre 1=aguardando 2=erro 3=aviso) | menu.estado=%u menu.anim=%u | lista: y0=%u altura=%u itens=%u | tecla_toque=%u | tela=%ux%u\n",
          tag, rd16(gMMain + 14624), rd32(gMenu + 156), rd16(gMenu + 178), rd16(gMenu + 172), rd16(gMenu + 174), rd16(gMenu + 176),
          rd32(gMMain + 14604), gGrp ? rd16(gGrp + 92) : 0, gGrp ? rd16(gGrp + 94) : 0);
}
// Diagnostico passivo da camera do mapa. So liga com --trace-camera; apenas LE memoria e imprime no terminal.
// Game+18552/18554 = camera X/Y (int16); curMapData+0/+2 = largura/altura do mapa em tiles (16 px cada).
static bool traceCamera = false;
static void cameraTick(long frame) {
  if (!traceCamera) return;
  static int lastW = -1, lastH = -1, lastNeg = -1;
  int camX = (int16_t)rd16(SO_BASE + 0x77EE0 + 18552), camY = (int16_t)rd16(SO_BASE + 0x77EE0 + 18554);
  int mw = (int16_t)rd16(SO_BASE + 0x80B18 + 0), mh = (int16_t)rd16(SO_BASE + 0x80B18 + 2);
  int neg = (camX < 0 || camY < 0) ? 1 : 0;
  if (frame % 60 != 0 && mw == lastW && mh == lastH && neg == lastNeg) return;
  lastW = mw; lastH = mh; lastNeg = neg;
  hostReport("[camera] quadro %ld camX=%d camY=%d mapa=%dx%d tiles (%dx%d px) tela=%dx%d divisao_prevista_x=%d %s\n",
          frame, camX, camY, mw, mh, mw * 16, mh * 16, SCREEN_W, SCREEN_H, SCREEN_W - mw * 16,
          neg ? "<< CAMERA NEGATIVA" : "");
}
// Centraliza apenas eixos menores que o viewport. Nao muda arquivos ou posicoes do mundo.
static constexpr uint16_t MAP_BACKGROUND = 0x8410; // cinza medio em RGB565
static void centerSmallMapCamera() {
  const int mw = (int16_t)rd16(SO_BASE + 0x80B18), mh = (int16_t)rd16(SO_BASE + 0x80B18 + 2);
  if (mw <= 0 || mh <= 0) return;
  const int dimensions[] = {mw * 16, mh * 16};
  const int screen[] = {SCREEN_W, SCREEN_H};
  for (int axis = 0; axis < 2; ++axis) {
    if (dimensions[axis] >= screen[axis]) continue;
    const u32 address = SO_BASE + 0x77EE0 + 18552 + 2 * axis;
    const int16_t centered = (int16_t)((dimensions[axis] - screen[axis]) / 2);
    if ((int16_t)rd16(address) != centered) wr(address, &centered, sizeof centered);
  }
}
// After each verified ARM camera store, including inline movement/scripts/shake.
// A memory-read hook instruments all emulated loads even with a four-byte filter;
// these bounded code hooks keep the same centering without that global cost.
static void cameraWriteHook(uc_engine*, uint64_t, uint32_t, void*) {
  centerSmallMapCamera();
}
// Correcao automatica da faixa duplicada em mapas menores que a tela (ex.: 30x30 tiles = 480 px em 800x480).
// No antigo framebuffer 800x480, a camera negativa alinhava o mapa a direita. Map_drawBuff le tiles sem checar limites
// e repete o cenario fora do mapa. Aqui, quando Map_drawBuff chama uma das duas rotinas que desenham UM tile
// fora do mapa, o x e jogado para fora da tela e o proprio clipping do jogo descarta o desenho.
// O filtro afeta somente tiles; a centralizacao compartilhada ajusta a camera em memoria.
// Desligar com --no-fix-map-edge (para comparar). Enderecos da .so: Map_drawBuff 0x19308 (660 bytes),
// Grp_drawClipWImageP 0xBB90, Grp_drawSimpleClipWImageP 0xB4BC; Game 0x77EE0; curMapData 0x80B18.
static bool fixMapEdge = true;
static void mapEdgeHook(uc_engine*, uint64_t, uint32_t, void*) {
  u32 lr = regLR() & ~1u;
  if (lr < SO_BASE + 0x19308 || lr >= SO_BASE + 0x19308 + 660) return;
  centerSmallMapCamera();
  int camX = (int16_t)rd16(SO_BASE + 0x77EE0 + 18552), camY = (int16_t)rd16(SO_BASE + 0x77EE0 + 18554);
  int mw = (int16_t)rd16(SO_BASE + 0x80B18), mh = (int16_t)rd16(SO_BASE + 0x80B18 + 2);
  int wx = (int)R(1) + camX, wy = (int)R(2) + camY;
  if (wx < 0 || wx >= mw * 16 || wy < 0 || wy >= mh * 16) W(1, 0x4000);  // fora da tela: o clipping descarta
}
// Complemento da correcao acima: ANTES de Map_drawBuff desenhar os tiles, pinta de cinza tudo que fica FORA do mapa.
// Motivo (evidencia dos prints): chuva e outros efeitos sao desenhados antes dos tiles; no 400x240 original o mapa
// os cobria por completo, mas num mapa menor que a tela eles ficam visiveis na faixa fora do mapa.
// HUD, heroi, monstros e efeitos desenhados DEPOIS de Map_drawBuff nao sao afetados.
// frameBuffer[idx] -> {largura, altura, ponteiro dos pixels RGB565}; vetor em 0x4EF5C; Map_drawBuff em 0x19308.
static void mapEdgeClearHook(uc_engine*, uint64_t, uint32_t, void*) {
  centerSmallMapCamera();
  u32 idx = R(0);
  if (idx >= 5) return;
  u32 fbs = rd32(SO_BASE + 0x4EF5C + 4 * idx);
  if (!fbs) return;
  int w = (int)rd32(fbs), h = (int)rd32(fbs + 4); u32 px = rd32(fbs + 8);
  if (w <= 0 || h <= 0 || w > 4096 || h > 4096 || !px) return;
  int camX = (int16_t)rd16(SO_BASE + 0x77EE0 + 18552), camY = (int16_t)rd16(SO_BASE + 0x77EE0 + 18554);
  int mw = (int16_t)rd16(SO_BASE + 0x80B18), mh = (int16_t)rd16(SO_BASE + 0x80B18 + 2);
  if (mw <= 0 || mh <= 0) return;  // mapa ainda nao carregado
  int x0 = std::clamp(-camX, 0, w), x1 = std::clamp(-camX + mw * 16, 0, w);
  int y0 = std::clamp(-camY, 0, h), y1 = std::clamp(-camY + mh * 16, 0, h);
  if (x0 == 0 && x1 == w && y0 == 0 && y1 == h) return;  // o mapa cobre a tela inteira
  std::vector<uint16_t> background((size_t)w, MAP_BACKGROUND);
  for (int y = 0; y < h; y++) {
    u32 row = px + (u32)y * (u32)w * 2;
    if (y < y0 || y >= y1) { wr(row, background.data(), (size_t)w * 2); continue; }
    if (x0 > 0) wr(row, background.data(), (size_t)x0 * 2);
    if (x1 < w) wr(row + (u32)x1 * 2, background.data(), (size_t)(w - x1) * 2);
  }
}
static void probeTick(long frame) {   // chamada a cada quadro: imprime quando o estado muda (e a cada 100 quadros)
  if (!probe || !gMMain || !gMenu) return;
  static u32 last = 0xFFFFFFFF;
  u32 sig = rd16(gMMain + 14624) | (rd32(gMenu + 156) << 8) | ((rd16(gMenu + 178) >= 16 ? 1u : 0u) << 24);
  if (sig != last || frame % 100 == 0) { char t[48]; snprintf(t, sizeof t, "quadro %ld", frame); probeState(t); last = sig; }
}

// ---------------------------------------------------------------- malloc simples
static u32 heapPtr = HEAP_BASE + 16;
static std::map<u32, u32> chunkSize;
static std::map<u32, std::vector<u32>> freeLists;
static u32 hmalloc(u32 n) {
  n = (std::max<u32>(n, 1) + 15) & ~15u;
  auto& fl = freeLists[n];
  if (!fl.empty()) { u32 p = fl.back(); fl.pop_back(); chunkSize[p] = n; return p; }
  if (heapPtr + n > HEAP_BASE + HEAP_SIZE) { hostReport("[host] heap esgotado (pediu %u)\n", n); return 0; }
  u32 p = heapPtr; heapPtr += n; chunkSize[p] = n; return p;
}
static void hfree(u32 p) {
  auto it = chunkSize.find(p);
  if (p == 0 || it == chunkSize.end()) return;
  freeLists[it->second].push_back(p); chunkSize.erase(it);
}

// ---------------------------------------------------------------- printf
static std::string fmtPrintf(u32 fmtAddr, int ai) {
  std::string f = cstr(fmtAddr), out;
  if (f == "$%d.%d" || f == "[%d& MORE]") return out;
  for (size_t i = 0; i < f.size(); i++) {
    if (f[i] != '%') { out.push_back(f[i]); continue; }
    i++;
    if (i >= f.size()) break;
    std::string flags; while (i < f.size() && strchr("-+ #0", f[i])) flags.push_back(f[i++]);
    int width = -1, prec = -1;
    if (i < f.size() && f[i] == '*') { width = (int)arg(ai++); i++; }
    else { if (i < f.size() && isdigit((unsigned char)f[i])) width = 0; while (i < f.size() && isdigit((unsigned char)f[i])) width = width * 10 + (f[i++] - '0'); }
    if (i < f.size() && f[i] == '.') { i++; prec = 0; if (i < f.size() && f[i] == '*') { prec = (int)arg(ai++); i++; } else while (i < f.size() && isdigit((unsigned char)f[i])) prec = prec * 10 + (f[i++] - '0'); }
    while (i < f.size() && strchr("hlLzjt", f[i])) i++;
    if (i >= f.size()) break;
    char c = f[i], spec[32], buf[512];
    std::string sp = "%" + flags;
    if (width >= 0) sp += std::to_string(width);
    if (prec >= 0) sp += "." + std::to_string(prec);
    if (c == '%') { out.push_back('%'); continue; }
    if (c == 'd' || c == 'i') { snprintf(spec, sizeof spec, "%sd", sp.c_str()); snprintf(buf, sizeof buf, spec, (int)arg(ai++)); out += buf; }
    else if (c == 'u' || c == 'x' || c == 'X' || c == 'o') { snprintf(spec, sizeof spec, "%s%c", sp.c_str(), c); snprintf(buf, sizeof buf, spec, (unsigned)arg(ai++)); out += buf; }
    else if (c == 'c') { snprintf(spec, sizeof spec, "%sc", sp.c_str()); snprintf(buf, sizeof buf, spec, (int)arg(ai++)); out += buf; }
    else if (c == 'p') { snprintf(buf, sizeof buf, "0x%x", arg(ai++)); out += buf; }
    else if (c == 's') { u32 p = arg(ai++); std::string s = p ? cstr(p) : "(null)"; snprintf(spec, sizeof spec, "%ss", sp.c_str()); std::vector<char> b(s.size() + 512); snprintf(b.data(), b.size(), spec, s.c_str()); out += b.data(); }
    else { out.push_back('%'); out.push_back(c); }
  }
  return out;
}

// ---------------------------------------------------------------- arquivos
static destinia::SaveTransaction saveTransaction;
static std::map<u32, FILE*> files;
static std::map<u32,std::string> filePaths;
static u32 nextFile = 0x5000;
static std::string hostPath(const std::string& p) {
  // [OK] a .so grava saves em /data/data/game.destiniaeng/files/<arquivo>
  const std::string pre = "/data/data/game.destiniaeng/files/";
  if (p.compare(0, pre.size(), pre) == 0) return saveDir + "/" + p.substr(pre.size());
  if (!p.empty() && p[0] == '/') return saveDir + p;
  return saveDir + "/" + p;
}
static bool readFile(const std::string& path, std::vector<uint8_t>& out) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return false;
  out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
  return true;
}

#include "../common/portuguese_text.h"
using destinia::isPortugueseAsset;
static bool applyPortugueseText(const std::string& name, std::vector<uint8_t>& data) {
  return destinia::applyPortugueseText(portugueseTextTest, name, data, note);
}

// ---------------------------------------------------------------- JNI falso
static std::map<u32, std::vector<uint8_t>> arrays;  // jbyteArray -> bytes
static u32 nextArr = 0x1000, nextCls = 0x100;
static std::map<u32, u32> elemsOf;                   // ponteiro emulado -> jbyteArray
static const std::map<std::string, u32> METHODS = {
    {"playVib", 1}, {"gotoURL", 2}, {"getAssetResSize", 3}, {"getAssetRes", 4}, {"playSound", 5},
    {"setSoundVolume", 6}, {"close", 7}, {"buyCash", 8}, {"logEvent", 9}};
static std::string arrText(u32 h) { auto it = arrays.find(h); return it == arrays.end() ? "" : std::string(it->second.begin(), it->second.end()); }

static void stubSet(u32 id, const std::string& name, std::function<void()> f) { stubName[id] = name; handlers[id] = std::move(f); }

static void codexFileRemoved(const std::string& path);
static void difficultyPrepareRemoval(const std::string& path);
static void difficultyEndCall();

static void setupStubs() {
  u32 n = ID_LIBC;
  auto L = [&](const char* name, std::function<void()> f) { importId[name] = n; stubSet(n, name, std::move(f)); n++; };

  L("memcpy", [] { u32 d = R(0), s = R(1), c = R(2); std::vector<uint8_t> b(c); rd(s, b.data(), c); wr(d, b.data(), c); W(0, d); });
  L("memset", [] {
    u32 d = R(0), c = R(2); std::vector<uint8_t> b(c, (uint8_t)R(1)); wr(d, b.data(), c); W(0, d);
    if (c == 0x3928 && !gMMain) { gMMain = d; if (probe) hostReport("[sonda] MMain em %08x (lib+%x)\n", d, d - SO_BASE); }
    else if (c == 0x214 && !gMenu) { gMenu = d; if (probe) hostReport("[sonda] Menu em %08x (lib+%x)\n", d, d - SO_BASE); }
    else if (c == 0x7c && gMMain && !gGrp) { gGrp = d; if (probe) hostReport("[sonda] Grp em %08x (lib+%x)\n", d, d - SO_BASE); }
  });
  L("strlen", [] { W(0, (u32)cstr(R(0)).size()); });
  L("strcpy", [] { std::string s = cstr(R(1)); wr(R(0), s.c_str(), s.size() + 1); });
  L("strncmp", [] { std::string a = cstr(R(0)), b = cstr(R(1)); W(0, (u32)strncmp(a.c_str(), b.c_str(), R(2))); });
  L("strstr", [] { std::string h = cstr(R(0)), nd = cstr(R(1)); size_t p = h.find(nd); W(0, p == std::string::npos ? 0 : R(0) + (u32)p); });
  L("sprintf", [] { std::string s = fmtPrintf(R(1), 2); wr(R(0), s.c_str(), s.size() + 1); W(0, (u32)s.size()); });
  L("malloc", [] { W(0, hmalloc(R(0))); });
  L("free", [] { hfree(R(0)); });
  L("clock", [] {
    // [?] no Android clock() devolve microssegundos de CPU; aqui usamos tempo real.
    static auto t0 = std::chrono::steady_clock::now();
    W(0, (u32)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
  });
  L("fopen", [] {
    std::string p = hostPath(cstr(R(0))), m = cstr(R(1));
    FILE* f = saveTransaction.open(fs::path(p),m);
    note("fopen " + p + " [" + m + "] -> " + (f ? "ok" : "falhou"));
    if (!f) { W(0, 0); return; }
    u32 h = nextFile; nextFile += 16; files[h] = f; filePaths[h] = p; W(0, h);
  });
  L("fclose", [] {
    const u32 handle=R(0); auto it=files.find(handle); int result=0;
    if (it!=files.end()) {
      result=saveTransaction.close(it->second); files.erase(it);
      auto name=filePaths.find(handle);
      if (name!=filePaths.end()) {
        destinia::sessionLog.write("ARQUIVO fechado: "+name->second+" resultado="+std::to_string(result),true);
        filePaths.erase(name);
      }
    }
    W(0,static_cast<u32>(result));
  });
  L("fread", [] {
    const u32 size=R(1),count=R(2);
    if (!size || !count) { W(0,0); return; }
    auto it=files.find(R(3)); if (it==files.end()) { W(0,0); return; }
    const uint64_t bytes=uint64_t(size)*count;
    if (bytes>HEAP_SIZE) { hostReport("[host] fread excede limite de memoria\n"); fatal=true; uc_emu_stop(uc); W(0,0); return; }
    std::vector<uint8_t> buffer(static_cast<size_t>(bytes));
    const size_t got=fread(buffer.data(),size,count,it->second);
    if (!wr(R(0),buffer.data(),got*size)) { hostReport("[host] fread: destino ARM invalido\n"); fatal=true; uc_emu_stop(uc); W(0,0); return; }
    W(0,static_cast<u32>(got));
  });
  L("fwrite", [] {
    const u32 size=R(1),count=R(2),handle=R(3);
    if (!size || !count) { W(0,0); return; }
    auto it=files.find(handle); if (it==files.end()) { W(0,0); return; }
    const uint64_t bytes=uint64_t(size)*count;
    if (bytes>HEAP_SIZE) { hostReport("[host] fwrite excede limite de memoria\n"); fatal=true; uc_emu_stop(uc); W(0,0); return; }
    std::vector<uint8_t> buffer(static_cast<size_t>(bytes));
    if (!rd(R(0),buffer.data(),buffer.size())) { hostReport("[host] fwrite: origem ARM invalida\n"); fatal=true; uc_emu_stop(uc); W(0,0); return; }
    const size_t written=fwrite(buffer.data(),size,count,it->second);
    destinia::sessionLog.write("SAVE escrita observada: "+filePaths[handle]+" bytes="+std::to_string(written*size)+" solicitados="+std::to_string(bytes),true);
    W(0,static_cast<u32>(written));
  });
  L("fseek", [] { auto it = files.find(R(0)); W(0, it == files.end() ? (u32)-1 : (u32)fseek(it->second, (long)(int)R(1), (int)R(2))); });
  L("ftell", [] { auto it = files.find(R(0)); W(0, it == files.end() ? (u32)-1 : (u32)ftell(it->second)); });
  L("remove", [] { const auto path=hostPath(cstr(R(0))); const int result=saveTransaction.remove(fs::path(path)); if(result==0)difficultyPrepareRemoval(path); W(0,(u32)result); });
  L("__android_log_print", [] {
    const std::string message=fmtPrintf(R(2),3),tag=cstr(R(1));
    if(verbose) hostReport("[jogo:%s] %s\n",tag.c_str(),message.c_str());
    else destinia::sessionLog.write("JOGO ["+tag+"] "+message,R(0)>=6);
  });
  L("__stack_chk_fail", [] { hostReport("[host] __stack_chk_fail chamado (pilha corrompida)\n"); fatal = true; uc_emu_stop(uc); });
  L("__aeabi_unwind_cpp_pr0", [] {});
  L("__aeabi_unwind_cpp_pr1", [] {});
  L("__aeabi_idiv", [] { int a = (int)R(0), b = (int)R(1); W(0, b ? (u32)(a / b) : 0); });
  L("__aeabi_uidiv", [] { u32 a = R(0), b = R(1); W(0, b ? a / b : 0); });
  L("__aeabi_idivmod", [] { int a = (int)R(0), b = (int)R(1); W(0, b ? (u32)(a / b) : 0); W(1, b ? (u32)(a % b) : 0); });
  L("__aeabi_uidivmod", [] { u32 a = R(0), b = R(1); W(0, b ? a / b : 0); W(1, b ? a % b : 0); });
  L("__aeabi_lmul", [] {
    uint64_t a = ((uint64_t)R(1) << 32) | R(0), b = ((uint64_t)R(3) << 32) | R(2), r = a * b;
    W(0, (u32)r); W(1, (u32)(r >> 32));
  });
  L("__aeabi_ldivmod", [] {
    int64_t a = (int64_t)(((uint64_t)R(1) << 32) | R(0)), b = (int64_t)(((uint64_t)R(3) << 32) | R(2));
    int64_t q = b ? a / b : 0, r = b ? a % b : 0;
    W(0, (u32)q); W(1, (u32)((uint64_t)q >> 32)); W(2, (u32)r); W(3, (u32)((uint64_t)r >> 32));
  });

  // ---- tabela de funcoes do JNIEnv (indices da especificacao JNI) ----
  auto J = [&](int idx, const char* name, std::function<void()> f) { stubSet(ID_JNI + idx, std::string("JNI.") + name, std::move(f)); };
  J(6, "FindClass", [] { std::string c = cstr(R(1)); note("FindClass " + c); W(0, nextCls++); });
  J(21, "NewGlobalRef", [] { W(0, R(1) ? R(1) : 1); });  // [?] a decompilacao perdeu o argumento; devolve o que veio
  J(113, "GetStaticMethodID", [] {
    std::string name = cstr(R(2)); auto it = METHODS.find(name);
    note("GetStaticMethodID " + name + " " + cstr(R(3)));
    W(0, it == METHODS.end() ? 0 : it->second);
  });
  J(171, "GetArrayLength", [] { auto it = arrays.find(R(1)); W(0, it == arrays.end() ? 0 : (u32)it->second.size()); });
  J(176, "NewByteArray", [] { u32 h = nextArr++; arrays[h].assign(R(1), 0); W(0, h); });
  J(184, "GetByteArrayElements", [] {
    auto it = arrays.find(R(1)); if (it == arrays.end()) { W(0, 0); return; }
    u32 p = hmalloc((u32)it->second.size()); wr(p, it->second.data(), it->second.size());
    if (R(2)) wr32(R(2), 1);
    elemsOf[p] = R(1); W(0, p);
  });
  J(192, "ReleaseByteArrayElements", [] {
    u32 arr = R(1), p = R(2), mode = R(3); auto it = arrays.find(arr);
    if (it != arrays.end() && mode != 2) rd(p, it->second.data(), it->second.size());  // 2 = JNI_ABORT
    if (mode != 1) { hfree(p); elemsOf.erase(p); }                                     // 1 = JNI_COMMIT
  });
  J(200, "GetByteArrayRegion", [] {
    auto it = arrays.find(R(1)); u32 st = R(2), len = R(3), buf = arg(4);
    if (it != arrays.end() && st + (size_t)len <= it->second.size()) wr(buf, it->second.data() + st, len);
  });
  J(208, "SetByteArrayRegion", [] {
    auto it = arrays.find(R(1)); u32 st = R(2), len = R(3), buf = arg(4);
    if (it != arrays.end() && st + (size_t)len <= it->second.size()) rd(buf, it->second.data() + st, len);
  });
  J(219, "GetJavaVM", [] { wr32(R(1), VM_OBJ); W(0, 0); });

  // [OK] getAssetResSize([B)I -> tamanho do arquivo em assets (0 se nao existe)
  J(129, "CallStaticIntMethod", [] {
    u32 mid = R(2);
    if (mid == 3) {
      std::string nm = arrText(R(3)); std::error_code ec; uintmax_t sz = 0;
      if (portugueseTextTest && isPortugueseAsset(nm)) {
        std::vector<uint8_t> data;
        if (readFile(assetsDir + "/" + nm, data)) {
          applyPortugueseText(nm, data);
          sz = data.size();
        } else {
          ec = std::make_error_code(std::errc::no_such_file_or_directory);
        }
      } else {
        sz = fs::file_size(assetsDir + "/" + nm, ec);
      }
      note("getAssetResSize " + nm + " -> " + (ec ? "0 (nao existe)" : std::to_string(sz)));
      W(0, ec ? 0 : (u32)sz);
    } else { note("CallStaticIntMethod metodo desconhecido " + std::to_string(mid)); W(0, 0); }
  });
  // [OK] getAssetRes([B)[B -> bytes do arquivo (null se nao existe)
  J(114, "CallStaticObjectMethod", [] {
    u32 mid = R(2);
    if (mid == 4) {
      std::string nm = arrText(R(3)); std::vector<uint8_t> data;
      if (!readFile(assetsDir + "/" + nm, data)) { note("getAssetRes " + nm + " -> NAO EXISTE"); W(0, 0); return; }
      bool translated = applyPortugueseText(nm, data);
      if (translated) hostReport("[host] textos PT aplicados em %s\n", nm.c_str());
      destinia::sessionLog.write("ASSET carregado: "+nm+" bytes="+std::to_string(data.size()));
      u32 h = nextArr++; arrays[h] = std::move(data); W(0, h);
    } else { note("CallStaticObjectMethod metodo desconhecido " + std::to_string(mid)); W(0, 0); }
  });
  // metodos estaticos void do Java. Aqui ficam anuncios/compras/som/vibracao.
  J(141, "CallStaticVoidMethod", [] {
    u32 mid = R(2), a0 = R(3);
    switch (mid) {
      case 1: break;                                                              // playVib: ignorado no PC
      case 2: note("gotoURL " + arrText(a0) + " (ignorado)"); break;              // abrir navegador: ignorado
      case 5: playSoundId((int)a0); break;
      case 6: {
        int ch = (int)a0, lv = (int)arg(5);
        note("setSoundVolume " + std::to_string(ch) + "," + std::to_string((int)arg(4)) + "," + std::to_string(lv));
        setSoundVol(ch, lv);
        break;
      }
      case 7: break;                                                              // fechar socket
      case 8: {
        pendingCashAmount=0;pendingBuyCash=true;
        note("buyCash("+std::to_string(a0)+"): compra indisponivel; cancelamento offline agendado");
        break;
      }
      case 9: break;                                                              // logEvent (Flurry): descartado
      default: note("CallStaticVoidMethod metodo desconhecido " + std::to_string(mid));
    }
  });

  // JavaVM: [OK] a .so so usa GetEnv (indice 6) com versao 0x10006
  stubSet(ID_VM + 6, "VM.GetEnv", [] { wr32(R(1), ENV_OBJ); W(0, 0); });
  stubSet(ID_VM + 4, "VM.AttachCurrentThread", [] { wr32(R(1), ENV_OBJ); W(0, 0); });
}

static u32 stubAddr(u32 id) { return STUB_BASE + id * 4; }

// ---------------------------------------------------------------- carregador ELF

static std::map<std::string, u32> exportsMap;

// ---------------------------------------------------------------- trace de viewport
// Estes hooks observam somente a entrada de funcoes exportadas. As assinaturas
// internas nao foram inferidas: r0-r3 e as primeiras palavras da pilha sao
// registrados crus, conforme AAPCS, para posterior correlacao no Ghidra.
static long long traceViewportTimestampMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
}

static bool traceViewportActive() {
  return traceViewport && traceViewportRecording && traceViewportLog.is_open() &&
         traceViewportFrame >= traceViewportStartFrame &&
         traceViewportFrame - traceViewportStartFrame < traceViewportFrames;
}

static void traceViewportDimensions(const char* tag) {
  if (!traceViewportLog.is_open()) return;
  const auto width = exportsMap.find("mainScreenWidth");
  const auto height = exportsMap.find("mainScreenHeight");
  if (width == exportsMap.end() || height == exportsMap.end()) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << traceViewportFrame << " " << tag
                     << " dimensions globals unavailable\n";
    return;
  }
  u32 w = 0, h = 0;
  const bool gotW = rd(width->second, &w, sizeof w);
  const bool gotH = rd(height->second, &h, sizeof h);
  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " frame=" << traceViewportFrame << " " << tag << " mainScreenWidth=";
  if (gotW) traceViewportLog << w; else traceViewportLog << "<read-failed>";
  traceViewportLog << " mainScreenHeight=";
  if (gotH) traceViewportLog << h; else traceViewportLog << "<read-failed>";
  traceViewportLog << "\n";
}

static void stopTraceViewportRecording(const char* reason) {
  if (!traceViewportRecording) return;
  traceViewportRecording = false;
  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " frame=" << traceViewportFrame << " recording stopped reason=" << reason << "\n";
  for (const char* name : TRACE_VIEWPORT_FUNCTIONS) {
    const TraceViewportCounts& counts = traceViewportTotals[name];
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << traceViewportFrame << " total fn=" << name
                     << " hook_installed=" << (traceViewportHookInstalled[name] ? 1 : 0)
                     << " calls=" << counts.calls << " recorded=" << counts.recorded
                     << " dropped_filter=" << counts.filtered
                     << " dropped_limit=" << counts.limitDropped << "\n";
  }
  traceViewportDimensions(reason);
  traceViewportLog.flush();
  hostReport("[trace] viewport recording stopped (%s).\n", reason);
}

static void traceViewportWriteFrameSummary(long frame) {
  if (!traceViewportRecording || !traceViewportLog.is_open() ||
      frame < traceViewportStartFrame || frame - traceViewportStartFrame >= traceViewportFrames)
    return;

  const auto frameIt = traceViewportFrameCounts.find(frame);
  for (const char* name : TRACE_VIEWPORT_FUNCTIONS) {
    TraceViewportCounts counts;
    if (frameIt != traceViewportFrameCounts.end()) {
      const auto functionIt = frameIt->second.find(name);
      if (functionIt != frameIt->second.end()) counts = functionIt->second.counts;
    }
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " frame-summary fn=" << name
                     << " hook_installed=" << (traceViewportHookInstalled[name] ? 1 : 0)
                     << " calls=" << counts.calls << " recorded=" << counts.recorded
                     << " dropped_filter=" << counts.filtered
                     << " dropped_limit=" << counts.limitDropped << "\n";
  }
}

static TraceViewportContext traceViewportReadContext() {
  TraceViewportContext context;
  context.sp = regSP();
  context.r0 = R(0); context.r1 = R(1); context.r2 = R(2); context.r3 = R(3);
  rd(context.sp, &context.stack0, sizeof context.stack0);
  rd(context.sp + 4, &context.stack1, sizeof context.stack1);
  u32 cpsr = 0;
  uc_reg_read(uc, UC_ARM_REG_CPSR, &cpsr);
  context.thumb = (cpsr & (1u << 5)) != 0;
  return context;
}

static bool traceViewportIsSampledFunction(const char* name) {
  return std::strcmp(name, "Grp_setValidRect") == 0 ||
         std::strcmp(name, "GameCanvas_mapCheck") == 0 ||
         std::strcmp(name, "mapMoveCheck2") == 0 ||
         std::strcmp(name, "Map_loadTile") == 0;
}

static bool traceViewportIsTransitionFunction(const char* name) {
  return std::strcmp(name, "Map_setMapView") == 0 ||
         std::strcmp(name, "Map_loadData") == 0 ||
         std::strcmp(name, "Map_freeData") == 0;
}

static void traceViewportHook(uc_engine*, uint64_t address, uint32_t, void* userData) {
  const char* name = static_cast<const char*>(userData);
  if (!traceViewportActive()) return;

  TraceViewportFrameCounts& frameCounts = traceViewportFrameCounts[traceViewportFrame][name];
  TraceViewportCounts& totals = traceViewportTotals[name];
  ++frameCounts.counts.calls;
  ++totals.calls;
  const TraceViewportContext context = traceViewportReadContext();
  const bool isSampled = traceViewportIsSampledFunction(name);
  if (isSampled && frameCounts.hasLastContext && context == frameCounts.lastContext) {
    ++frameCounts.counts.filtered;
    ++totals.filtered;
    return;
  }
  if (isSampled) {
    frameCounts.lastContext = context;
    frameCounts.hasLastContext = true;
  }

  if (totals.recorded >= TRACE_VIEWPORT_MAX_EVENTS_PER_FUNCTION ||
      (isSampled && frameCounts.counts.recorded >= TRACE_VIEWPORT_MAX_SAMPLED_EVENTS_PER_FRAME)) {
    ++frameCounts.counts.limitDropped;
    ++totals.limitDropped;
    return;
  }
  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " event_class=" << (traceViewportIsTransitionFunction(name) ? "transition" : "frequent")
                   << " phase=" << (traceViewportFrame < 0 ? "init" : "run")
                   << " frame=" << traceViewportFrame
                   << " fn=" << name
                   << " pc=0x" << std::hex << static_cast<u32>(address)
                   << " thumb=" << (context.thumb ? 1 : 0)
                   << " sp=0x" << context.sp
                   << " r0=0x" << context.r0 << " r1=0x" << context.r1
                   << " r2=0x" << context.r2 << " r3=0x" << context.r3
                   << " stack0=0x" << context.stack0 << " stack1=0x" << context.stack1
                   << std::dec << "\n";
  ++frameCounts.counts.recorded;
  ++totals.recorded;
}

static bool installTraceViewportHook(const char* symbol) {
  traceViewportHookInstalled[symbol] = false;
  const auto it = exportsMap.find(symbol);
  if (it == exportsMap.end()) {
    traceViewportLog << "[trace] skip " << symbol << ": symbol not exported\n";
    return false;
  }
  const u32 address = it->second & ~1u;  // simbolos Thumb tem bit 0 marcado no ELF
  uint16_t instruction = 0;
  if (address < SO_BASE || address >= SO_BASE + loadedSoSpan || !rd(address, &instruction, sizeof instruction)) {
    traceViewportLog << "[trace] skip " << symbol << ": invalid executable address 0x"
                     << std::hex << address << std::dec << "\n";
    return false;
  }
  uc_hook hook = 0;
  const uc_err result = uc_hook_add(uc, &hook, UC_HOOK_CODE, reinterpret_cast<void*>(traceViewportHook),
                                    const_cast<char*>(symbol), address, address);
  if (result != UC_ERR_OK) {
    traceViewportLog << "[trace] skip " << symbol << ": uc_hook_add=" << uc_strerror(result) << "\n";
    return false;
  }
  traceViewportHooks.push_back(hook);
  traceViewportHookInstalled[symbol] = true;
  traceViewportLog << "[trace] hook " << symbol << " addr=0x" << std::hex << address
                   << " first16=0x" << instruction << std::dec << "\n";
  return true;
}

static bool startTraceViewport() {
  std::error_code ec;
  fs::create_directories(traceViewportOutDir, ec);
  if (ec) { hostReport("[trace] nao consegui criar %s: %s\n", traceViewportOutDir.c_str(), ec.message().c_str()); return false; }
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  const fs::path path = fs::path(traceViewportOutDir) /
      ("viewport_trace_" + std::to_string(millis) + ".log");
  fs::path uniquePath = path;
  unsigned int suffix = 1;
  while (fs::exists(uniquePath, ec)) {
    if (ec) { hostReport("[trace] erro verificando %s: %s\n", uniquePath.string().c_str(), ec.message().c_str()); return false; }
    uniquePath = fs::path(traceViewportOutDir) /
        ("viewport_trace_" + std::to_string(millis) + "_" + std::to_string(suffix++) + ".log");
  }
  if (ec) { hostReport("[trace] erro verificando %s: %s\n", uniquePath.string().c_str(), ec.message().c_str()); return false; }
  traceViewportLog.open(uniquePath, std::ios::out | std::ios::trunc);
  if (!traceViewportLog) { hostReport("[trace] nao consegui abrir %s\n", uniquePath.string().c_str()); return false; }
  traceViewportLog << "# Destinia host_zoom viewport trace (passive)\n"
                   << "# frame limit per F9 recording=" << traceViewportFrames
                   << " event limit per function per recording=" << TRACE_VIEWPORT_MAX_EVENTS_PER_FUNCTION
                   << " sampled-event limit per function per frame="
                   << TRACE_VIEWPORT_MAX_SAMPLED_EVENTS_PER_FRAME << "\n"
                   << "# press F9 to start/stop; each start captures a uniquely named BMP\n"
                   << "# transition events are recorded separately from sampled frequent events\n"
                   << "# framebuffer query hooks disabled; global dimensions sampled in summaries\n"
                   << "# frame summaries include calls=0; hook_installed distinguishes missing hooks from no calls\n"
                   << "# dropped_filter counts repeated sampled contexts; dropped_limit counts per-function/frame caps\n"
                   << "# raw AAPCS context only; argument meanings are intentionally not inferred\n";
  for (const char* symbol : TRACE_VIEWPORT_FUNCTIONS)
    installTraceViewportHook(symbol);
  traceViewportDimensions("before-init");
  hostReport("[trace] viewport pronto: %s; pressione F9 para iniciar/parar o registro (limite %ld quadros por captura).\n",
          uniquePath.string().c_str(), traceViewportFrames);
  return true;
}

static u32 resolveImport(const std::string& name) {
  if (name == "__stack_chk_guard") return GUARD;
  auto it = importId.find(name);
  if (it != importId.end()) return stubAddr(it->second);
  if (nextMissing>=STUB_COUNT) throw std::runtime_error("Limite de imports ELF excedido");
  u32 id = nextMissing++;
  stubSet(id, "FALTANDO:" + name, [name] { hostReport("[host] a .so chamou a funcao nao implementada: %s\n", name.c_str()); fatal = true; uc_emu_stop(uc); });
  importId[name] = id;
  hostReport("[host] aviso: importacao sem implementacao: %s\n", name.c_str());
  return stubAddr(id);
}

static bool loadSo(const std::string& path) {
  try {
    if (fs::file_size(path) > destinia::ArmElfImage::maxSize) throw std::runtime_error("arquivo excede 64 MiB");
    std::vector<uint8_t> file;
    if (!readFile(path, file)) throw std::runtime_error("nao foi possivel abrir a biblioteca");
    destinia::ArmElfImage image; image.parse(file);
    for (size_t i=0;i<image.symbols.size();++i) {
      const auto& s=image.symbols[i];
      if (s.shndx!=0 && s.name) exportsMap[image.names[i]]=SO_BASE+s.value;
    }
    for (const auto& r:image.relocations) {
      u32 value=0; std::memcpy(&value,image.bytes.data()+r.offset,4);
      if (r.type==23) value+=SO_BASE;
      else if (r.type==21 || r.type==22 || r.type==2) {
        const auto& s=image.symbols[r.symbol];
        const u32 address=s.shndx!=0 ? SO_BASE+s.value : resolveImport(image.names[r.symbol]);
        value=r.type==2 ? value+address : address;
      } else { hostReport("[host] aviso: relocacao tipo %u ignorada\n",r.type); continue; }
      std::memcpy(image.bytes.data()+r.offset,&value,4);
    }
    const u32 span=static_cast<u32>(image.bytes.size());
    if (uc_mem_map(uc,SO_BASE,span,UC_PROT_ALL)!=UC_ERR_OK || !wr(SO_BASE,image.bytes.data(),span)) {
      hostReport("[host] Falha ao mapear/carregar a .so\n"); return false;
    }
    loadedSoSpan=span;
    hostReport("[host] .so carregada: %u KB, %zu simbolos exportados\n",span/1024,exportsMap.size());
    return true;
  } catch (const std::exception& error) {
    hostReport("[host] ELF invalido ou indisponivel: %s\n",error.what()); return false;
  }
}

// ---------------------------------------------------------------- execucao
static void dumpState(const char* why) {
  hostReport("\n=== PROBLEMA: %s ===\n", why);
  u32 pc = regPC();
  hostReport("pc=0x%08x (na .so: 0x%x, no Ghidra: 0x%x)  lr=0x%08x  sp=0x%08x\n", pc, pc - SO_BASE, pc - SO_BASE + 0x10000, regLR(), regSP());
  for (int i = 0; i < 13; i++) hostReport("r%d=0x%08x%s", i, R(i), (i % 4 == 3) ? "\n" : "  ");
  hostReport("\nultimas chamadas ao host:\n");
  for (auto& s : ring) hostReport("  %s\n", s.c_str());
  hostReport("Copie tudo isto e mande para o Claude.\n");
}

static void stubHook(uc_engine*, uint64_t address, uint32_t, void*) {
  if (address == MAGIC_RET) { hitMagic = true; uc_emu_stop(uc); return; }
  u32 id = ((u32)address - STUB_BASE) / 4;
  if (id < STUB_COUNT && handlers[id]) {
    try { handlers[id](); }
    catch(const std::exception& error) {
      fatal=true; uc_emu_stop(uc);
      hostReport("[host] ERRO no stub %s: %s\n",stubName[id].c_str(),error.what());
    }
  } else { hostReport("[host] stub sem handler id=0x%x\n", id); fatal = true; uc_emu_stop(uc); }
}

static bool codexDrainNativeMenu();
static bool callFn(u32 addr, std::initializer_list<u32> args, u32* ret) {
  struct DifficultyCallGuard{~DifficultyCallGuard(){difficultyEndCall();difficultyArm.endCall();}} difficultyGuard;
  destinia::SaveCall saveCall(saveTransaction);
  std::vector<u32> a(args);
  u32 sp = STACK_BASE + STACK_SIZE - 0x100;
  size_t extra = a.size() > 4 ? a.size() - 4 : 0;
  sp -= (u32)(extra * 4); sp &= ~7u;
  for (size_t i = 0; i < extra; i++) wr32(sp + 4 * (u32)i, a[4 + i]);
  for (int i = 0; i < 4; i++) W(i, i < (int)a.size() ? a[i] : 0);
  uc_reg_write(uc, UC_ARM_REG_SP, &sp);
  u32 lr = MAGIC_RET; uc_reg_write(uc, UC_ARM_REG_LR, &lr);
  hitMagic = false; fatal = false;
  uc_err e = uc_emu_start(uc, addr, MAGIC_RET, 5 * 1000 * 1000 /*5 s*/, 0);
  if(!difficultyArm.error.empty()){hostReport("[dificuldade ARM] %s\n",difficultyArm.error.c_str());executionFailed=true;return false;}
  if (e != UC_ERR_OK) { dumpState(uc_strerror(e)); executionFailed=true; destinia::sessionLog.write("ERRO de emulacao",true); return false; }
  if (fatal) { dumpState("chamada fatal"); executionFailed=true; destinia::sessionLog.write("ERRO de emulacao",true); return false; }
  if (!hitMagic && (regPC() & ~1u) == MAGIC_RET) hitMagic = true;  // o Unicorn para ANTES de executar o endereco final
  if (!hitMagic) { dumpState("tempo esgotado (a .so ficou presa num laco ou parou)"); executionFailed=true; destinia::sessionLog.write("ERRO de emulacao",true); return false; }
  if (ret) *ret = R(0);
  try {saveCall.commit();for(const auto& path:saveTransaction.committedRemovals())codexFileRemoved(path.string());}
  catch(const std::exception& error) {hostReport("[host] Save nao publicado: %s\n",error.what());executionFailed=true;return false;}
  if(!bossMage.error.empty()){hostReport("[mago] Erro ARM: %s\n",bossMage.error.c_str());executionFailed=true;return false;}
  bossMage.dispatch();
  return codexDrainNativeMenu();
}

// ---------------------------------------------------------------- BMP (para o modo sem janela)
static void rgb565ToRgba(const uint8_t* src, uint8_t* dst) {
  for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
    uint16_t p = src[2 * i] | (src[2 * i + 1] << 8);
    dst[4 * i + 0] = ((p >> 11) & 31) * 255 / 31; dst[4 * i + 1] = ((p >> 5) & 63) * 255 / 63; dst[4 * i + 2] = (p & 31) * 255 / 31; dst[4 * i + 3] = 255;
  }
}
static bool writeBmp(const std::string& path, const uint8_t* fb565) {
  std::vector<uint8_t> rgba(SCREEN_W * SCREEN_H * 4); rgb565ToRgba(fb565, rgba.data());
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;
  u32 rowBytes = SCREEN_W * 3, size = 54 + rowBytes * SCREEN_H, zero = 0, off = 54, hdr = 40, planesBpp = (24u << 16) | 1u;
  int w = SCREEN_W, h = SCREEN_H;
  f.write("BM", 2); f.write((char*)&size, 4); f.write((char*)&zero, 4); f.write((char*)&off, 4);
  f.write((char*)&hdr, 4); f.write((char*)&w, 4); f.write((char*)&h, 4); f.write((char*)&planesBpp, 4);
  for (int i = 0; i < 6; i++) f.write((char*)&zero, 4);
  for (int y = SCREEN_H - 1; y >= 0; y--)
    for (int x = 0; x < SCREEN_W; x++) { const uint8_t* p = &rgba[4 * (y * SCREEN_W + x)]; uint8_t bgr[3] = {p[2], p[1], p[0]}; f.write((char*)bgr, 3); }
  f.flush();
  return static_cast<bool>(f);
}

static bool captureTraceViewportFrame(const std::vector<uint8_t>& frameBuffer, long frame) {
  if (frameBuffer.size() < FB_BYTES) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " framebuffer_bmp_error=incomplete-buffer\n";
    hostReport("[trace] BMP ignorado: framebuffer incompleto (%zu de %u bytes).\n",
            frameBuffer.size(), FB_BYTES);
    return false;
  }

  static unsigned long long captureSequence = 0;
  const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::error_code ec;
  fs::path path;
  while (true) {
    path = fs::path(traceViewportOutDir) /
        ("viewport_frame_" + std::to_string(frame) + "_" + std::to_string(micros) + "_" +
         std::to_string(captureSequence++) + ".bmp");
    if (!fs::exists(path, ec)) break;
    if (ec) {
      traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                       << " frame=" << frame << " framebuffer_bmp_error="
                       << ec.message() << "\n";
      hostReport("[trace] nao consegui verificar destino do BMP %s: %s\n",
              path.string().c_str(), ec.message().c_str());
      return false;
    }
  }
  if (ec) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " framebuffer_bmp_error="
                     << ec.message() << "\n";
    hostReport("[trace] nao consegui verificar destino do BMP %s: %s\n",
            path.string().c_str(), ec.message().c_str());
    return false;
  }
  if (!writeBmp(path.string(), frameBuffer.data())) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " framebuffer_bmp_error=write-failed path="
                     << path.string() << "\n";
    hostReport("[trace] falha ao gravar BMP %s\n", path.string().c_str());
    return false;
  }

  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " frame=" << frame << " framebuffer_bmp=" << path.string() << "\n";
  hostReport("[trace] framebuffer salvo: %s\n", path.string().c_str());
  return true;
}

static void toggleTraceViewportRecording(long frame, const std::vector<uint8_t>& frameBuffer) {
  if (!traceViewport || !traceViewportLog.is_open()) return;
  traceViewportFrame = frame;
  if (traceViewportRecording) {
    stopTraceViewportRecording("F9");
    return;
  }

  traceViewportRecording = true;
  traceViewportStartFrame = frame;
  traceViewportTotals.clear();
  traceViewportFrameCounts.clear();
  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " frame=" << frame << " recording started\n";
  traceViewportDimensions("recording-start");
  captureTraceViewportFrame(frameBuffer, frame);
  traceViewportLog.flush();
  hostReport("[trace] viewport recording started at frame %ld.\n", frame);
}

static uint16_t read565(const uint8_t* pixels, int x, int y) {
  const size_t offset = 2 * (size_t)(y * SCREEN_W + x);
  return (uint16_t)pixels[offset] | ((uint16_t)pixels[offset + 1] << 8);
}

static int getPlayerStatusHudScale(const uint8_t* pixels) {
  for (int scale : {2, 1}) {
    int redPixels = 0, bluePixels = 0;
    for (int y = 8 * scale; y <= 18 * scale; ++y) {
      for (int x = 19 * scale; x <= 51 * scale; ++x) {
        const uint16_t color = read565(pixels, x, y);
        const int red = ((color >> 11) & 31) * 255 / 31;
        const int green = ((color >> 5) & 63) * 255 / 63;
        const int blue = (color & 31) * 255 / 31;
        if (y <= 12 * scale && red > 90 && red * 10 > green * 13 && red * 10 > blue * 12)
          ++redPixels;
        if (y >= 13 * scale && blue > 90 && blue * 10 > red * 12 && blue * 10 > green * 9)
          ++bluePixels;
      }
    }
    const int minPixels = 4 * scale * scale;
    if (redPixels >= minPixels && bluePixels >= minPixels) return scale;
  }
  return 0;
}

static void write565(uint8_t* pixels, int x, int y, uint16_t color) {
  const size_t offset = 2 * (size_t)(y * SCREEN_W + x);
  pixels[offset] = (uint8_t)color;
  pixels[offset + 1] = (uint8_t)(color >> 8);
}

static void drawHudExperienceText(uint8_t* pixels, uint32_t current, uint32_t required) {
  if (required == 0) return;

  char text[64];
  const double percentage = 100.0 * current / required;
  snprintf(text, sizeof(text), "%u / %u (%.2f%%)", current, required, percentage);
  for (char* c = text; *c; ++c) if (*c == '.') *c = ',';

  static const uint8_t digits[10][7] = {
      {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
      {14, 17, 1, 2, 4, 8, 31}, {30, 1, 1, 14, 1, 1, 30},
      {2, 6, 10, 18, 31, 2, 2}, {31, 16, 16, 30, 1, 1, 30},
      {6, 8, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
      {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 2, 12}};
  auto rowBits = [&](char c, int row) -> uint8_t {
    if (c >= '0' && c <= '9') return digits[c - '0'][row];
    switch (c) {
      case '/': { static const uint8_t glyph[] = {1, 2, 2, 4, 8, 8, 16}; return glyph[row]; }
      case ',': { static const uint8_t glyph[] = {0, 0, 0, 0, 0, 4, 8}; return glyph[row]; }
      case '(': { static const uint8_t glyph[] = {2, 4, 8, 8, 8, 4, 2}; return glyph[row]; }
      case ')': { static const uint8_t glyph[] = {8, 4, 2, 2, 2, 4, 8}; return glyph[row]; }
      case '%': { static const uint8_t glyph[] = {17, 2, 4, 8, 17, 0, 0}; return glyph[row]; }
      default: return 0;
    }
  };

  const int targetScale = std::max(2, SCREEN_W / 200);
  const int scale = std::max(1, std::min(targetScale / 2, (SCREEN_W - 8) / ((int)strlen(text) * 6)));
  const int textWidth = (int)strlen(text) * 6 * scale - scale;
  const int textHeight = 7 * scale;
  const int left = (SCREEN_W - textWidth) / 2;
  const int top = SCREEN_H - 11 * targetScale;
  const uint16_t panel = (uint16_t)((2u << 11) | (5u << 5) | 7u);
  const uint16_t border = (uint16_t)((8u << 11) | (40u << 5) | 12u);
  const uint16_t ink = (uint16_t)((31u << 11) | (61u << 5) | 28u);
  for (int y = top - 2; y < top + textHeight + 2; ++y) {
    for (int x = left - 2; x < left + textWidth + 2; ++x) {
      if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H - scale) continue;
      write565(pixels, x, y, (x == left - 2 || x == left + textWidth + 1 ||
                              y == top - 2 || y == top + textHeight + 1) ? border : panel);
    }
  }

  int cursor = left;
  for (const char* c = text; *c; ++c, cursor += 6 * scale) {
    for (int row = 0; row < 7; ++row) {
      const uint8_t bits = rowBits(*c, row);
      for (int col = 0; col < 5; ++col) {
        if (!(bits & (1u << (4 - col)))) continue;
        for (int dy = 0; dy < scale; ++dy)
          for (int dx = 0; dx < scale; ++dx)
            write565(pixels, cursor + col * scale + dx, top + row * scale + dy, ink);
      }
    }
  }
}

static bool getCharacterExperience(uint32_t& current, uint32_t& required) {
  auto game = exportsMap.find("Game");
  if (game == exportsMap.end()) return false;

  const uint32_t level = rd16(game->second + 0x48FA);
  if (level == 0 || level > 99) return false;
  if (level == 99) {
    current = required = 0;
    return true;
  }
  const uint64_t adjusted = level + 2u + (uint64_t)level * level / 10u;
  const uint64_t threshold = adjusted * adjusted + 20u;
  if (threshold == 0 || threshold > UINT32_MAX) return false;

  current = rd32(game->second + 0x48FC);
  required = (uint32_t)threshold;
  return true;
}

static void difficultyBossDefeated(unsigned slot,int species,u32 p);
static void difficultyNewMap();
static void difficultyRespawn(u32 p);
static void difficultyFileRemoved(const std::string& path);
#include "../common/codex_host.h"
#include "../common/codex_ui.h"

static void runtimeLogTick(long frame,double emulationMs) {
  static double elapsed=0; static long measured=0; static std::string previous;
  elapsed+=emulationMs; ++measured;
  if (frame==1 || frame%16==0) {
    std::string state="menu="+std::to_string(menuState());
    if (gMMain) state+=" rede="+std::to_string(rd16(gMMain+14624));
    auto game=exportsMap.find("Game");
    if (game!=exportsMap.end()) state+=" nivel="+std::to_string(rd16(game->second+0x48FA))+" xp="+std::to_string(rd32(game->second+0x48FC));
    auto map=exportsMap.find("curMapData");
    if (map!=exportsMap.end()) state+=" mapa_tiles="+std::to_string(rd16(map->second))+"x"+std::to_string(rd16(map->second+2));
    if (state!=previous) { destinia::sessionLog.write("ESTADO observado: "+state,true); previous=state; }
  }
  if (frame%80==0) {
    destinia::sessionLog.write("DESEMPENHO quadro="+std::to_string(frame)+" emulacao_media_ms="+std::to_string(elapsed/measured),true);
    elapsed=0; measured=0;
  }
}

// ---------------------------------------------------------------- principal
#include "../common/offline_store_host.h"
#include "../common/combat_power_host.h"
#include "../common/inventory_host.h"
#include "../common/codex_native_host.h"
#include "../common/difficulty_host.h"
#include "../common/weapons_catalog_host.h"
#include "../common/shared_stash_host.h"
#include "../common/bounties_host.h"

static int runHost(int argc, char** argv) {
  std::string languageOverride;
  int fullscreenOverride=-1; bool explicitScale=false,savePreferences=true;
  fs::path settingsPath=destinia::preferencesFile();
  int scale = 3, headlessFrames = 0; std::string outDir = "frames", reportsDir;
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    auto next = [&]() { if(i+1>=argc) throw std::runtime_error("Argumento sem valor: "+a); return std::string(argv[++i]); };
    if (a == "--game") gameDir = next(); else if (a == "--so") soPath = next(); else if (a == "--save") saveDir = next();
    else if (a == "--scale") { scale = atoi(next().c_str()); explicitScale=true; } else if (a == "--headless") headlessFrames = atoi(next().c_str());
    else if (a == "--reports") reportsDir = next();
    else if (a == "--settings") settingsPath=fs::absolute(next());
    else if (a == "--no-save-preferences") savePreferences=false;
    else if (a == "--fullscreen") fullscreenOverride=1;
    else if (a == "--windowed") fullscreenOverride=0;
    else if (a == "--out") outDir = next(); else if (a == "-v") verbose = true;
    else if (a == "--probe") probe = true;
    else if (a == "--trace-camera") traceCamera = true;
    else if (a == "--no-fix-map-edge") fixMapEdge = false;
    else if (a == "--profile") profile = true;
    else if (a == "--mute") muteAudio = true;
    else if (a == "--purchase-cancel-test") purchaseCancelTest = true;
    else if (a == "--lang") {languageOverride=next();if(languageOverride!="pt"&&languageOverride!="en")throw std::runtime_error("Idioma deve ser pt ou en.");}
    else if (a == "--trace-viewport") traceViewport = true;
    else if (a == "--trace-frames") {
      const std::string value = next();
      char* end = nullptr;
      const long parsed = std::strtol(value.c_str(), &end, 10);
      if (value.empty() || !end || *end || parsed < 0) { printf("--trace-frames precisa de um inteiro >= 0\n"); return 0; }
      traceViewportFrames = parsed;
    }
    else if (a == "--trace-out") traceViewportOutDir = next();
    else if (a == "--autotap") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame, &autoX, &autoY) != 3) { printf("--autotap precisa de QUADRO,X,Y\n"); return 0; } }
    else if (a == "--autotap2") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame2, &autoX2, &autoY2) != 3) { printf("--autotap2 precisa de QUADRO,X,Y\n"); return 0; } }
    else { printf("Uso: destinia_host [--game PASTA] [--so ARQUIVO] [--save PASTA] [--reports PASTA] [--settings ARQUIVO] [--no-save-preferences] [--windowed|--fullscreen] [--scale N] [--fullscreen] [--lang pt|en] [--headless QUADROS --out PASTA] [--trace-viewport --trace-frames N --trace-out PASTA] [-v] [--probe] [--profile] [--mute] [--autotap QUADRO,X,Y] [--autotap2 QUADRO,X,Y]\n"); return 0; }
  }
  preferences=destinia::Preferences::load(settingsPath,"host_zoom");
  portugueseTextTest=(languageOverride.empty()?preferences.language:languageOverride)=="pt";
  bool fullscreen=fullscreenOverride<0?preferences.fullscreen:fullscreenOverride!=0;
  int windowWidth=preferences.width,windowHeight=preferences.height;
  if(explicitScale) { scale=std::clamp(scale,1,10); windowWidth=400*scale; windowHeight=240*scale; }
  const fs::path reports=reportsDir.empty()?destinia::defaultReportsDir():fs::path(reportsDir);
  if(destinia::sessionLog.start(reports,"host_zoom")) hostReport("[host] Log TXT: %s\n",(destinia::sessionLog.directory()/"log.txt").string().c_str());
  hostReport("[host] Destinia 0.10.3 Beta - host_zoom\n");
  if(saveDir.empty()) saveDir=destinia::modeSaveDir(destinia::configuredSaveDir(),preferences.gameMode).string();
  saveDir=fs::absolute(saveDir).lexically_normal().string();
  hostReport("[host] Save compartilhado: %s\n",saveDir.c_str());
  if (soPath.empty()) soPath = gameDir + "/lib/armeabi/libdestinia_jni.so";
  assetsDir = gameDir + "/assets";
  fs::create_directories(saveDir);
  if(!saveLock.acquire(fs::path(saveDir)/".destinia.lock")) {
    hostReport("[host] Nao foi possivel travar a pasta de save: outro jogo pode estar aberto ou falta acesso.\n"); return 1;
  }
  destinia::verifySaveMode(saveDir,preferences.gameMode);
  hostReport("[host] Modo de jogo: %s\n",preferences.gameMode.c_str());
  saveTransaction.configure(saveDir);
  saveTransaction.recover();
  const auto backup=destinia::backupSave(saveDir);
  hostReport("[host] Backup: %s\n",backup.empty()?"nenhum progresso anterior para copiar":backup.string().c_str());

  if (uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc) != UC_ERR_OK) { hostReport("uc_open falhou\n"); return 1; }
  uc_mem_map(uc, HEAP_BASE, HEAP_SIZE, UC_PROT_ALL);
  uc_mem_map(uc, STUB_BASE, STUB_COUNT * 4, UC_PROT_ALL);
  uc_mem_map(uc, MAGIC_RET & ~0xFFFu, 0x1000, UC_PROT_ALL);
  uc_mem_map(uc, DATA_BASE, DATA_SIZE, UC_PROT_ALL);
  uc_mem_map(uc, STACK_BASE, STACK_SIZE, UC_PROT_ALL);

  std::vector<u32> bx(STUB_COUNT, 0xE12FFF1E);  // "bx lr" em cada stub
  wr(STUB_BASE, bx.data(), bx.size() * 4);
  u32 bxlr = 0xE12FFF1E; wr(MAGIC_RET, &bxlr, 4);

  setupStubs();
  // JNIEnv: ENV_OBJ -> tabela de ponteiros; todas as entradas apontam para stubs
  wr32(ENV_OBJ, ENV_TAB);
  for (int i = 0; i < JNI_FUNCS; i++) {
    u32 id = ID_JNI + i;
    if (!handlers[id]) { stubSet(id, "JNI.#" + std::to_string(i), [i] { hostReport("[host] JNI nao implementado: indice %d\n", i); fatal = true; uc_emu_stop(uc); }); }
    wr32(ENV_TAB + 4 * i, stubAddr(id));
  }
  wr32(VM_OBJ, VM_TAB);
  for (int i = 0; i < 8; i++) {
    u32 id = ID_VM + i;
    if (!handlers[id]) stubSet(id, "VM.#" + std::to_string(i), [i] { hostReport("[host] JavaVM nao implementado: indice %d\n", i); fatal = true; uc_emu_stop(uc); });
    wr32(VM_TAB + 4 * i, stubAddr(id));
  }
  wr32(GUARD, 0xA5C3F00D);

  if (!loadSo(soPath)) return 1;
  if(!destinia::installVirtualControls(uc,SO_BASE,soPath,preferences.virtualControls)) {
    hostReport("[host] Nao foi possivel ocultar controles: biblioteca ARM incompativel ou falha nos hooks.\n");return 1;
  }
  hostReport("[host] Controles virtuais durante gameplay: %s\n",preferences.virtualControls?"visiveis":"ocultos (teclado/controle e menus preservados)");
  uc_hook hk; uc_hook_add(uc, &hk, UC_HOOK_CODE, (void*)stubHook, nullptr, STUB_BASE, STUB_BASE + STUB_COUNT * 4 - 1);
  uc_hook hk2; uc_hook_add(uc, &hk2, UC_HOOK_CODE, (void*)stubHook, nullptr, MAGIC_RET, MAGIC_RET + 3);
  if (exportsMap["Game"] != SO_BASE + 0x77EE0 || exportsMap["curMapData"] != SO_BASE + 0x80B18) {
    hostReport("[host] Layout ARM incompativel com a camera do zoom.\n"); return 1;
  }
  // Exact APK 1.0.6 layout: all 16 camera STRH completion PCs were disassembled.
  // Fail explicitly on another binary rather than install hooks at unknown code.
  std::vector<uint8_t> cameraLibrary;
  if (!readFile(soPath, cameraLibrary)) {hostReport("[host] Nao foi possivel verificar a camera ARM.\n");return 1;}
  uint64_t cameraFingerprint=14695981039346656037ull;
  for(auto byte:cameraLibrary) cameraFingerprint=(cameraFingerprint^byte)*1099511628211ull;
  if(cameraFingerprint!=0x74b2547839b88777ull) {
    hostReport("[host] Biblioteca ARM incompativel com a centralizacao verificada do zoom (APK 1.0.6).\n");return 1;
  }
  const u32 cameraWriterPCs[]={0x11b96,0x11bc8,0x14e64,0x14eb8,0x14ee4,0x14f0c,0x14f7a,0x14f8e,
                            0x19666,0x1966e,0x19d32,0x19d3c,0x1e3aa,0x1e3ac,0x1f92e,0x1f94c};
  for(auto pc:cameraWriterPCs) {
    uc_hook hook;
    if(uc_hook_add(uc,&hook,UC_HOOK_CODE,(void*)cameraWriteHook,nullptr,SO_BASE+pc,SO_BASE+pc)!=UC_ERR_OK) {
      hostReport("[host] Falha ao registrar centralizacao da camera.\n");return 1;
    }
  }
  if (fixMapEdge) {
    uc_hook hk3, hk4;
    uc_hook_add(uc, &hk3, UC_HOOK_CODE, (void*)mapEdgeHook, nullptr, SO_BASE + 0xBB90, SO_BASE + 0xBB90);
    uc_hook_add(uc, &hk4, UC_HOOK_CODE, (void*)mapEdgeHook, nullptr, SO_BASE + 0xB4BC, SO_BASE + 0xB4BC);
    uc_hook hk5;
    uc_hook_add(uc, &hk5, UC_HOOK_CODE, (void*)mapEdgeClearHook, nullptr, SO_BASE + 0x19308, SO_BASE + 0x19308);
  }

  for (const char* n : {"Java_game_destiniaeng_GameThread_initGame", "Java_game_destiniaeng_GameThread_jniRun", "Java_game_destiniaeng_GameThread_jniTouch"})
    if (!exportsMap.count(n)) { hostReport("Simbolo nao encontrado: %s\n", n); return 1; }
  if (traceViewport && !startTraceViewport()) return 1;
  u32 fnInit = exportsMap["Java_game_destiniaeng_GameThread_initGame"];
  uiRenderer.statusBaseScale=1;
  uiRenderer.install(uc,SO_BASE,exportsMap,SCREEN_W,SCREEN_H,preferences.uiScale,false,soPath);
  dropNamesRenderer.install(uc,SO_BASE,exportsMap,SCREEN_W,SCREEN_H,soPath);
  {std::vector<uint8_t> table;if(!readFile(assetsDir+"/item_en.gdt",table))throw std::runtime_error("Tabela de loot ausente");lootQol.install(uc,SO_BASE,exportsMap,SCREEN_W,SCREEN_H,soPath,table);lootQol.options=preferences.loot;}
  try{bossMage.install(uc,SO_BASE,exportsMap,SCREEN_W,SCREEN_H,assetsDir,soPath,[]{return lootQol.autoTarget<0;});
    bossMage.menu.opener = [&]{ bossRushUI.openMenu(); };
    bossRushUI.getGold = [&]() -> int { return int(callFn(exportsMap["getMyGold"], {})); };
    bossRushUI.setGold = [&](int g) { callFn(exportsMap["setMyGold"], {uint32_t(g)}); };
    bossRushUI.teleport = [&](int a, int m, int e) { callFn(exportsMap["Map_loadData"], {uint32_t(a), uint32_t(m), uint32_t(e)}); };
  }catch(const std::exception& error){hostReport("[mago] Desativado: %s\n",error.what());}
  fcInstall();
  u32 fnRun = exportsMap["Java_game_destiniaeng_GameThread_jniRun"];
  u32 fnTouch = exportsMap["Java_game_destiniaeng_GameThread_jniTouch"];
  auto grab = [&](const char* n) { auto it = exportsMap.find(n); return it == exportsMap.end() ? 0u : it->second; };
  fnLogoPress = grab("LogoCanvas_mKeyPressed");
  fnMenuPress = grab("MenuCanvas_mKeyPressed"); fnMenuRelease = grab("MenuCanvas_mKeyReleased");
  fnGamePress = grab("GameCanvas_mKeyPressed"); fnGameRelease = grab("GameCanvas_mKeyReleased");
  fnGetCash = grab("getMyCash"); fnSetCash = grab("setMyCash");
  fnProcCommMsg = grab("Java_game_destiniaeng_GameThread_procCommMsg");

  // [OK] initGame(env,obj, byte[] deviceID,int len, byte[] modelo,int len, w, h, isKorean)
  // isKorean = 0 -> a .so escolhe os arquivos *_en (ingles).
  std::string devId = "DESTINIA-PC-0001", model = "PC";
  u32 hId = nextArr++, hModel = nextArr++, hScreen = nextArr++;
  arrays[hId].assign(devId.begin(), devId.end());
  arrays[hModel].assign(model.begin(), model.end());
  arrays[hScreen].assign(FB_BYTES, 0);
  if (!callFn(fnInit, {ENV_OBJ, 0x10, hId, (u32)devId.size(), hModel, (u32)model.size(), SCREEN_W, SCREEN_H, 0})) return 2;
  if (traceViewport) traceViewportDimensions("after-init");
  installCodex();
  installDifficulty();
  if(!difficultyReady){hostReport("[dificuldade ARM] Inicializacao recusada; progresso preservado.\n");return 1;}
  difficultyArm.install(uc,SO_BASE,exportsMap,soPath,[](unsigned slot){return difficultyProgress.current(slot);});
  installInventoryProtection();
  destinia::initLootEconomy(saveDir);
  weaponsCatalogHost.init(fs::path(saveDir));
  sharedStashHost.init(fs::path(saveDir));
  bountiesHost.init(fs::path(saveDir));
  g_onMonsterDefeated = [&](int s, int id){ bountiesHost.onMonsterKilled(s, id); };
  hostReport("[host] initGame ok em %dx%d. Rodando o jogo...\n", SCREEN_W, SCREEN_H);
  hostReport("[host] zoom out: mapas pequenos centralizados; fundo cinza; UI nativa.\n");
  if (portugueseTextTest) hostReport("[host] traducao PT ativa: itens, equipamentos, habilidades, menus, Fabricacao e Missoes.\n");

  // [OK] jniTouch(env,obj, x, y, tipo): tipo 0=toque 1=solta 2=move 3=tecla de menu (Thumb decodificado)
  bool mouseDown=false; int lastTouchX=0,lastTouchY=0;
  auto touch = [&](int x, int y, int type) {
    if(sharedStashHost.open || bountiesHost.open || weaponsCatalogHost.open || difficultyMenuTouch(x,y,type) || codexMenuTouch(x,y,type)) return;
    lastTouchX=x; lastTouchY=y;
    if(type!=2) destinia::sessionLog.write("ENTRADA toque x="+std::to_string(x)+" y="+std::to_string(y)+" tipo="+std::to_string(type));
    if(preferences.uiScale!=100&&type!=3){u32 state=0;if(!callFn(exportsMap.at("_getTouchState"),{},&state))return;auto point=uiRenderer.touchPoint(x,y,int(state));x=point.first;y=point.second;}
    bool ok = callFn(fnTouch, {ENV_OBJ, 0x10, (u32)x, (u32)y, (u32)type});
    if (probe && type != 2) { hostReport("[sonda] jniTouch(x=%d, y=%d, tipo=%d) -> %s\n", x, y, type, ok ? "ok" : "FALHOU"); probeState(type == 0 ? "logo apos PRESSIONAR" : type == 1 ? "logo apos SOLTAR" : "logo apos tecla"); }
  };

  SDL_Window* win = nullptr; SDL_Renderer* ren = nullptr; SDL_Texture* tex = nullptr; SDL_GameController* controller = nullptr;
  auto releaseGraphics=[&]() {
    if (audioReady) {
#ifdef HAS_SDL_MIXER
      Mix_HaltMusic();
      for (auto& p:sfxCache) Mix_FreeChunk(p.second);
      for (auto& p:musCache) Mix_FreeMusic(p.second);
      sfxCache.clear(); musCache.clear();
      Mix_CloseAudio(); Mix_Quit();
#endif
      audioReady=false;
    }
    if (controller) SDL_GameControllerClose(controller);
    if (tex) SDL_DestroyTexture(tex);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    if (!headlessFrames) SDL_Quit();
  };
  struct GraphicsGuard { std::function<void()> release; ~GraphicsGuard() { release(); } } graphicsGuard{releaseGraphics};

  auto setFullscreen = [&](bool enabled) {
    if(enabled && !(SDL_GetWindowFlags(win)&SDL_WINDOW_FULLSCREEN)) SDL_GetWindowSize(win,&windowWidth,&windowHeight);
    if(SDL_SetWindowFullscreen(win,enabled?SDL_WINDOW_FULLSCREEN_DESKTOP:0)!=0) {
      fullscreen=(SDL_GetWindowFlags(win)&SDL_WINDOW_FULLSCREEN)!=0;
      hostReport("[host] Nao foi possivel alternar tela cheia: %s\n",SDL_GetError()); return false;
    }
    fullscreen=enabled;
    if(!enabled) SDL_SetWindowSize(win,windowWidth,windowHeight);
    hostReport("[host] Exibicao: %s.\n",enabled?"tela cheia":"janela"); return true;
  };
  if (!headlessFrames) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0) { hostReport("SDL_Init: %s\n", SDL_GetError()); return 1; }
    if (!muteAudio) {
#ifdef HAS_SDL_MIXER
      Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
      if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == 0) { Mix_AllocateChannels(16); audioReady = true; applyAudioPreferences(); }
      else hostReport("[host] audio indisponivel: %s\n", Mix_GetError());
#else
      hostReport("[host] audio indisponivel: compile com SDL2_mixer para habilitar o som.\n");
#endif
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // pixels nitidos
    win = SDL_CreateWindow("Destinia 0.10.3 Beta", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if(!win) { hostReport("SDL_CreateWindow: %s\n",SDL_GetError()); return 1; }
    if (fullscreen) setFullscreen(true);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
    if(!ren) { hostReport("SDL_CreateRenderer: %s\n",SDL_GetError()); return 1; }
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if(!tex) { hostReport("SDL_CreateTexture: %s\n",SDL_GetError()); return 1; }
    for (int i = 0; i < SDL_NumJoysticks(); i++) { if (SDL_IsGameController(i)) { controller = SDL_GameControllerOpen(i); if (controller) { hostReport("[host] Controle conectado: %s\n", SDL_GameControllerName(controller)); break; } } }
    if (traceViewport)
      hostReport("[host] Teclado: WASD/setas mover, Enter/Z confirmar, Esc/X voltar, I menu, F5 save, F11 tela cheia, F9 iniciar/parar trace viewport. DualSense: analogico mover, X atacar, O voltar, triangulo menu, Share save, L1 acelerar.\n");
    else
      hostReport("[host] Teclado: WASD/setas mover, Enter/Z confirmar, Esc/X voltar, I menu, F5 save, F11 tela cheia. DualSense: analogico mover, X atacar, O voltar, triangulo menu, Share save, L1 acelerar.\n");
  } else fs::create_directories(outDir);

  auto gameViewport = [&]() {
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    if (ow <= 0 || oh <= 0) return SDL_Rect{0, 0, 1, 1};
    int width, height;
    if ((int64_t)ow * SCREEN_H > (int64_t)oh * SCREEN_W) {
      height = oh;
      width = std::max(1, (int)((int64_t)oh * SCREEN_W / SCREEN_H));
    } else {
      width = ow;
      height = std::max(1, (int)((int64_t)ow * SCREEN_H / SCREEN_W));
    }
    return SDL_Rect{(ow - width) / 2, (oh - height) / 2, width, height};
  };
  auto mapMouseToGame = [&](int wx, int wy, int& x, int& y, bool clampOutside) {
    int ww = 0, wh = 0, ow = 0, oh = 0;
    SDL_GetWindowSize(win, &ww, &wh);
    SDL_GetRendererOutputSize(ren, &ow, &oh);
    if (ww <= 0 || wh <= 0 || ow <= 0 || oh <= 0) return false;
    const int ox = (int)((int64_t)wx * ow / ww);
    const int oy = (int)((int64_t)wy * oh / wh);
    const SDL_Rect vp = gameViewport();
    const bool inside = ox >= vp.x && ox < vp.x + vp.w && oy >= vp.y && oy < vp.y + vp.h;
    if (!inside && !clampOutside) return false;
    x = std::clamp((int)((int64_t)(ox - vp.x) * SCREEN_W / vp.w), 0, SCREEN_W - 1);
    y = std::clamp((int)((int64_t)(oy - vp.y) * SCREEN_H / vp.h), 0, SCREEN_H - 1);
    return inside;
  };

  destinia::PlayerControls controls(preferences,controller,gameKey,[&](){touch(0,0,3);},[&](){
    if(mouseDown) { touch(lastTouchX,lastTouchY,1); mouseDown=false; }
  },[&](){
    preferences.virtualControls=!preferences.virtualControls;
    destinia::setVirtualControlsVisible(preferences.virtualControls);
    hostReport("[host] L3+R3: controles virtuais %s.\n",preferences.virtualControls?"visiveis":"ocultos");
    if(savePreferences)try{destinia::saveVirtualControlsPreference(settingsPath,"host_zoom",preferences.virtualControls);}
    catch(const std::exception& error){hostReport("[host] Visibilidade aplicada somente nesta sessao: %s\n",error.what());}
  });
  bool wasPaused=false;
  bool running = true; long frame = 0;
  auto profileWindowStart = std::chrono::steady_clock::now();
  long profileWindowFrames = 0;
  double profileRunMs = 0.0, profileRenderMs = 0.0, profileSleepMs = 0.0, profileOtherMs = 0.0;
  double profileMaxRunMs = 0.0;
  while (running && !executionFailed) {
    traceViewportFrame = frame;
    auto t0 = std::chrono::steady_clock::now();
    double runMs = 0.0, renderMs = 0.0, sleepMeasuredMs = 0.0;
    if (!headlessFrames) {
      SDL_Event ev;
      controls.pollShortcuts();
      while (SDL_PollEvent(&ev)) {
        if(controls.touchShortcut(ev))continue;
        if(bossRushUI.open) {
          if(ev.type==SDL_CONTROLLERBUTTONDOWN && ev.cbutton.which==controls.controllerId()) {
            auto binding=preferences.buttons.find((SDL_GameControllerButton)ev.cbutton.button);
            if(binding!=preferences.buttons.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)bossRushUI.handleKey(k[a],true);}
          }
          if((ev.type==SDL_KEYDOWN) && !ev.key.repeat) {
            if(ev.key.keysym.sym==SDLK_ESCAPE)bossRushUI.handleKey(10,true);
            else if(ev.key.keysym.sym==SDLK_UP)bossRushUI.handleKey(3,true);
            else if(ev.key.keysym.sym==SDLK_DOWN)bossRushUI.handleKey(9,true);
            else if(ev.key.keysym.sym==SDLK_RETURN || ev.key.keysym.sym==SDLK_SPACE)bossRushUI.handleKey(17,true);
            else {auto binding=preferences.keys.find(ev.key.keysym.sym);if(binding!=preferences.keys.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)bossRushUI.handleKey(k[a],true);}}
          }
          continue;
        }
        if(codexOpen && (ev.type==SDL_CONTROLLERBUTTONDOWN || ev.type==SDL_CONTROLLERBUTTONUP || ev.type==SDL_CONTROLLERAXISMOTION)) {
          if(ev.type==SDL_CONTROLLERBUTTONDOWN && ev.cbutton.which==controls.controllerId()) {
            auto binding=preferences.buttons.find((SDL_GameControllerButton)ev.cbutton.button);
            if(binding!=preferences.buttons.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)codexMenuKey(k[a],true);}
          }
          continue;
        }
        if(ev.type==SDL_QUIT) running=false;
        else if(ev.type==SDL_WINDOWEVENT && ev.window.event==SDL_WINDOWEVENT_FOCUS_LOST){inventoryTouchCaptured=false;fcTouchCaptured=false;controls.event(ev);}
        else if(ev.type==SDL_KEYDOWN || ev.type==SDL_KEYUP) {
          if(ev.key.repeat) continue;
          const bool down=ev.type==SDL_KEYDOWN;
          if(ev.key.keysym.sym==SDLK_F6 && !preferences.keys.count(SDLK_F6) && (inventoryOpen||inventoryVisible())){if(down){controls.release();inventoryMenuKey(10016,true);}continue;}
          if(((ev.key.keysym.sym==SDLK_F7&&!preferences.keys.count(SDLK_F7))||(ev.key.keysym.sym==SDLK_p&&!preferences.keys.count(SDLK_p))) && (inventoryOpen||inventoryVisible())){if(down){controls.release();inventoryMenuKey(10014,true);}continue;}
          if(codexOpen && ev.key.keysym.sym!=SDLK_F11) {
            if(down) {
              if(ev.key.keysym.sym==SDLK_ESCAPE)codexMenuKey(10,true);
              else if(ev.key.keysym.sym==SDLK_RIGHT)codexMenuKey(7,true);
              else if(ev.key.keysym.sym==SDLK_LEFT)codexMenuKey(5,true);
              else if(ev.key.keysym.sym==SDLK_UP)codexMenuKey(3,true);
              else if(ev.key.keysym.sym==SDLK_DOWN)codexMenuKey(9,true);
              else if(ev.key.keysym.sym==SDLK_PAGEUP)codexChangePage(-1);
              else if(ev.key.keysym.sym==SDLK_PAGEDOWN)codexChangePage(1);
              else {auto binding=preferences.keys.find(ev.key.keysym.sym);if(binding!=preferences.keys.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)codexMenuKey(k[a],true);}}
            }
            continue;
          }
          if(ev.key.keysym.sym==SDLK_F4 || (ev.key.keysym.sym==SDLK_w && (SDL_GetModState() & KMOD_CTRL))) {
            if(down) { weaponsCatalogHost.toggle(rd16(exportsMap.at("Game")+0x4886), exportsMap.at("Game"), [](uint32_t a){return rd16(a);}); }
            continue;
          }
          if(weaponsCatalogHost.open) {
            if(down) weaponsCatalogHost.handleKey(ev.key.keysym.sym, true);
            continue;
          }
          if(ev.key.keysym.sym==SDLK_F5 || (ev.key.keysym.sym==SDLK_b && (SDL_GetModState() & KMOD_CTRL))) {
            if(down) { sharedStashHost.toggle(); }
            continue;
          }
          if(sharedStashHost.open) {
            if(down) sharedStashHost.handleKey(ev.key.keysym.sym, true, exportsMap.at("Game"), [](uint32_t a){return rd16(a);}, [](uint32_t a, uint16_t v){wr16(a, v);});
            continue;
          }
          if(ev.key.keysym.sym==SDLK_F8 || (ev.key.keysym.sym==SDLK_c && (SDL_GetModState() & KMOD_CTRL))) {
            if(down) { bountiesHost.toggle(); }
            continue;
          }
          if(bountiesHost.open) {
            if(down) bountiesHost.handleKey(ev.key.keysym.sym, true, rd16(exportsMap.at("Game")+0x4886), [&](uint32_t gold, uint16_t cash){
              if(fnGetCash && fnSetCash) {
                u32 curCash = 0; callFn(fnGetCash, {}, &curCash);
                callFn(fnSetCash, {curCash + cash});
              }
              u32 curGold = rd32(exportsMap.at("Game")+0x4894);
              wr32(exportsMap.at("Game")+0x4894, curGold + gold);
            });
            continue;
          }
          if(ev.key.keysym.sym==SDLK_F11) { if(down) setFullscreen(!fullscreen); }
          else if(ev.key.keysym.sym==SDLK_F9) { if(traceViewport && down) toggleTraceViewportRecording(frame,arrays[hScreen]); }
          else controls.event(ev);
        } else if(ev.type==SDL_MOUSEBUTTONDOWN || ev.type==SDL_MOUSEBUTTONUP || ev.type==SDL_MOUSEMOTION) {
          if(!controls.focused || (controls.paused() && !codexOpen)) continue;
          int wx=ev.type==SDL_MOUSEMOTION?ev.motion.x:ev.button.x,wy=ev.type==SDL_MOUSEMOTION?ev.motion.y:ev.button.y;
          int x=0,y=0;
          bool inside=mapMouseToGame(wx,wy,x,y,mouseDown && ev.type!=SDL_MOUSEBUTTONDOWN);
          if((inside || bossRushUI.open || codexTouchCaptured || inventoryTouchCaptured || fcTouchCaptured) && (ev.type==SDL_MOUSEBUTTONDOWN || ev.type==SDL_MOUSEBUTTONUP) && ev.button.button==SDL_BUTTON_LEFT) {
            if(weaponsCatalogHost.open && weaponsCatalogHost.handleTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1,SCREEN_W,SCREEN_H)){controls.release();continue;}
            if(sharedStashHost.open && sharedStashHost.handleTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1,SCREEN_W,SCREEN_H)){controls.release();continue;}
            if(bountiesHost.open && bountiesHost.handleTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1,SCREEN_W,SCREEN_H,rd16(exportsMap.at("Game")+0x4886),[&](uint32_t gold, uint16_t cash){
              if(fnGetCash && fnSetCash) {
                u32 curCash = 0; callFn(fnGetCash, {}, &curCash);
                callFn(fnSetCash, {curCash + cash});
              }
              u32 curGold = rd32(exportsMap.at("Game")+0x4894);
              wr32(exportsMap.at("Game")+0x4894, curGold + gold);
            })){controls.release();continue;}
            if(bossRushUI.handleTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1,SCREEN_W,SCREEN_H)){controls.release();continue;}
            if(inventoryMenuTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1)){controls.release();continue;}
            if(fcMenuTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1)){controls.release();continue;}
            if(difficultyMenuTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1) || codexMenuTouch(x,y,ev.type==SDL_MOUSEBUTTONDOWN?0:1)){controls.release();continue;}
          }
          if(codexOpen) continue;
          if(probe && ev.type!=SDL_MOUSEMOTION) {
            int ww=0,wh=0,ow=0,oh=0; SDL_GetWindowSize(win,&ww,&wh); SDL_GetRendererOutputSize(ren,&ow,&oh);
            hostReport("[sonda] SDL %s botao=%d janela_px=(%d,%d) tamanho_janela=%dx%d saida_render=%dx%d -> jogo=(%d,%d) dentro=%d\n",
                       ev.type==SDL_MOUSEBUTTONDOWN?"BAIXO":"CIMA",(int)ev.button.button,wx,wy,ww,wh,ow,oh,x,y,(int)inside);
          }
          if(ev.type==SDL_MOUSEBUTTONDOWN && ev.button.button==SDL_BUTTON_LEFT && inside) { controls.event(ev); mouseDown=true; touch(x,y,0); }
          else if(ev.type==SDL_MOUSEBUTTONUP && ev.button.button==SDL_BUTTON_LEFT && mouseDown) { mouseDown=false; touch(x,y,1); }
          else if(ev.type==SDL_MOUSEMOTION && mouseDown) touch(x,y,2);
        } else {
          if(probe && ev.type==SDL_CONTROLLERBUTTONDOWN && ev.cbutton.which==controls.controllerId())
            hostReport("[sonda] Controle BOTAO: %d\n",(int)ev.cbutton.button);
          controls.event(ev);
          if(ev.type==SDL_WINDOWEVENT && ev.window.event==SDL_WINDOWEVENT_SIZE_CHANGED && !fullscreen)
            SDL_GetWindowSize(win,&windowWidth,&windowHeight);
        }
      }
      if(inventoryOpen){controls.release();controls.pollSticks();updateAudioState(controls.focused,true);auto& panel=arrays[hScreen];inventoryPaint(panel.data());if(SDL_UpdateTexture(tex,nullptr,panel.data(),SCREEN_W*2)!=0){executionFailed=true;break;}SDL_Rect dst=gameViewport();SDL_SetRenderDrawColor(ren,0,0,0,255);SDL_RenderClear(ren);SDL_RenderCopy(ren,tex,nullptr,&dst);SDL_RenderPresent(ren);SDL_Delay(62);continue;}
      if(codexOpen) {
        controls.release();updateAudioState(controls.focused,true);
        auto panel=arrays[hScreen];nativeCodexPainter.paint(panel.data());
        if(SDL_UpdateTexture(tex,nullptr,panel.data(),SCREEN_W*2)!=0) {executionFailed=true;break;}
        SDL_Rect dst=gameViewport();SDL_SetRenderDrawColor(ren,0,0,0,255);SDL_RenderClear(ren);SDL_RenderCopy(ren,tex,nullptr,&dst);SDL_RenderPresent(ren);SDL_Delay(62);continue;
      }
      controls.pollSticks(); fastForward=controls.accelerating;
      const bool paused=controls.paused();
      if(paused!=wasPaused) {
        controls.release();
        SDL_SetWindowTitle(win,paused?"Destinia 0.10.3 Beta - PAUSADO":"Destinia 0.10.3 Beta");
        hostReport("[host] %s.\n",paused?"Pausado":"Retomado"); wasPaused=paused;
      }
      updateAudioState(controls.focused,paused);
      if(paused) { SDL_Delay(16); continue; }
    }
    if(executionFailed) break;
    // [SONDA] toque sintetico de teste (so com --autotap): pressiona no quadro N e solta 3 quadros depois
    if (autoFrame >= 0 && frame == autoFrame) touch(autoX, autoY, 0);
    if (autoFrame >= 0 && frame == autoFrame + 3) touch(autoX, autoY, 1);
    if (autoFrame2 >= 0 && frame == autoFrame2) touch(autoX2, autoY2, 0);
    if (autoFrame2 >= 0 && frame == autoFrame2 + 3) touch(autoX2, autoY2, 1);
    if(executionFailed) break;
    // [OK] jniRun(env,obj, byte[] framebuffer, int contador) -> ms ate o proximo quadro (0 = sair)
    u32 wait = 0;
    const auto runStart = std::chrono::steady_clock::now();
    traceViewportFrame = frame;
    dropNamesRenderer.enabled=controls.dropNamesHeld;
    bossMage.focused=controls.focused&&!controls.paused();
    if(!lootQol.collect(uint64_t(frame),controls.focused,[&](u32 fn){return callFn(fn,{6});})){executionFailed=true;break;}
    inventoryBeginFrame();
    fcBeginFrame();
    uiRenderer.beginFrame();
    const bool runSucceeded = callFn(fnRun, {ENV_OBJ, 0x10, hScreen, (u32)frame}, &wait);
    runMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - runStart).count();
    centerSmallMapCamera();
    traceViewportWriteFrameSummary(frame);
    if (!runSucceeded) {
      if (traceViewportRecording) stopTraceViewportRecording("run-error");
      break;
    }
    if (pendingBuyCash) {
      controls.release();
      if(!finishOfflinePurchase()){executionFailed=true;break;}
      if(!headlessFrames && SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"Loja offline","Compras indisponiveis neste modo. Seu saldo nao foi alterado.",win)!=0)
        hostReport("[host] Aviso: nao foi possivel mostrar a mensagem da loja.\n");
    }
    frame++;
    runtimeLogTick(frame,runMs);
    difficultyFlush();
    if(frame%16==0) codexFlush();
    destinia::lootEconomyProcessOffline(saveDir, fnGetCash, fnSetCash, callFn);
    destinia::lootEconomyPeriodicSave(saveDir, frame);
    probeTick(frame);
    cameraTick(frame);
    if (traceViewportRecording &&
        frame - traceViewportStartFrame >= traceViewportFrames)
      stopTraceViewportRecording("frame-limit");
    if (wait == 0) {
      hostReport("[host] o jogo pediu para encerrar.\n");
      if (traceViewportRecording) stopTraceViewportRecording("game-exit");
      break;
    }

    auto& frameBuffer = arrays[hScreen];
    const int hudScale = preferences.uiScale==100?getPlayerStatusHudScale(frameBuffer.data()):(uiRenderer.statusDrawn?1:0);
    if (hudScale != 0) {
      uint32_t currentExperience = 0, requiredExperience = 0;
      if (getCharacterExperience(currentExperience, requiredExperience))
        drawHudExperienceText(frameBuffer.data(), currentExperience, requiredExperience);
    }
    if(preferences.uiScale==100?hudScale!=0:uiRenderer.statusDrawn){std::array<destinia::SkillHintRect,4> hints{};for(size_t i=0;i<hints.size();++i){int16_t r[4]{};if(rd(exportsMap.at("MMain")+0x11a+u32(i)*8,r,8))hints[i]={r[0],r[1],r[2],r[3]};}destinia::paintSkillHints(frameBuffer.data(),SCREEN_W,SCREEN_H,preferences,controls.lastInputDevice(),hints,preferences.uiScale);}
    if(!codexOpen)fcPaint(frameBuffer.data(),preferences.uiScale);
    bossMage.paint(frameBuffer.data());
    bossRushUI.paint(frameBuffer.data(), SCREEN_W, SCREEN_H);
    lootQol.paint(frameBuffer.data());
    inventoryPaint(frameBuffer.data());
    nativeCodexPainter.paint(frameBuffer.data());
    difficultyPainter.paint(frameBuffer.data());
    weaponsCatalogHost.paint(frameBuffer.data(), SCREEN_W, SCREEN_H, rd16(exportsMap.at("Game")+0x4886), preferences.uiScale);
    sharedStashHost.paint(frameBuffer.data(), SCREEN_W, SCREEN_H, preferences.uiScale);
    bountiesHost.paint(frameBuffer.data(), SCREEN_W, SCREEN_H, rd16(exportsMap.at("Game")+0x4886), preferences.uiScale);
    const uint8_t* fb = frameBuffer.data();
    if (headlessFrames) {
      if (frame == 1 || frame % 10 == 0) { char nm[64]; snprintf(nm, sizeof nm, "/frame_%05ld.bmp", frame); writeBmp(outDir + nm, fb); }
      if (frame >= headlessFrames) break;
    } else {
      const auto renderStart = std::chrono::steady_clock::now();
      if(SDL_UpdateTexture(tex,nullptr,fb,SCREEN_W*2)!=0) { hostReport("SDL_UpdateTexture: %s\n",SDL_GetError()); executionFailed=true; break; }
      int ow = 0, oh = 0;
      SDL_GetRendererOutputSize(ren, &ow, &oh);
      SDL_Rect dst = gameViewport();
      SDL_SetRenderDrawColor(ren, 128, 128, 128, 255);
      SDL_RenderClear(ren);
      SDL_RenderCopy(ren, tex, nullptr, &dst);
      SDL_RenderPresent(ren);
      renderMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - renderStart).count();
    }
    // [?] Java: se jniRun devolve 1000 (houve carga de arquivo) faz GC e dorme 100 ms; senao dorme (ms - tempo gasto).
    long elapsed = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    // Ritmo do quadro: honra o 'wait' que o PROPRIO JOGO pediu. O Destinia e de passo fixo
    // (~16 FPS); respeitar o 'wait' mantem a velocidade original e evita CPU 100% / aquecimento.
    double budgetMs = wait >= 1000 ? 100.0 : (double)wait;
    long sleepMs = (long)std::max(0.0, budgetMs - (double)elapsed);
    const auto sleepStart = std::chrono::steady_clock::now();
    if (fastForward) {
      sleepMs = 0;
    } else if (sleepMs > 0 && !headlessFrames) {
      SDL_Delay((Uint32)sleepMs);
    }
    sleepMeasuredMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - sleepStart).count();
    if (profile) {
      const double frameMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
      profileWindowFrames++;
      profileRunMs += runMs;
      profileRenderMs += renderMs;
      profileSleepMs += sleepMeasuredMs;
      profileOtherMs += std::max(0.0, frameMs - runMs - renderMs - sleepMeasuredMs);
      profileMaxRunMs = std::max(profileMaxRunMs, runMs);
      const double profileSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - profileWindowStart).count();
      if (profileSeconds >= 5.0) {
        const double divisor = static_cast<double>(profileWindowFrames);
        hostReport(
                "[perfil] %.1fs: %.1f FPS | por quadro (ms): emulacao %.2f media/%.2f max; render %.2f; espera %.2f; restante %.2f\n",
                profileSeconds, profileWindowFrames / profileSeconds,
                profileRunMs / divisor, profileMaxRunMs, profileRenderMs / divisor,
                profileSleepMs / divisor, profileOtherMs / divisor);
        profileWindowStart = std::chrono::steady_clock::now();
        profileWindowFrames = 0;
        profileRunMs = profileRenderMs = profileSleepMs = profileOtherMs = profileMaxRunMs = 0.0;
      }
    }
  }
  if (traceViewportRecording) stopTraceViewportRecording("host-exit");
  if (headlessFrames) hostReport("[host] %ld quadros executados. Imagens em %s\n", frame, outDir.c_str());
  if(!headlessFrames && !executionFailed && savePreferences) {
    try { destinia::saveWindowPreferences(settingsPath,"host_zoom",windowWidth,windowHeight,fullscreen); }
    catch(const std::exception& e) { hostReport("[host] Aviso: %s\n",e.what()); }
  }
  if(destinia::lootEconomyReady){destinia::lootEconomy.lastTimestamp=int64_t(std::time(nullptr));try{destinia::saveLootEconomy(fs::path(saveDir),destinia::lootEconomy);}catch(...){}}
  return executionFailed ? 2 : 0;
}


int main(int argc,char** argv) {
  if(argc==2 && std::strcmp(argv[1],"--version")==0) { std::puts("Destinia 0.10.3 Beta"); return 0; }
  int result=1;
  if(argc==2 && std::string(argv[1])=="--import-classic") {
    try {const auto base=destinia::configuredSaveDir();destinia::importClassicSave(base,destinia::modeSaveDir(base,"expandido"));printf("Copia criada no modo expandido; a partida classica foi preservada.\n");return 0;}
    catch(const std::exception& error){fprintf(stderr,"%s\n",error.what());return 1;}
  }
  if(argc==3 && std::string(argv[1])=="--check-settings") {
    try { if(!fs::is_regular_file(argv[2])) throw std::runtime_error("Arquivo de preferencias nao encontrado."); destinia::Preferences::load(argv[2],"host_zoom"); printf("Preferencias validas.\n"); return 0; }
    catch(const std::exception& e) { fprintf(stderr,"%s\n",e.what()); return 1; }
  }
  try { result=runHost(argc,argv); }
  catch(const std::exception& error) { hostReport("[host] ERRO: %s\n",error.what()); }
  for(auto& file:files) if(saveTransaction.close(file.second)!=0) { hostReport("[host] ERRO ao fechar arquivo pendente\n"); result=1; }
  files.clear(); filePaths.clear();
  difficultyFlush();
  codexFlush();
  saveLock.release();
  if(uc) uc_close(uc);
  destinia::sessionLog.write("FIM DA SESSAO codigo="+std::to_string(result),true);
  destinia::sessionLog.close();
  return result;
}
