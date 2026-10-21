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

namespace fs = std::filesystem;
using u32 = uint32_t;

static std::string defaultSaveDir() {
  const char* localAppData = std::getenv("LOCALAPPDATA");
  if (!localAppData || !*localAppData) return "save";
  return (fs::path(localAppData) / "Destinia" / "save").string();
}

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
static bool verbose = false, fatal = false, hitMagic = false;
static std::string gameDir = "../game", soPath, assetsDir, saveDir = defaultSaveDir();
static u32 loadedSoSpan = 0;

static std::vector<std::function<void()>> handlers(STUB_COUNT);
static std::vector<std::string> stubName(STUB_COUNT);
static std::map<std::string, u32> importId;  // nome -> id do stub
static u32 nextMissing = ID_MISSING;
static std::vector<std::string> ring;        // ultimas chamadas (para debug)

static void note(const std::string& s) {
  ring.push_back(s);
  if (ring.size() > 40) ring.erase(ring.begin());
  if (verbose) fprintf(stderr, "[host] %s\n", s.c_str());
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
static void setSoundVol(int ch, int level) {
#ifdef HAS_SDL_MIXER
  int v = std::clamp(level, 0, 5) * MIX_MAX_VOLUME / 5;
  if (ch == 1) Mix_VolumeMusic(v); else Mix_Volume(-1, v);
#else
  (void)ch;
  (void)level;
#endif
}

// Teclas WIPI usadas pelo port Vita: 3/9/5/7 = cima/baixo/esq/dir, 17 = confirmar, 10 = voltar, 27 = menu, 1 = start.
static void sendKey(u32 pressFn, u32 releaseFn, int key, bool down) {
  u32 fn = down ? pressFn : releaseFn;
  if (!fn) return;
  callFn(fn, {(u32)key});
}
static void gameKey(int key, bool down) {
  int st = menuState();
  if (st <= 0) { if (down && fnLogoPress) callFn(fnLogoPress, {(u32)key}); return; }
  if (st == 1) sendKey(fnMenuPress, fnMenuRelease, key, down);
  else sendKey(fnGamePress, fnGameRelease, key, down);
}
static void probeState(const char* tag) {
  if (!probe) return;
  if (!gMMain || !gMenu) { fprintf(stderr, "[sonda] %s: enderecos ainda nao descobertos (MMain=%08x Menu=%08x Grp=%08x)\n", tag, gMMain, gMenu, gGrp); return; }
  fprintf(stderr, "[sonda] %s: rede=%u (0=livre 1=aguardando 2=erro 3=aviso) | menu.estado=%u menu.anim=%u | lista: y0=%u altura=%u itens=%u | tecla_toque=%u | tela=%ux%u\n",
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
  fprintf(stderr, "[camera] quadro %ld camX=%d camY=%d mapa=%dx%d tiles (%dx%d px) tela=%dx%d divisao_prevista_x=%d %s\n",
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
// Antes da leitura ARM: cobre tambem calculos de camera inline, sem depender de Map_setMapView.
static void cameraReadHook(uc_engine*, uc_mem_type, uint64_t, int, int64_t, void*) {
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
  if (heapPtr + n > HEAP_BASE + HEAP_SIZE) { fprintf(stderr, "[host] heap esgotado (pediu %u)\n", n); return 0; }
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
static std::map<u32, FILE*> files;
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

static const std::map<std::string, std::string>& portugueseText() {
  static const std::map<std::string, std::string> translations = {
      {"Fungus", "Fungo"}, {"Magic Fungus", "Fungo Magico"}, {"Dark Fungus", "Fungo Sombrio"},
      {"Scorpion", "Escorpiao"}, {"King Scorpion", "Rei Escorpiao"}, {"Demon Scorpion", "Escorpiao Demonio"},
      {"Curimon", "Curimon"}, {"Magic Curimon", "Curimon Magico"}, {"Dark Curimon", "Curimon Sombrio"},
      {"Piccolo", "Piccolo"}, {"Magic Piccolo", "Piccolo Magico"}, {"Dark Piccolo", "Piccolo Sombrio"},
      {"Mimic", "Mimico"}, {"Dark Mimic", "Mimico Sombrio"}, {"Chaos Mimic", "Mimico do Caos"},
      {"Fanatic", "Fanatico"}, {"Shadow Follower", "Seguidor Sombrio"},
      {"Dark Follower", "Seguidor Sombrio"}, {"Tentacle Mon", "Monstro Tentaculo"},
      {"King Tentacle", "Rei Tentaculo"}, {"Shadow Tentacle", "Tentaculo Sombrio"},
      {"Sasquatch", "Sasquatch"}, {"King Sasquatch", "Rei Sasquatch"},
      {"Demon Sasquatch", "Sasquatch Demonio"}, {"Black Mage", "Mago Negro"},
      {"Shadow Sorcerer", "Feiticeiro Sombrio"}, {"Dark Mage", "Mago Sombrio"},
      {"Skeleton", "Esqueleto"}, {"Magic Skeleton", "Esqueleto Magico"},
      {"Dark Skeleton", "Esqueleto Sombrio"}, {"Carnivore Wasp", "Vespa Carnivora"},
      {"Venomous Honey Bee", "Abelha Venenosa"}, {"King Wasp", "Vespa Rainha"},
      {"Ramu", "Ramu"}, {"Magic Ramu", "Ramu Magico"}, {"Dark Ramu", "Ramu Sombrio"},
      {"Magic Slime", "Gosma Magica"}, {"Demon Slime", "Gosma Demonio"},
      {"Dark Slime", "Gosma Sombria"}, {"Fire Spirit", "Espirito de Fogo"},
      {"Flame Spirit", "Espirito Flamejante"}, {"Lava Spirit", "Espirito de Lava"},
      {"Lycanthrope", "Licantropo"}, {"Dark Lycanthrope", "Licantropo Sombrio"},
      {"Demon Lycanthrope", "Licantropo Demonio"}, {"Bearman", "Homem-Urso"},
      {"Magic Bearman", "Homem-Urso Magico"}, {"Dark Bearman", "Homem-Urso Sombrio"},
      {"Magic Worm", "Verme Magico"}, {"Demon Worm", "Verme Demonio"},
      {"Great Worm", "Verme Gigante"}, {"Pumpkin Soldier", "Soldado Abobora"},
      {"Pumpkin Ghost", "Fantasma Abobora"}, {"Dark Pumpkin", "Abobora Sombria"},
      {"Frost Spirit", "Espirito de Geada"}, {"Ice Spirit", "Espirito de Gelo"},
      {"Ice Spirit ", "Espirito de Gelo "}, {"Glacier Spirit", "Espirito Glacial"},
      {"Zephyr", "Zefiro"}, {"Magic Zephyr", "Zefiro Magico"}, {"Dark Zephyr", "Zefiro Sombrio"},
      {"Imperial Soldier", "Soldado Imperial"}, {"Deserter Soldier", "Soldado Desertor"},
      {"Elite Soldier", "Soldado de Elite"}, {"Dungeon Guardian", "Guardiao da Caverna"},
      {"Defcan", "Defcan"}, {"Salas", "Salas"}, {"Sileth", "Sileth"},
      {"Berserk Tamur", "Tamur Furioso"}, {"Kaiser", "Kaiser"}, {"Baramos", "Baramos"},
      {"Diablo", "Diablo"}, {"Lava Guardian", "Guardiao da Lava"},
      {"Inventory", "Inventario"}, {"Equipment", "Equipamento"}, {"Equip", "Equipar"},
      {"Unequip", "Desequipar"}, {"Items", "Itens"}, {"Item", "Item"},
      {"Skills", "Habilidades"}, {"Skill", "Habilidade"}, {"Making", "Fabricacao"},
      {"Quest", "Missao"}, {"Quests", "Missoes"}, {"Main Menu", "Menu Principal"},
      {"Pause", "Pausa"}, {"Resume", "Continuar"}, {"Option", "Opcoes"},
      {"Options", "Opcoes"}, {"Save", "Salvar"}, {"Load", "Carregar"},
      {"Back", "Voltar"}, {"Exit", "Sair"}, {"Confirm", "Confirmar"},
      {"Cancel", "Cancelar"}, {"Select", "Selecionar"}, {"Use", "Usar"},
      {"Sell", "Vender"}, {"Buy", "Comprar"}, {"Create", "Criar"},
      {"Combine", "Combinar"}, {"Enhance", "Aprimorar"}, {"Appraise", "Identificar"},
      {"Pet", "Mascote"}, {"Status", "Status"}, {"Party", "Grupo"},
      {"BACK", "VOLTAR"}, {"Quest completed:<C0>%s<C>", "Missao concluida:<C0>%s<C>"},
      {"Want to accept this quest?", "Deseja aceitar esta missao?"},
      {"The quest accepted.", "Missao aceita."}, {"This quest completed.", "Missao concluida."},
      {"A new skill acquired.", "Nova habilidade adquirida."},
      {"You cannot complete this quest because your Inventory is full.", "Nao e possivel concluir esta missao: inventario cheio."},
      {"Your Inventory is full.", "Seu inventario esta cheio."},
      {"Has high Attack Strength and Vitality.\nMain Weapon: Giant Sword", "Possui alta Forca de Ataque e Vitalidade.\nArma Principal: Espada Grande"},
      {"Has high Dexterity and specializes in long-ranged attacks.\nMain Weapon: Gun", "Possui alta Destreza e e especialista em ataques a distancia.\nArma Principal: Arma de Fogo"},
      {"Has fast Attack and Movement Speeds.\nMain Weapon: One-handed Sword", "Possui alta velocidade de Ataque e Movimento.\nArma Principal: Espada de Uma Mao"},
      {"Equipment", "Equipamento"}, {"Select an item to compound.", "Selecione um item para combinar."},
      {"Select an item to fit an orb.", "Selecione um item para encaixar uma orbe."},
      {"Select an item to reinforce.", "Selecione um item para reforcar."},
      {"Select an item to identify. ", "Selecione um item para identificar. "},
      {"Select an item to reset.", "Selecione um item para redefinir."},
      {"Select an item to remove orbs.", "Selecione um item para remover orbes."},
      {"Active Skill: %s[LV%d]", "Habilidade Ativa: %s[Nv%d]"},
      {"Passive Skill: %s[LV%d]", "Habilidade Passiva: %s[Nv%d]"},
      {"%s Making", "Fabricacao de %s"}, {"Use Recipes to create equipment and items.", "Use receitas para criar equipamentos e itens."},
      {"Weapon", "Arma"}, {"Greatsword", "Espada Grande"}, {"Great Sword", "Espada Grande"},
      {"Sword", "Espada"}, {"Armor", "Armadura"},
      {"[Materials]", "[Materiais]"}, {"Material", "Material"},
      {"Select an advanced class.", "Selecione uma classe avancada."},
      {"Please select a price.", "Selecione um preco."},
      {"Select a price.", "Selecione um preco."},
      {"Compound items. (Auto-save)", "Combine itens. (Salvamento automatico)"},
      {"Disassemble an item. (Auto-save)", "Desmonte um item. (Salvamento automatico)"},
      {"Identify an item. (Auto-save)", "Identifique um item. (Salvamento automatico)"},
      {"Equip an orb. (Auto-save)", "Equipe uma orbe. (Salvamento automatico)"},
      {"Reset the status of an identified item.\n(Auto-save)", "Redefina os atributos de um item identificado.\n(Salvamento automatico)"},
      {"Reset the effects of an orb.\n(Auto-save)", "Redefina os efeitos de uma orbe.\n(Salvamento automatico)"},
      {"Attempt to reinforce an item.(Auto-save) Failing to reinforce the item can lower its grade or even destroy it.", "Tente reforcar um item. (Salvamento automatico) Uma falha pode reduzir sua qualidade ou ate destrui-lo."},
      {"Attempt to reinforce an item.(Auto-save) Failing to reinforce the item can lower its grade.", "Tente reforcar um item. (Salvamento automatico) Uma falha pode reduzir sua qualidade."},
      {"Using the item automatically saves the game.", "Usar este item salva o jogo automaticamente."},
      {"Open the box and obtain an item.\n(Auto-save)", "Abra a caixa e obtenha um item.\n(Salvamento automatico)"},
      {"Reset all of your stats.\n(Auto-save)", "Redefina todos os seus atributos.\n(Salvamento automatico)"},
      {"Reset all of your specialties.\n(Auto-save)", "Redefina todas as suas especialidades.\n(Salvamento automatico)"},
      {"Do you want to abandon this quest?", "Deseja abandonar esta missao?"},
      {"Select Character", "Selecionar Personagem"},
      {"BACK", "VOLTAR"}, {"Select a quantity(Auto save)", "Selecione a quantidade (salvamento automatico)"},
      {"Please select a quantity.", "Selecione uma quantidade."},
      {"Please select an item to gift.", "Selecione um item para presentear."},
      {"Please select an item to sell.", "Selecione um item para vender."},
      {"You cannot sell equipped items.", "Nao e possivel vender itens equipados."},
      {"The item's sale has been cancelled.", "A venda do item foi cancelada."},
      {"Please select a title.", "Selecione um titulo."},
      {"Main Menu", "Menu Principal"}, {"Special Shop", "Loja Especial"},
      {"Weapon Making", "Fabricacao de Armas"}, {"Armor Making", "Fabricacao de Armaduras"},
      {"Greatsword Making", "Fabricacao de Espadas Grandes"},
      {"Sword Making", "Fabricacao de Espadas"},
      {"Making of Weapon", "Fabricacao de Armas"},
      {"Making of Armor", "Fabricacao de Armaduras"},
      {"Making of Greatsword", "Fabricacao de Espadas Grandes"},
      {"Manufacturing", "Fabricacao"}, {"Quest List", "Lista de Missoes"},
      {"Quest completed", "Missao concluida"}, {"Quest accepted", "Missao aceita"},
      {"Active Skill", "Habilidade Ativa"}, {"Passive Skill", "Habilidade Passiva"},
      {"Skill Point", "Ponto de Habilidade"}, {"Skill Points", "Pontos de Habilidade"},
      {"Attribute", "Atributo"}, {"Attributes", "Atributos"},
      {"Specialty", "Especialidade"}, {"Specialties", "Especialidades"},
      {"Character", "Personagem"}, {"Character Status", "Status do Personagem"},
      {"Game Menu", "Menu do Jogo"}, {"Continue", "Continuar"},
      {"System", "Sistema"}, {"Help", "Ajuda"}, {"Quest Info", "Info da Missao"},
      {"Making Menu", "Menu de Fabricacao"}, {"Skill Menu", "Menu de Habilidades"},
      {"Item Menu", "Menu de Itens"}, {"Quest Menu", "Menu de Missoes"},
      {"Quest item.", "Item de missao."},
      {"Inferior Appraisal Scroll", "Perg. Ident. Inferior"},
      {"This can be used to identify Inferior equipment (Lv 1 ~ 19) ", "Identifica equipamento inferior (Nv. 1 a 19)"},
      {"Resurrection Scroll", "Pergaminho de Ressurreicao"},
      {"This can be used to instantly resurrect.", "Ressuscita instantaneamente."},
      {"Special Forging Stone", "Pedra de Forja Especial"},
      {"This can be used to reinforce equipment at any level. It safeguards equipment against breakage upon reinforcement failur}", "Reforca equipamentos de qualquer nivel e evita sua quebra em caso de falha."},
      {"Gold", "Ouro"}, {"Cash", "Creditos"}, {"Yes", "Sim"}, {"No", "Nao"},
      {"Return", "Voltar"}, {"Start", "Iniciar"}, {"Setting", "Configuracao"},
      {"Sound", "Som"}, {"Music", "Musica"}, {"On", "Ligado"}, {"Off", "Desligado"},
      {"Lv", "Nv"}, {"HP", "PV"}, {"MP", "PM"},
      {"Warrior", "Guerreiro"}, {"Shooter", "Atirador"}, {"Assassin", "Assassino"},
      {"Main Weapon", "Arma Principal"}, {"One-handed Sword", "Espada de Uma Mao"},
      {"Giant Sword", "Espada Grande"}, {"Long-ranged", "Longo Alcance"},
      {"Dash Attack", "Ataque Rapido"}, {"Regenerative Aura", "Aura Regenerativa"},
      {"Slash", "Corte"}, {"Crushing Strike", "Golpe Esmagador"},
      {"Combative Aura", "Aura de Combate"}, {"Fearful Shout", "Grito Amedrontador"},
      {"Fire Sword", "Espada de Fogo"}, {"Grinding Strike", "Golpe Triturador"},
      {"Immunity Aura", "Aura de Imunidade"}, {"Destructive Shout", "Grito Destrutivo"},
      {"Twister", "Tornado"}, {"Colossal Blow", "Golpe Colossal"},
      {"Meteor Strike", "Ataque Meteoro"}, {"Power Shot", "Tiro Poderoso"},
      {"Wisdom Aura", "Aura da Sabedoria"}, {"Aimed Shot", "Tiro Preciso"},
      {"Stun Gun", "Arma Atordoante"}, {"Wind Aura", "Aura do Vento"},
      {"Weakening Shout", "Grito Enfraquecedor"}, {"Wild Shot", "Tiro Selvagem"},
      {"Stun Wave", "Onda Atordoante"}, {"Ruthless Aura", "Aura Implacavel"},
      {"Foggy Shout", "Grito Nebuloso"}, {"Summon Machine Gun", "Invocar Metralhadora"},
      {"Finishing Shot", "Tiro Final"}, {"Hyper Bazooka", "Bazuca Hiper"},
      {"Asura Slash", "Corte Asura"}, {"Slaying Aura", "Aura Assassina"},
      {"Rupturing Blow", "Golpe Rupturante"}, {"Jump Attack", "Ataque Saltando"},
      {"Evasive Aura", "Aura Evasiva"}, {"Intimidating Shout", "Grito Intimidador"},
      {"Whirling Slash", "Corte Giratorio"}, {"Thunderstorm", "Tempestade"},
      {"Berserk Aura", "Aura Berserk"}, {"Killer Shout", "Grito Mortal"},
      {"Hiding", "Esconderijo"}, {"Frenzied Hacking", "Golpe Frenetico"},
      {"Star Fall", "Queda Estelar"}, {"Meteor", "Meteoro"},
      {"Cleverness", "Astucia"}, {"Bodybuilding", "Condicionamento"},
      {"Quickness", "Agilidade"}, {"Veteran Shooter", "Atirador Veterano"},
      {"Accurate Fire", "Tiro Preciso"}, {"Explosive Power", "Poder Explosivo"},
      {"Evasion Tactic", "Tatica Evasiva"}, {"Combat Strategy", "Estrategia de Combate"},
      {"Deadly Wound", "Ferida Mortal"}, {"Killer Instinct", "Instinto Assassino"},
      {"Destructive Power", "Poder Destrutivo"}, {"Ruthlessness", "Crueldade"},
      {"Regeneration", "Regeneracao"}, {"Intimidate", "Intimidar"},
      {"Crush", "Esmagar"}, {"Absorb", "Absorver"}, {"Training", "Treinamento"},
      {"Destruct", "Destruir"}, {"Solid", "Resistencia"}, {"Swift", "Rapidez"},
      {"Slaughter", "Massacre"}, {"Heal", "Curar"},
      {"Attack Strength", "Forca de Ataque"}, {"Critical Attack Chance", "Chance de Ataque Critico"},
      {"Critical damage", "Dano critico"}, {"Evasion Rate", "Taxa de Esquiva"},
      {"Accuracy Rate", "Precisao"}, {"Movement Speed", "Velocidade de Movimento"},
      {"Damage Receipt", "Dano Recebido"}, {"Damage", "Dano"},
      {"Defense", "Defesa"}, {"Attack", "Ataque"}, {"Bleeding damage", "Dano de sangramento"},
      {"Stun for", "Atordoa por"}, {"Stun", "Atordoamento"},
      {"Frost", "Geada"}, {"Fang", "Presa"}, {"Blood", "Sangue"},
      {"Rush", "Investida"}, {"Skull", "Craneo"}, {"Penetrator", "Perfurador"},
      {"Soul", "Alma"}, {"Destroyer", "Destruidor"}, {"Mad", "Insano"},
      {"Crusader", "Cruzado"}, {"Deathly", "Mortal"}, {"Guide", "Guia"},
      {"Raging", "Furioso"}, {"Storm", "Tempestade"}, {"Bringer", "Portador"},
      {"Hand", "Mao"}, {"Small", "Pequeno"}, {"Shield", "Escudo"},
      {"Heaven", "Ceu"}, {"Lethal", "Letal"}, {"Demolisher", "Demolidor"},
      {"Anti-frost", "Antigelo"}, {"Fiend", "Demonio"}, {"Hell", "Inferno"},
      {"Infinite", "Infinito"}, {"Vampire", "Vampiro"}, {"Vampiric Rate", "Taxa de Vampirismo"},
      {"Lv ", "Nv. "},
      {"sec.", "s"}, {"seconds", "segundos"}, {"second", "segundo"},
      {"Increase the duration of", "Aumenta a duracao de"},
      {"Reduce the downtime of", "Reduz o tempo de recarga de"},
      {"Reduce the MP consumption of", "Reduz o consumo de PM de"},
      {"Create a %d& chance of Stunning a target when", "Cria %d& de chance de Atordoar o alvo quando"},
      {"Increase the Critical Attack Chance of", "Aumenta a chance de Ataque Critico de"},
      {"Increase your", "Aumenta sua"}, {"Increase your HP Recovery Speed.", "Aumenta a velocidade de recuperacao de PV."},
      {"Increase your MP Recovery Speed.", "Aumenta a velocidade de recuperacao de PM."},
      {"Increase your Evasion Rate.", "Aumenta sua Taxa de Esquiva."},
      {"Increase your Attack Strength.", "Aumenta sua Forca de Ataque."},
      {"Increase your Critical Attack Chance by 5% for 60 seconds.", "Aumenta sua chance de Ataque Critico em 5% por 60 segundos."},
      {"by %d seconds.", "em %d segundos."}, {"by %d second.", "em %d segundo."},
      {"by %d&.", "em %d&."}, {"by %d.", "em %d."},
      {"for %d seconds.", "por %d segundos."},
      {"Damage x", "Dano x"}, {"% Damage", "% de Dano"},
      {"% Bleeding damage for", "% de Dano de sangramento por"},
      {"HP Recovery", "Recuperacao de PV"}, {"MP Recovery", "Recuperacao de PM"},
      {"Instantly restores ", "Restaura instantaneamente "},
      {"Restores ", "Restaura "}, {"Adds ", "Adiciona "},
      {"Provides ", "Concede "}, {"This can be used to create ", "Use para criar "},
      {"This can be used to reinforce ", "Use para reforcar "},
      {"This can be used to identify ", "Use para identificar "},
      {"when equipped to an", "ao equipar em um"},
      {"equipment item", "equipamento"},
      {"any level", "qualquer nivel"},
      {"Inventory slots", "espacos no inventario"},
      {"HP over", "PV ao longo de"},
      {"MP over", "PM ao longo de"},
      {" HP.", " PV."}, {" MP.", " PM."},
      {" HP ", " PV "}, {" MP ", " PM "},
      {"HP and MP", "PV e PM"},
      {"HP Recovery", "recuperacao de PV"},
      {"MP Recovery", "recuperacao de PM"},
      {"Critical Attack Chance", "chance de Ataque Critico"},
      {"Accuracy Rate", "Precisao"},
      {"Evasion Rate", "Taxa de Esquiva"},
      {"Vampiric Rate", "Taxa de Vampirismo"},
      {"Strength", "Forca"}, {"Dexterity", "Destreza"},
      {"Vitality", "Vitalidade"}, {"Intelligence", "Inteligencia"},
      {"Damage", "Dano"}, {"Defense", "Defesa"},
      {"Restores your", "Recupera seu"},
      {"Resurrect", "Ressuscitar"},
      {"Breakage", "Quebra"},
      {"reinforcement failure", "falha no reforco"},
      {"upon reinforcement failure", "em caso de falha no reforco"},
      {"create equipment", "fabricar equipamentos"},
      {"create potions", "fabricar pocoes"},
      {"create corresponding item", "criar o item correspondente"},
      {"reset its status to unidentified", "remover sua identificacao"},
      {"removes all orbs", "remove todas as orbes"},
      {"safeguards equipment against breakage", "evita que o equipamento quebre"},
      {"produces a", "gera uma"},
      {"produces an", "gera um"},
      {"latest village", "ultima vila"},
      {"This can be used on identified equipment at all levels to reset its status to unidentified.", "Use para remover a identificacao de equipamentos de qualquer nivel."},
      {"Use this recipe to create the corresponding item.", "Use esta receita para criar o item correspondente."},
      {"Produces ", "Gera "}, {"Returns to your latest village.", "Retorna a sua ultima vila."},
      {"Removes all orbs from an equipment item.", "Remove todas as orbes de um equipamento."},
      {"Resets your stats.", "Redefine seus atributos."},
      {"Resets your specialties.", "Redefine suas especialidades."},
      {"Instantly resurrect.", "Ressuscita instantaneamente."},
      {" over ", " ao longo de "}, {" at any level.", " de qualquer nivel."},
      {" at all levels ", " de qualquer nivel "}, {" (Lv ", " (Nv. "},
      {" ~ ", " a "}, {"seconds.", "segundos."}, {" seconds.", " segundos."},
      {"% Damage Receipt", "% de Dano Recebido"}, {"Critical damage", "dano Critico"},
      {"Critical Attack", "Ataque Critico"}, {"inflicts", "causa"},
      {"weakest point", "ponto mais fraco"}, {"area-effective damage", "dano em area"},
      {"friendly forces", "aliados"}, {"friendly forcey forces", "aliados"},
      {"all skills", "todas as habilidades"}, {"item drop rate", "chance de obter itens"},
      {"gold drop rate", "chance de obter ouro"}, {"Attack with", "Ataque com"},
      {"decreasing an enemy's", "reduzir a"}, {"Stunning", "Atordoar"},
      {"Stuns", "Atordoa"}, {"chance of", "chance de"}, {"when ", "quando "},
      {"while ", "enquanto "}, {"target", "alvo"}, {"bullets", "balas"},
      {"bullet", "bala"}, {"magic", "magia"}, {"powerful", "poderoso"},
      {"fire ", "disparar "}, {"Fire ", "Fogo "}, {"enemy's", "do inimigo"},
      {"your ", "seu "}, {"his ", "seu "}, {" to ", " para "},
      {" with ", " com "}, {" and ", " e "}, {"Attack Strength x", "Forca de Ataque x"},
      {"Dash forward to perform a triple attack.", "Avance rapidamente para executar um ataque triplo."},
      {"Shoot spirit swords to attack enemies in front of you.", "Dispare espadas espirituais contra os inimigos a sua frente."},
      {"Forcefully strike an enemy to inflict damage and cause Bleeding.", "Acerte um inimigo com forca para causar dano e sangramento."},
      {"Shout to decrease the Movement Speed of surrounding enemies.", "Grite para reduzir a velocidade dos inimigos ao redor."},
      {"Infuse your sword with the power of fire to strike surrounding enemies.", "Imbua sua espada com fogo e ataque os inimigos ao redor."},
      {"Cause Bleeding to surrounding enemies.", "Causa sangramento nos inimigos ao redor."},
      {"Relieve your pain and reduce the amount of damage received from enemies.", "Reduz o dano recebido dos inimigos."},
      {"Shout to decrease the Defense of surrounding enemies.", "Grite para reduzir a Defesa dos inimigos ao redor."},
      {"Inflict multiple damage with a spinning attack.", "Cause varios danos com um ataque giratorio."},
      {"Jump high to deliver a deadly blow.", "Salte alto e desfira um golpe mortal."},
      {"Summon a meteor that inflicts area-effective damage.", "Invoque um meteoro que causa dano em area."},
      {"Infuse your gun with magic to fire powerful bullets.", "Imbua sua arma com magia para disparar balas poderosas."},
      {"Increase your MP Recovery Speed.", "Aumenta a velocidade de recuperacao de PM."},
      {"Attack an enemy's at his weakest point.", "Ataque o inimigo em seu ponto fraco."},
      {"Attack and Stun enemies in front of you.", "Ataque e atordoe os inimigos a sua frente."},
      {"Increase your Evasion Rate.", "Aumenta sua Taxa de Esquiva."},
      {"Weaken surrounding enemies to decrease their Defense.", "Enfraquece os inimigos ao redor e reduz sua Defesa."},
      {"Perform rapid firing attacks at all surrounding enemies.", "Dispare rapidamente contra todos os inimigos ao redor."},
      {"Fire a magic bullet that Stuns surrounding enemies.", "Dispare uma bala magica que atordoa os inimigos ao redor."},
      {"Decrease the enemy's Accuracy Rate by fogging up their vision.", "Reduz a Precisao do inimigo ao obscurecer sua visao."},
      {"Summon a powerful machine gun. ", "Invoque uma metralhadora poderosa. "},
      {"Finish a target with a powerful bullet.", "Derrote o alvo com uma bala poderosa."},
      {"Perform powerful multi-attacks to enemies in front of you.", "Execute varios ataques poderosos contra os inimigos a sua frente."},
      {"Inflict lethal damage on an enemy by stabbing his body.", "Cause dano letal ao perfurar o inimigo."},
      {"Jump-attack and Stun a target.", "Salte, ataque e atordoe o alvo."},
      {"Shout to intimidate an enemy, causing him to lose his will to fight.", "Intimide o inimigo com um grito e reduza sua vontade de lutar."},
      {"Infuse your sword with Fire magic to inflict area-effective damage.", "Imbua sua espada com magia de Fogo para causar dano em area."},
      {"Attack and Stun surrounding enemies.", "Ataque e atordoe os inimigos ao redor."},
      {"Unleash your rage to increase Attack Strength.", "Liberte sua furia para aumentar a Forca de Ataque."},
      {"While hiding from enemy detection, perform Stunning attacks with tremendous damage.", "Fique oculto e execute ataques atordoantes que causam muito dano."},
      {"Hack an enemy to inflict major damage.", "Golpeie o inimigo e cause muito dano."},
      {"Attack with a %d& chance of Stunning an enemy for %d.%d second. This skill lasts for 10 seconds.", "Ataca com %d& de chance de atordoar o inimigo por %d.%d segundo. Dura 10 segundos."},
      {"Attack with a %d& chance of decreasing an enemy's Attack Strength by %d& for 10 seconds. This skill lasts for 10 seconds.", "Ataca com %d& de chance de reduzir a Forca de Ataque do inimigo em %d& por 10 segundos. Dura 10 segundos."},
      {"Attack with a %d& chance of decreasing an enemy's Defense by %d& for 10 seconds. This skill lasts for 10 seconds.", "Ataca com %d& de chance de reduzir a Defesa do inimigo em %d& por 10 segundos. Dura 10 segundos."},
      {"Attack with a %d& chance of regaining %d& of inflicted damage as HP. This skill lasts for 10 seconds.", "Ataca com %d& de chance de recuperar PV equivalentes a %d& do dano causado. Dura 10 segundos."},
      {"Reduce the MP consumption of all skills by %d& for 10 seconds.", "Reduz o consumo de PM de todas as habilidades em %d& por 10 segundos."},
      {"Increase the Attack Strength of friendly forces by %d& for 10 seconds.", "Aumenta a Forca de Ataque dos aliados em %d& por 10 segundos."},
      {"Increase the Defense of friendly forces by %d& for 10 seconds.", "Aumenta a Defesa dos aliados em %d& por 10 segundos."},
      {"Increase the Evasion Rate of friendly forces by %d& for 10 seconds.", "Aumenta a Taxa de Esquiva dos aliados em %d& por 10 segundos."},
      {"Increase the Critical Attack Chance of friendly forces by %d& for 10 seconds.", "Aumenta a chance de Ataque Critico dos aliados em %d& por 10 segundos."},
      {"Restores my master's HP by %d&.", "Recupera %d& dos PV do meu mestre."},
      {"Increase the Attack Strength of friendly forces by %d&.", "Aumenta a Forca de Ataque dos aliados em %d&."},
      {"Increase your gold drop rate by %d&.", "Aumenta a chance de obter ouro em %d&."},
      {"Increase your item drop rate by %d&.", "Aumenta a chance de obter itens em %d&."},
      {"Increase the Accuracy Rate of friendly forces by %d&.", "Aumenta a Precisao dos aliados em %d&."},
      {"Increase the Evasion Rate of friendly forces by %d&.", "Aumenta a Taxa de Esquiva dos aliados em %d&."},
      {"Increase the Critical Attack Chance of friendly forces by %d&.", "Aumenta a chance de Ataque Critico dos aliados em %d&."},
      {"Increase the HP of friendly forces by %d&.", "Aumenta os PV dos aliados em %d&."},
      {"Increase the MP of friendly forces by %d&.", "Aumenta os PM dos aliados em %d&."},
      {"Noob", "Novato"}, {"Crude", "Rustico"}, {"Simple", "Simples"},
      {"Inferior", "Inferior"}, {"Mediocre", "Medio"}, {"Average", "Medio"},
      {"Superior", "Superior"}, {"Supreme", "Supremo"}, {"Celestial", "Celestial"},
      {"Ultimate", "Supremo"}, {"Normal", "Comum"}, {"Magic", "Magico"},
      {"Rare", "Raro"}, {"Epic", "Epico"}, {"Old", "Antigo"},
      {"Worn", "Gasto"}, {"Iron", "Ferro"}, {"Scale", "Escamas"},
      {"Suede", "Camurca"}, {"Chain", "Malha"}, {"Refined", "Refinado"},
      {"Bone", "Osso"}, {"Silver", "Prata"}, {"Silvery", "Prateado"},
      {"Hunting", "Caca"}, {"Hunter", "Cacador"}, {"Gold", "Ouro"},
      {"Golden", "Dourado"}, {"Fighter", "Guerreiro"}, {"Fight", "Combate"},
      {"Great", "Grande"}, {"Rune", "Runa"}, {"Battle", "Batalha"},
      {"War", "Guerra"}, {"Savage", "Selvagem"}, {"Divine", "Divino"},
      {"Holy", "Sagrado"}, {"Ancient", "Antigo"}, {"Active Volcano", "Vulcao Ativo"},
      {"Dragon Scale", "Escama de Dragao"}, {"Bloodlust", "Sede de Sangue"},
      {"Goddess", "Deusa"}, {"Death", "Morte"}, {"Hell Destroyer", "Destruidor do Inferno"},
      {"Vanquisher", "Vencedor"}, {"Eternal", "Eterno"}, {"Flawless", "Impecavel"},
      {"Sword", "Espada"}, {"Blade", "Lamina"}, {"Gun", "Arma"},
      {"Cannon", "Canhao"}, {"Shooter", "Atirador"}, {"Rifle", "Rifle"},
      {"Armor", "Armadura"}, {"Gloves", "Luvas"}, {"Shoes", "Botas"},
      {"Helm", "Elmo"}, {"Helmet", "Capacete"}, {"Hood", "Capuz"},
      {"Noob Blade", "Lamina de Novato"}, {"Old Blade", "Lamina Antiga"},
      {"Longsword", "Espada Longa"}, {"Saber", "Sabre"},
      {"Zweihander", "Espada de Duas Maos"}, {"Breaker", "Quebrador"},
      {"Moon Blade", "Lamina da Lua"}, {"Venom Slayer", "Matador de Veneno"},
      {"Dark Edge", "Lamina Sombria"}, {"Dark Slayer", "Matador Sombrio"},
      {"Air Blade", "Lamina do Ar"}, {"Ancient Saber", "Sabre Antigo"},
      {"Terror Blade", "Lamina do Terror"}, {"Giant Butcher", "Acougueiro Gigante"},
      {"Punishing Massacrer", "Massacrador Punitivo"}, {"Death Fang", "Presa da Morte"},
      {"Blood Cutter", "Cortador de Sangue"}, {"Destructive Guide", "Guia Destrutivo"},
      {"Meat Tenderizer", "Amaciador de Carne"}, {"Dragon Slayer", "Matador de Dragoes"},
      {"Iron Edge", "Lamina de Ferro"}, {"Broadsword", "Espada Larga"},
      {"Rapier", "Rapieira"}, {"Gladius", "Gladio"}, {"Scimitar", "Cimitarra"},
      {"Steel Blade", "Lamina de Aco"}, {"Glacier sword", "Espada Glacial"},
      {"Volcano sword", "Espada Vulcanica"}, {"Phantom Edge", "Lamina Fantasma"},
      {"Storm sword", "Espada da Tempestade"}, {"Tornado sword", "Espada Tornado"},
      {"Frost Fang", "Presa de Gelo"}, {"Blood Rush", "Investida de Sangue"},
      {"Skull Penetrator", "Perfurador de Cranio"}, {"Soul Destroyer", "Destruidor de Almas"},
      {"Mad Crusader", "Cruzado Louco"}, {"Deathly Guide", "Guia Mortal"},
      {"Raging Blood", "Sangue Furioso"}, {"Raging Storm", "Tempestade Furiosa"},
      {"Bowgun", "Besta"}, {"Musket", "Mosquete"}, {"Sun Rider", "Cavaleiro do Sol"},
      {"Dead Eye", "Olho Morto"}, {"Blaster", "Disparador"},
      {"Drill Shooter", "Atirador Perfurador"}, {"Mega Blaster", "Mega Disparador"},
      {"Giga Shooter", "Atirador Giga"}, {"Accurate Shooter", "Atirador Preciso"},
      {"Lethal Blow", "Golpe Letal"}, {"Demolisher Musket", "Mosquete Demolidor"},
      {"Fiend Crusher", "Esmagador de Demonios"}, {"Hell Grinder", "Moedor Infernal"},
      {"Storm Destroyer", "Destruidor da Tempestade"},
      {"Infinite Crusher", "Esmagador Infinito"}, {"Noob Hat", "Chapeu de Novato"},
      {"Royal Gloves", "Luvas Reais"}, {"Ancient Hands", "Manoplas Antigas"},
      {"Flawless Armor ", "Armadura Impecavel"}, {"Ring of Truth", "Anel da Verdade"},
      {"Ring of Pledge", "Anel do Pacto"}, {"Death Loop", "Elo da Morte"},
      {"Hellish Loop", "Elo Infernal"},
      {"Hat", "Chapeu"}, {"Ring", "Anel"}, {"Hands", "Manoplas"},
      {"Potion", "Pocao"}, {"Vial", "Frasco"}, {"Orb", "Orbe"},
      {"Backpack", "Mochila"}, {"Forging Stone", "Pedra de Forja"},
      {"Appraisal", "Identificacao"}, {"Reset", "Redefinicao"},
      {"Scroll", "Pergaminho"}, {"Special", "Especial"},
      {"Traveler's", "Viajante"}, {"Expert's", "Especialista"},
      {"Appraisal Scroll", "Pergaminho de Identificacao"}, {"Monster Egg", "Ovo de Monstro"},
      {"Return Scroll", "Pergaminho de Retorno"}, {"Orb Removal Scroll", "Pergaminho de Remocao de Orbes"},
      {"Stat Reset Scroll", "Pergaminho de Redefinicao de Atributos"},
      {"Specialty Reset Scroll", "Pergaminho de Redefinicao de Especialidades"},
      {"Jewelry Box", "Caixa de Joias"}, {"Weapon Chest", "Bau de Arma"},
      {"Armor Chest", "Bau de Armadura"}, {"Resurrection Scroll", "Pergaminho de Ressurreicao"},
      {"Recipe", "Receita"}, {"Gem", "Gema"}, {"Mithril", "Mithril"},
      {"Steel", "Aco"}, {"Copper", "Cobre"}, {"Worn Leather", "Couro Gasto"},
      {"Tough Leather", "Couro Resistente"}, {"Durable Leather", "Couro Duravel"},
      {"Magic Leather", "Couro Magico"}, {"Dragon Leather", "Couro de Dragao"},
      {"Linen", "Linho"}, {"Wool", "La"}, {"Silk", "Seda"},
      {"Magic Fabric", "Tecido Magico"}, {"Rune Fabric", "Tecido de Runas"},
      {"Life Plant", "Planta Vital"}, {"Mystic Leaf", "Folha Mistica"},
      {"Magic Plant", "Planta Magica"}, {"Ginseng", "Ginseng"},
      {"Illusion Leaf", "Folha da Ilusao"}, {"Magic Powder", "Po Magico"},
      {"Fairy Powder", "Po de Fada"}, {"Abyss Powder", "Po do Abismo"},
      {"Spirit Powder", "Po Espiritual"}, {"Sky Powder", "Po Celestial"},
      {"Magic Fountain", "Fonte Magica"}, {"Fairy Fountain", "Fonte de Fada"},
      {"Abyss Fountain", "Fonte do Abismo"}, {"Spirit Fountain", "Fonte Espiritual"},
      {"Sky Fountain", "Fonte Celestial"}, {"Magic Essence", "Essencia Magica"},
      {"Fairy Essence", "Essencia de Fada"}, {"Abyss Essence", "Essencia do Abismo"},
      {"Spirit Essence", "Essencia Espiritual"}, {"Sky Essence", "Essencia Celestial"},
      {"Magic Core", "Nucleo Magico"}, {"Fairy Core", "Nucleo de Fada"},
      {"Abyss Core", "Nucleo do Abismo"}, {"Spirit Core", "Nucleo Espiritual"},
      {"Sky Core", "Nucleo Celestial"},
      {"slots.", "espacos."}, {"Instantly", "Instantaneamente"},
      {"equipped", "equipado"}, {"equipping", "equipar"},
      {"create", "criar"}, {"reinforce", "reforcar"},
      {"identify", "identificar"}, {"unidentified", "nao identificado"},
      {"item.", "item."}, {"equipment.", "equipamento."},
      {"Mystic Crystal", "Cristal Mistico"}, {"Shield Breaker Stone", "Pedra Quebra-Escudo"},
      {"Holy Spirit Stone", "Pedra do Espirito Sagrado"}, {"Mushroom Spores", "Esporos de Cogumelo"},
      {"Mushroom Spore", "Esporo de Cogumelo"}, {"Piccolo Skins", "Peles de Piccolo"},
      {"Scorpion Tails", "Caudas de Escorpiao"}, {"Silver Coins", "Moedas de Prata"},
      {"Bone Fragments", "Fragmentos de Osso"}, {"Magic Slimes", "Slimes Magicos"},
      {"Frost Spirits", "Espiritos de Gelo"}, {"Dark Followers", "Seguidores das Trevas"},
      {"Shadow Followers", "Seguidores das Sombras"}, {"Dark Curimons", "Curimons Sombrios"},
      {"Demon Slimes", "Slimes Demonios"}, {"Magic Curimons", "Curimons Magicos"},
      {"Great Worms", "Vermes Gigantes"}, {"King Wasps", "Vespas-Rei"},
      {"Dark Slimes", "Slimes Sombrios"}, {"Deserter Soldiers", "Soldados Desertores"},
      {"Dark Skeletons", "Esqueletos Sombrios"}, {"Magic Skeletons", "Esqueletos Magicos"},
      {"Lycanthropes", "Licantropos"}, {"Demon Lycanthropes", "Licantropos Demonios"},
      {"Mushroom Specimens", "Amostras de Cogumelo"}, {"Insect Specimens", "Amostras de Inseto"},
      {"Venom Stings", "Ferroes Venenosos"}, {"Venom Sting", "Ferrao Venenoso"},
      {"Flame Fragments", "Fragmentos de Chama"}, {"Dark Talismans", "Talismas Sombrios"},
      {"Lost Silver Coins", "Moedas de Prata Perdidas"}, {"Lost Treasure", "Tesouro Perdido"},
      {"Go find ", "Va buscar "}, {"Go back to ", "Volte para "},
      {"Go to the ", "Va para o "}, {"Meet ", "Encontre "},
      {"To the Ancient Ruins", "Rumo as Ruinas Antigas"},
      {"Back to the Camp", "De volta ao Acampamento"},
      {"Deterring the Resurrection", "Impedir a Ressurreicao"},
      {"Hunting Magic Slimes", "Cacando Slimes Magicos"},
      {"Collecting Mushroom Spore", "Coletando Esporos de Cogumelo"},
      {"Taming Pet", "Domando um Mascote"},
      {"Learning Composition", "Aprendendo a Combinar"},
      {"Letter Delivery", "Entrega de Carta"},
      {"Lost Treasure", "Tesouro Perdido"},
      {"Hunting Dark Followers", "Cacando Seguidores das Trevas"},
      {"Exterminating Great Worms", "Eliminando Vermes Gigantes"},
      {"Killing Deserter Soldiers", "Derrotando Soldados Desertores"},
      {"Eliminating Lava Guardian", "Derrotando o Guardiao de Lava"},
      {"Collecting Nippers", "Coletando Alicates"},
      {"Collecting Venom Stings", "Coletando Ferroes Venenosos"},
      {"Collecting Specimens", "Coletando Amostras"},
      {"Subjugating Frost Spirits", "Derrotando Espiritos de Gelo"},
      {"Subjugating Lycanthropes", "Derrotando Licantropos"},
      {"Exterminating King Wasps", "Eliminando Vespas-Rei"},
      {"Exterminating Dark Slimes", "Eliminando Slimes Sombrios"},
      {"Taming", "Domando"}, {"Learning", "Aprendendo"}, {"Composition", "Combinacao"},
      {"Honey", "Mel"}, {"Honeys", "Meles"}, {"Skin", "Pele"}, {"Skins", "Peles"},
      {"Mushroom", "Cogumelo"}, {"Specimens", "Amostras"}, {"Fragment", "Fragmento"},
      {"Fragments", "Fragmentos"}, {"Venom", "Veneno"}, {"Tail", "Cauda"},
      {"Tails", "Caudas"}, {"Coin", "Moeda"}, {"Coins", "Moedas"},
      {"Strength", "Forca"}, {"Dexterity", "Destreza"}, {"Vitality", "Vitalidade"},
      {"Intelligence", "Inteligencia"}, {"Luck", "Sorte"}, {"Appraisal Scroll", "Pergaminho de Identificacao"},
      {"This can be used to identify", "Use para identificar"},
      {"equipment", "equipamento"}, {"equipment item", "equipamento"},
      {"Collect ", "Colete "}, {"Return to ", "Retorne para "}, {"Go to ", "Va para "},
      {"Find ", "Encontre "}, {"Hunt ", "Cace "}, {"Hunt", "Caca"},
      {"Hunting ", "Cacando "}, {"Collecting ", "Coletando "},
      {"Exterminate ", "Elimine "}, {"Subjugate ", "Derrote "},
      {"Deliver ", "Entregue "}, {"Obtain ", "Obtenha "}, {"in the ", " em "},
      {" in ", " em "}, {" near the ", " perto de "}, {" south of ", " ao sul de "},
      {" north of ", " ao norte de "}, {" of the ", " do "},
      {"Friendly forces", "Aliados"}, {"Increase ", "Aumenta "},
      {"Reduce ", "Reduz "}, {"chance of", "chance de"},
      {"Attack and", "Ataque e"}, {"enemies", "inimigos"}, {"enemy", "inimigo"},
      {"surrounding", "ao redor"}, {" for ", " por "}, {" by ", " em "},
      {"%s %d acquired", "%s %d adquirido"},
      {"%s %d& increase", "%s %d& de aumento"},
      {"%s acquired", "%s adquirido"},
      {"%s obtained", "%s obtido"},
      {"%s[LV%d]", "%s[NV%d]"},
      {"(%dW %dL)", "(%dV %dD)"},
      {"(Completed)", "(Concluido)"},
      {"<C%d>%s<C> acquired", "<C%d>%s<C> adquirido"},
      {"<Trial>", "<Teste>"},
      {"??? (Locked)", "??? (Bloqueado)"},
      {"Accuracy", "Precisao"},
      {"Accuracy Orb", "Orbe de Precisao"},
      {"Add", "Adicionar"},
      {"An error has occurred.", "Ocorreu um erro."},
      {"Archer", "Arqueiro"},
      {"Assign Hotkey", "Atribuir Atalho"},
      {"Auction Hall", "Salao de Leiloes"},
      {"Auto Target", "Alvo Automatico"},
      {"Bag", "Mochila"},
      {"Bat", "Morcego"},
      {"Blademaster", "Mestre da Lamina"},
      {"Bruiser", "Brigao"},
      {"Buy :", "Comprar :"},
      {"Buy:", "Comprar:"},
      {"Celestial Accuracy Orb", "Orbe Celestial de Precisao"},
      {"Celestial Critical Orb", "Orbe Celestial de Critico"},
      {"Celestial Evasion Orb", "Orbe Celestial de Evasao"},
      {"Change Class", "Mudar Classe"},
      {"Cheetah", "Guepardo"},
      {"Cobra", "Cobra"},
      {"Completed", "Concluido"},
      {"Compound", "Combinar"},
      {"Cooltime : %dsec", "Recarga : %ds"},
      {"Core", "Nucleo"},
      {"Critical", "Critico"},
      {"Critical Orb", "Orbe de Critico"},
      {"Daily", "Diario"},
      {"Daily Ranking", "Ranking Diario"},
      {"Dark Talisman", "Talisma Sombrio"},
      {"Defeated", "Derrotado"},
      {"Defend", "Defender"},
      {"Details:", "Detalhes:"},
      {"Disassemble", "Desmontar"},
      {"Discard", "Descartar"},
      {"Dolphin", "Golfinho"},
      {"EXP %d acquired", "EXP %d obtido"},
      {"Eagle", "Aguia"},
      {"Elven", "Elfico"},
      {"Enter an amount of gold.", "Insira uma quantidade de ouro."},
      {"Essence", "Essencia"},
      {"Evasion", "Evasao"},
      {"Evasion Orb", "Orbe de Evasao"},
      {"Evolve", "Evoluir"},
      {"Evolve:  <C%d>%s<C>", "Evoluir:  <C%d>%s<C>"},
      {"Fabric", "Tecido"},
      {"Failed", "Falhou"},
      {"Falcon", "Falcao"},
      {"Fast", "Rapido"},
      {"Fountain", "Fonte"},
      {"General", "Geral"},
      {"Gift Item", "Item de Presente"},
      {"Give Up", "Desistir"},
      {"Graphic", "Grafico"},
      {"Grizzly", "Urso Grizzly"},
      {"Gunner", "Atirador"},
      {"Hall of Honor", "Salao da Honra"},
      {"Health", "Saude"},
      {"Herb", "Erva"},
      {"High", "Alto"},
      {"Hippo", "Hipopotamo"},
      {"Honor", "Honra"},
      {"Honor + %d", "Honra + %d"},
      {"Honor Point", "Ponto de Honra"},
      {"Honor Point + %d", "Ponto de Honra + %d"},
      {"Honor Shop", "Loja de Honra"},
      {"Honor:", "Honra:"},
      {"Hotkey", "Atalho"},
      {"Hound", "Cao de Caca"},
      {"Inferior Accuracy Orb", "Orbe de Precisao Inferior"},
      {"Inferior Critical Orb", "Orbe de Critico Inferior"},
      {"Inferior Evasion Orb", "Orbe de Evasao Inferior"},
      {"Infiltrate Lagrant.", "Infiltrar-se em Lagrant."},
      {"Infiltrating the Castle", "Infiltrando-se no Castelo"},
      {"Info", "Informacoes"},
      {"Insect Specimen", "Especime de Inseto"},
      {"Jaguar", "Jaguatirica"},
      {"Killer", "Matador"},
      {"Knight", "Cavaleiro"},
      {"Leather", "Couro"},
      {"Legend", "Lenda"},
      {"Lion", "Leao"},
      {"Lose:", "Derrotas:"},
      {"Low", "Baixo"},
      {"Lynx", "Lince"},
      {"Machine Gun", "Metralhadora"},
      {"Mailbox", "Caixa de Entrada"},
      {"Manage Data", "Gerenciar Dados"},
      {"Master", "Mestre"},
      {"Missing Raina", "Raina Desaparecida"},
      {"Monkey", "Macaco"},
      {"Move", "Mover"},
      {"Multi Mode", "Modo Multi"},
      {"My Info", "Minhas Informacoes"},
      {"My Ranking:", "Meu Ranking:"},
      {"Mystic Crystals", "Cristais Misticos"},
      {"Nipper", "Pinca"},
      {"Nippers", "Pincas"},
      {"Notice", "Aviso"},
      {"Objective:", "Objetivo:"},
      {"Old Gun", "Arma Velha"},
      {"Ore", "Minerio"},
      {"Others", "Outros"},
      {"Panther", "Pantera"},
      {"Peter's Lunchbox", "Lancheira do Peter"},
      {"Please enter a player ID.", "Por favor, insira um ID de jogador."},
      {"Point", "Ponto"},
      {"Powder", "Po"},
      {"Power", "Poder"},
      {"Purchased.", "Comprado."},
      {"Rat", "Rato"},
      {"Received Gift Box", "Caixa de Presente Recebida"},
      {"Recipient : %s", "Destinatario : %s"},
      {"Refuge Camp", "Acampamento de Refugio"},
      {"Refuge Outpost", "Posto de Refugio"},
      {"Register this item?", "Registrar este item?"},
      {"Req. LV:%d", "NV Requisito:%d"},
      {"Require: %s %d Points", "Requer: %s %d Pontos"},
      {"Require: %s 5 Points", "Requer: %s 5 Pontos"},
      {"Required LV:%d", "NV Requerido:%d"},
      {"Resistance Camp", "Acampamento da Resistencia"},
      {"Rewards:", "Recompensas:"},
      {"Rogue", "Picaro"},
      {"Sage's", "do Sabio"},
      {"Saving Raina", "Salvando Raina"},
      {"Saving Urpa", "Salvando Urpa"},
      {"Selling Price : %d", "Preco de Venda : %d"},
      {"Shark", "Tubarao"},
      {"Slime Serum", "Soro de Gosma"},
      {"Slime Serums", "Soros de Gosma"},
      {"Slow", "Lento"},
      {"Sold.", "Vendido."},
      {"Soldier", "Soldado"},
      {"Speed", "Velocidade"},
      {"Success", "Sucesso"},
      {"Superior Accuracy Orb", "Orbe de Precisao Superior"},
      {"Superior Critical Orb", "Orbe de Critico Superior"},
      {"Superior Evasion Orb", "Orbe de Evasao Superior"},
      {"Swiftness", "Agilidade"},
      {"Tech", "Tecnica"},
      {"The character data does not exist.", "Os dados do personagem nao existem."},
      {"The duel will begin shortly.", "O duelo comecara em instantes."},
      {"The game has been saved.", "O jogo foi salvo."},
      {"A network error has occurred.\nError Status = %d", "Ocorreu um erro de rede.\nCodigo de Erro = %d"},
      {"GAMEVIL Inc.\nVer %s\nwww.gamevil.com\n\nIf you have any questions, please use the Customer Inquiry board at our official website.", "GAMEVIL Inc.\nVer %s\nwww.gamevil.com\n\nSe voce tiver alguma duvida, use o painel de Atendimento ao Cliente no nosso site oficial."},
      {"Success. \n<C%d>%s<C> acquired", "Sucesso. \n<C%d>%s<C> adquirido"},
      {"Transferring data...\nPlease do not terminate the game.", "Transferindo dados...\nNao encerre o jogo."},
      {"item has been obtained.\n%s acquired", "item obtido.\n%s adquirido"},
      {"The game has ended.", "O jogo terminou."},
      {"The gift has been sent.", "O presente foi enviado."},
      {"The network is busy. Please try again later.(%d)", "A rede esta ocupada. Tente novamente mais tarde.(%d)"},
      {"The network is not responding.", "A rede nao esta respondendo."},
      {"The recipe is already added.", "A receita ja foi adicionada."},
      {"The requested data does not exist.", "Os dados solicitados nao existem."},
      {"Thief", "Ladrao"},
      {"This bag still contains some items. Please empty it first.", "Esta mochila ainda contem alguns itens. Esvazie-a primeiro."},
      {"This cannot be traded.", "Isto nao pode ser negociado."},
      {"This pet cannot evolve any further.", "Este mascote nao pode evoluir mais."},
      {"This phone number does not exist.", "Este numero de telefone nao existe."},
      {"Thorium", "Torio"},
      {"Thwart the resurrection.", "Impedir a ressurreicao."},
      {"Tiger", "Tigre"},
      {"Titan", "Tita"},
      {"Title acquired:<C9>%s<C>", "Titulo obtido:<C9>%s<C>"},
      {"To Bergheim", "Para Bergheim"},
      {"To Fermion", "Para Fermion"},
      {"To Lagrant", "Para Lagrant"},
      {"To Magus", "Para Magus"},
      {"To Urpa", "Para Urpa"},
      {"Total Ranking", "Ranking Total"},
      {"Transferring data... Please do not terminate the game.", "Transferindo dados... Nao encerre o jogo."},
      {"Transparency", "Transparencia"},
      {"Unidentified ", "Nao Identificado "},
      {"Unidentified item.", "Item nao identificado."},
      {"Urpa's Friend", "Amigo de Urpa"},
      {"Vibration", "Vibracao"},
      {"Weasel", "Doninha"},
      {"Weekly", "Semanal"},
      {"Weekly Ranking", "Ranking Semanal"},
      {"Wickedness", "Maldade"},
      {"Wild", "Selvagem"},
      {"Win:", "Vitorias:"},
      {"Wolf", "Lobo"},
      {"You Won", "Voce Venceu"},
      {"You do not have enough money.", "Voce nao tem dinheiro suficiente."},
      {"You do not meet the stat requirements.", "Voce nao atende aos requisitos de atributos."},
      {"Your bag is full.", "Sua mochila esta cheia."},
      {"Your class has been changed.", "Sua classe foi alterada."},
      {"Your level is too low.", "Seu nivel e muito baixo."},
      {"Your pet's bag is full.", "A mochila do seu mascote esta cheia."},
      {"Your previous bag has more space than the new one.", "Sua mochila anterior tem mais espaco que a nova."},
      {"Your specialties have been reset.", "Suas especialidades foram redefinidas."},
      {"Your stats have been reset.", "Seus atributos foram redefinidos."},
      {"[Data Management]", "[Gerenciamento de Dados]"},
      {"[Manage Data]", "[Gerenciar Dados]"},
      {"[Shop]", "[Loja]"},
  };
  return translations;
}

static std::string translatePortugueseString(const std::string& source) {
  auto exact = portugueseText().find(source);
  if (exact != portugueseText().end()) return exact->second;
  struct ItemNoun {
    const char* suffix;
    const char* noun;
    bool feminine;
    bool plural;
  };
  static const ItemNoun itemNouns[] = {
      {"Weapon Chest", "Bau de Arma", false, false}, {"Armor Chest", "Bau de Armadura", false, false},
      {"Jewelry Box", "Caixa de Joias", true, false}, {"HP Potion", "Pocao de PV", true, false},
      {"MP Potion", "Pocao de PM", true, false}, {"HP Vial", "Frasco de PV", false, false},
      {"MP Vial", "Frasco de PM", false, false}, {"Gloves", "Luvas", true, true},
      {"Shoes", "Botas", true, true}, {"Armor", "Armadura", true, false},
      {"Sword", "Espada", true, false}, {"Ring", "Anel", false, false},
      {"Helmet", "Capacete", false, false}, {"Helm", "Elmo", false, false},
      {"Hood", "Capuz", false, false}, {"Gun", "Arma", true, false},
      {"Cannon", "Canhao", false, false}, {"Rifle", "Rifle", false, false}};
  for (const auto& item : itemNouns) {
    const std::string suffix = item.suffix;
    if (source.size() <= suffix.size() ||
        source.compare(source.size() - suffix.size(), suffix.size(), suffix) != 0 ||
        source[source.size() - suffix.size() - 1] != ' ')
      continue;

    std::string prefix = source.substr(0, source.size() - suffix.size() - 1);
    static const std::map<std::string, std::string> descriptors = {
        {"Noob", "de Novato"}, {"Crude", "Rustico"}, {"Simple", "Simples"},
        {"Inferior", "Inferior"}, {"Mediocre", "Medio"}, {"Average", "Medio"},
        {"Superior", "Superior"}, {"Supreme", "Supremo"}, {"Celestial", "Celestial"},
        {"Old", "Antigo"}, {"Worn", "Gasto"}, {"Iron", "de Ferro"},
        {"Scale", "de Escamas"}, {"Suede", "de Camurca"}, {"Chain", "de Malha"},
        {"Refined", "Refinado"}, {"Bone", "de Osso"}, {"Silver", "de Prata"},
        {"Silvery", "Prateado"}, {"Hunting", "de Caca"}, {"Hunter", "de Cacador"},
        {"Gold", "de Ouro"}, {"Golden", "Dourado"}, {"Fighter", "de Guerreiro"},
        {"Fight", "de Combate"}, {"Great", "Grande"}, {"Rune", "de Runa"},
        {"Battle", "de Batalha"}, {"War", "de Guerra"}, {"Savage", "Selvagem"},
        {"Divine", "Divino"}, {"Holy", "Sagrado"}, {"Ancient", "Antigo"},
        {"Active Volcano", "do Vulcao Ativo"}, {"Dragon Scale", "de Escama de Dragao"},
        {"Bloodlust", "com Sede de Sangue"}, {"Goddess", "da Deusa"}, {"Zeus", "de Zeus"},
        {"Death", "da Morte"}, {"Hell Destroyer", "do Destruidor do Inferno"},
        {"Vanquisher", "Vencedor"}, {"Eternal", "Eterno"}, {"Flawless", "Impecavel"},
        {"Hand", "de Mao"}, {"Small", "Pequeno"}, {"Noob", "de Novato"},
        {"Infernal Giant", "do Gigante Infernal"}, {"Berserk", "Berserker"},
        {"Knight", "de Cavaleiro"}, {"Dark", "Sombrio"}, {"Flame", "Flamejante"},
        {"Glacier", "Glacial"}, {"Volcano", "Vulcanico"}, {"Storm", "da Tempestade"},
        {"Tornado", "Tornado"}, {"Jagged", "Serrilhado"}, {"Long", "Longa"},
        {"Broad", "Larga"}, {"Bastard", "Bastarda"}, {"Yaksha", "Yaksha"},
        {"Master", "Mestre"}, {"Infernal", "Infernal"}, {"Tazzler", "Atordoante"},
        {"Wyatt", "Wyatt"}, {"Shield", "de Escudo"}, {"Heaven", "Celestial"},
        {"Accurate", "Preciso"}, {"Twister", "Tornado"}, {"Lethal", "Letal"},
        {"Demolisher", "Demolidor"}, {"Anti-frost", "Antigelo"}, {"Fiend", "Demonio"},
        {"Hell", "Infernal"}, {"Infinite", "Infinito"}, {"Spell", "Magico"},
        {"Ice", "de Gelo"}, {"Glass", "de Vidro"}, {"Jade", "de Jade"},
        {"Crystal", "de Cristal"}, {"Cubic", "Cubico"}, {"Sapphire", "de Safira"},
        {"Topaz", "de Topazio"}, {"Magic", "Magico"}, {"Ruby", "de Rubi"},
        {"Diamond", "de Diamante"}, {"Monarch", "do Monarca"}, {"Spiritual", "Espiritual"},
        {"Legend", "Lendario"}, {"Copper", "de Cobre"}, {"Bronze", "de Bronze"}};
    auto descriptor = descriptors.find(prefix);
    if (descriptor == descriptors.end()) continue;
    std::string translatedDescriptor = descriptor->second;
    if (item.feminine) {
      if (prefix == "Crude") translatedDescriptor = "Rustica";
      else if (prefix == "Inferior") translatedDescriptor = "Inferior";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Media";
      else if (prefix == "Superior") translatedDescriptor = "Superior";
      else if (prefix == "Supreme") translatedDescriptor = "Suprema";
      else if (prefix == "Old") translatedDescriptor = "Antiga";
      else if (prefix == "Worn") translatedDescriptor = "Gasta";
      else if (prefix == "Refined") translatedDescriptor = "Refinada";
      else if (prefix == "Silvery") translatedDescriptor = "Prateada";
      else if (prefix == "Golden") translatedDescriptor = "Dourada";
      else if (prefix == "Savage") translatedDescriptor = "Selvagem";
      else if (prefix == "Divine") translatedDescriptor = "Divina";
      else if (prefix == "Holy") translatedDescriptor = "Sagrada";
      else if (prefix == "Ancient") translatedDescriptor = "Antiga";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedora";
      else if (prefix == "Eternal") translatedDescriptor = "Eterna";
      else if (prefix == "Flawless") translatedDescriptor = "Impecavel";
      else if (prefix == "Small") translatedDescriptor = "Pequena";
      else if (prefix == "Bastard") translatedDescriptor = "Bastarda";
      else if (prefix == "Jagged") translatedDescriptor = "Serrilhada";
      else if (prefix == "Long") translatedDescriptor = "Longa";
      else if (prefix == "Broad") translatedDescriptor = "Larga";
      else if (prefix == "Glacier") translatedDescriptor = "Glacial";
      else if (prefix == "Volcano") translatedDescriptor = "Vulcanica";
      else if (prefix == "Accurate") translatedDescriptor = "Precisa";
      else if (prefix == "Lethal") translatedDescriptor = "Letal";
      else if (prefix == "Cubic") translatedDescriptor = "Cubica";
      else if (prefix == "Legend") translatedDescriptor = "Lendaria";
    } else {
      if (prefix == "Crude") translatedDescriptor = "Rustico";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Medio";
      else if (prefix == "Supreme") translatedDescriptor = "Supremo";
      else if (prefix == "Old") translatedDescriptor = "Antigo";
      else if (prefix == "Worn") translatedDescriptor = "Gasto";
      else if (prefix == "Refined") translatedDescriptor = "Refinado";
      else if (prefix == "Silvery") translatedDescriptor = "Prateado";
      else if (prefix == "Golden") translatedDescriptor = "Dourado";
      else if (prefix == "Divine") translatedDescriptor = "Divino";
      else if (prefix == "Holy") translatedDescriptor = "Sagrado";
      else if (prefix == "Ancient") translatedDescriptor = "Antigo";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedor";
      else if (prefix == "Eternal") translatedDescriptor = "Eterno";
      else if (prefix == "Small") translatedDescriptor = "Pequeno";
      else if (prefix == "Flawless") translatedDescriptor = "Impecavel";
      else if (prefix == "Legend") translatedDescriptor = "Lendario";
    }
    if (item.plural) {
      if (prefix == "Crude") translatedDescriptor = "Rusticas";
      else if (prefix == "Inferior") translatedDescriptor = "Inferiores";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Medias";
      else if (prefix == "Superior") translatedDescriptor = "Superiores";
      else if (prefix == "Supreme") translatedDescriptor = "Supremas";
      else if (prefix == "Celestial") translatedDescriptor = "Celestiais";
      else if (prefix == "Old") translatedDescriptor = "Antigas";
      else if (prefix == "Worn") translatedDescriptor = "Gastas";
      else if (prefix == "Simple") translatedDescriptor = "Simples";
      else if (prefix == "Refined") translatedDescriptor = "Refinadas";
      else if (prefix == "Silvery") translatedDescriptor = "Prateadas";
      else if (prefix == "Golden") translatedDescriptor = "Douradas";
      else if (prefix == "Savage") translatedDescriptor = "Selvagens";
      else if (prefix == "Divine") translatedDescriptor = "Divinas";
      else if (prefix == "Holy") translatedDescriptor = "Sagradas";
      else if (prefix == "Ancient") translatedDescriptor = "Antigas";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedoras";
      else if (prefix == "Eternal") translatedDescriptor = "Eternas";
      else if (prefix == "Flawless") translatedDescriptor = "Impecaveis";
      else if (prefix == "Small") translatedDescriptor = "Pequenas";
      else if (prefix == "Great") translatedDescriptor = "Grandes";
    }
    return std::string(item.noun) + " " + translatedDescriptor;
  }

  const std::string produced = "Produces a ";
  if (source.compare(0, produced.size(), produced) == 0) {
    size_t weaponAt = source.find(" weapon (Lv ");
    size_t armorAt = source.find(" armor item (Lv ");
    size_t kindAt = weaponAt != std::string::npos ? weaponAt : armorAt;
    if (kindAt != std::string::npos) {
      std::string grade = source.substr(produced.size(), kindAt - produced.size());
      std::string translatedGrade = translatePortugueseString(grade);
      if (translatedGrade.compare(0, 4, "Raro") == 0 &&
          (translatedGrade.size() == 4 || translatedGrade[4] == ' '))
        translatedGrade.replace(0, 4, "Rara");
      if (!translatedGrade.empty())
        translatedGrade[0] = (char)std::tolower((unsigned char)translatedGrade[0]);
      const size_t typeLength = weaponAt != std::string::npos ? 7 : 11;
      std::string range = source.substr(kindAt + typeLength);
      std::string kind = weaponAt != std::string::npos ? "uma arma " : "uma peca de armadura ";
      return "Produz " + kind + translatedGrade + range;
    }
  }

  static const std::vector<std::pair<std::string, std::string>> ordered = [] {
    std::vector<std::pair<std::string, std::string>> rules(portugueseText().begin(), portugueseText().end());
    std::stable_sort(rules.begin(), rules.end(), [](const auto& left, const auto& right) {
      return left.first.size() > right.first.size();
    });
    return rules;
  }();
  std::string result = source;
  for (const auto& entry : ordered) {
    if ((entry.first.size() < 4 && entry.first != "HP" && entry.first != "MP") ||
        result.find(entry.first) == std::string::npos) continue;
    size_t at = 0;
    while ((at = result.find(entry.first, at)) != std::string::npos) {
      auto isWordCharacter = [](unsigned char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_';
      };
      bool leftBoundary = entry.first.empty() || !isWordCharacter((unsigned char)entry.first.front()) ||
                          at == 0 || !isWordCharacter((unsigned char)result[at - 1]);
      size_t end = at + entry.first.size();
      bool rightBoundary = entry.first.empty() || !isWordCharacter((unsigned char)entry.first.back()) ||
                           end == result.size() || !isWordCharacter((unsigned char)result[end]);
      if (!leftBoundary || !rightBoundary) {
        at += entry.first.size();
        continue;
      }
      result.replace(at, entry.first.size(), entry.second);
      at += entry.second.size();
    }
  }
  return result;
}

static bool translateTable(std::vector<uint8_t>& data) {
  if (data.size() < 2) return false;
  uint16_t count = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  size_t pos = 2;
  std::vector<uint8_t> translated;
  translated.reserve(data.size() + 8);
  translated.push_back(data[0]);
  translated.push_back(data[1]);
  bool changed = false;

  for (uint16_t i = 0; i < count; ++i) {
    if (pos + 2 > data.size()) return false;
    uint16_t length = (uint16_t)data[pos] | ((uint16_t)data[pos + 1] << 8);
    pos += 2;
    if (pos + length > data.size()) return false;

    const uint8_t* text = data.data() + pos;
    std::string original((const char*)text, length);
    std::string replacement = translatePortugueseString(original);
    if (replacement.size() <= UINT16_MAX && replacement != original) {
      uint16_t newLength = (uint16_t)replacement.size();
      translated.push_back((uint8_t)newLength);
      translated.push_back((uint8_t)(newLength >> 8));
      translated.insert(translated.end(), replacement.begin(), replacement.end());
      changed = true;
    } else {
      translated.push_back((uint8_t)length);
      translated.push_back((uint8_t)(length >> 8));
      translated.insert(translated.end(), text, text + length);
    }
    pos += length;
  }

  if (pos != data.size()) return false;
  if (changed) data = std::move(translated);
  return changed;
}

static bool translateGdt(const std::string& name, std::vector<uint8_t>& data) {
  bool changed = false;
  size_t skippedForCapacity = 0;
  size_t pos = 0;
  while (pos < data.size()) {
    while (pos < data.size() && data[pos] == 0) pos++;
    size_t start = pos;
    while (pos < data.size() && data[pos] != 0) pos++;
    if (start == pos || pos == data.size() || pos - start > 512) continue;
    std::string original((const char*)data.data() + start, pos - start);
    if (std::any_of(original.begin(), original.end(), [](unsigned char c) { return c < 0x20 || c > 0x7e; }))
      continue;
    std::string replacement = translatePortugueseString(original);
    if (replacement == original) continue;
    size_t fieldEnd = pos;
    while (fieldEnd < data.size() && data[fieldEnd] == 0) fieldEnd++;
    if (replacement.size() + 1 > fieldEnd - start) {
      skippedForCapacity++;
      continue;
    }
    std::fill(data.begin() + start, data.begin() + fieldEnd, 0);
    std::copy(replacement.begin(), replacement.end(), data.begin() + start);
    changed = true;
  }
  if (skippedForCapacity)
    note("traducao PT: " + std::to_string(skippedForCapacity) +
         " texto(s) mantido(s) em ingles por falta de espaco em " + name);
  return changed;
}

static bool isPortugueseAsset(const std::string& name) {
  static const std::vector<std::string> tables = {
      "gametxt_en.tdt", "gametxt_f_en.tdt", "equip_name_en.tdt", "equip_head_en.tdt",
      "qitem_name_en.tdt", "skill0_desc_en.tdt", "skill1_desc_en.tdt", "skill2_desc_en.tdt",
      "pet_skill_en.tdt", "attr0_desc_en.tdt", "attr1_desc_en.tdt", "attr2_desc_en.tdt"};
  static const std::vector<std::string> records = {
      "item_en.gdt", "quest_en.gdt", "quest_hard_en.gdt", "mon_data_en.gdt"};
  return std::find(tables.begin(), tables.end(), name) != tables.end() ||
         std::find(records.begin(), records.end(), name) != records.end();
}

static bool applyPortugueseText(const std::string& name, std::vector<uint8_t>& data) {
  if (!portugueseTextTest || !isPortugueseAsset(name)) return false;
  bool changed = false;
  if (name.size() >= 4 && name.substr(name.size() - 4) == ".tdt") changed = translateTable(data);
  else if (name.size() >= 4 && name.substr(name.size() - 4) == ".gdt") changed = translateGdt(name, data);
  if (!changed) note("traducao PT: nenhum texto reconhecido em " + name);
  return changed;
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

static void setupStubs() {
  u32 n = ID_LIBC;
  auto L = [&](const char* name, std::function<void()> f) { importId[name] = n; stubSet(n, name, std::move(f)); n++; };

  L("memcpy", [] { u32 d = R(0), s = R(1), c = R(2); std::vector<uint8_t> b(c); rd(s, b.data(), c); wr(d, b.data(), c); W(0, d); });
  L("memset", [] {
    u32 d = R(0), c = R(2); std::vector<uint8_t> b(c, (uint8_t)R(1)); wr(d, b.data(), c); W(0, d);
    if (c == 0x3928 && !gMMain) { gMMain = d; if (probe) fprintf(stderr, "[sonda] MMain em %08x (lib+%x)\n", d, d - SO_BASE); }
    else if (c == 0x214 && !gMenu) { gMenu = d; if (probe) fprintf(stderr, "[sonda] Menu em %08x (lib+%x)\n", d, d - SO_BASE); }
    else if (c == 0x7c && gMMain && !gGrp) { gGrp = d; if (probe) fprintf(stderr, "[sonda] Grp em %08x (lib+%x)\n", d, d - SO_BASE); }
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
    if (m.find('w') != std::string::npos || m.find('a') != std::string::npos || m.find('+') != std::string::npos)
      fs::create_directories(fs::path(p).parent_path());
    FILE* f = fopen(p.c_str(), m.c_str());
    note("fopen " + p + " [" + m + "] -> " + (f ? "ok" : "falhou"));
    if (!f) { W(0, 0); return; }
    u32 h = nextFile; nextFile += 16; files[h] = f; W(0, h);
  });
  L("fclose", [] { auto it = files.find(R(0)); if (it != files.end()) { fclose(it->second); files.erase(it); } W(0, 0); });
  L("fread", [] {
    auto it = files.find(R(3)); if (it == files.end()) { W(0, 0); return; }
    u32 sz = R(1), cnt = R(2); std::vector<uint8_t> b((size_t)sz * cnt);
    size_t got = fread(b.data(), sz ? sz : 1, cnt, it->second);
    wr(R(0), b.data(), got * sz); W(0, (u32)got);
  });
  L("fwrite", [] {
    auto it = files.find(R(3)); if (it == files.end()) { W(0, 0); return; }
    u32 sz = R(1), cnt = R(2); std::vector<uint8_t> b((size_t)sz * cnt); rd(R(0), b.data(), b.size());
    W(0, (u32)fwrite(b.data(), sz ? sz : 1, cnt, it->second));
  });
  L("fseek", [] { auto it = files.find(R(0)); W(0, it == files.end() ? (u32)-1 : (u32)fseek(it->second, (long)(int)R(1), (int)R(2))); });
  L("ftell", [] { auto it = files.find(R(0)); W(0, it == files.end() ? (u32)-1 : (u32)ftell(it->second)); });
  L("remove", [] { std::string p = hostPath(cstr(R(0))); W(0, (u32)remove(p.c_str())); });
  L("__android_log_print", [] { if (verbose) { std::string s = fmtPrintf(R(2), 3); fprintf(stderr, "[jogo:%s] %s\n", cstr(R(1)).c_str(), s.c_str()); } });
  L("__stack_chk_fail", [] { fprintf(stderr, "[host] __stack_chk_fail chamado (pilha corrompida)\n"); fatal = true; uc_emu_stop(uc); });
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
      if (translated) fprintf(stderr, "[host] textos PT aplicados em %s\n", nm.c_str());
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
        static const u32 cashByProduct[] = {170000, 80000, 45000, 14000, 6500, 1000};
        if (a0 < sizeof(cashByProduct) / sizeof(cashByProduct[0])) {
          pendingCashAmount = cashByProduct[a0];
          pendingBuyCash = true;
          note("buyCash(" + std::to_string((int)a0) + ") -> compra local gratuita de " + std::to_string(pendingCashAmount) + " cash");
        } else {
          pendingCashAmount = 0;
          pendingBuyCash = false;
          note("buyCash(" + std::to_string((int)a0) + ") ignorado: produto desconhecido");
        }
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
struct Ehdr { uint8_t ident[16]; uint16_t type, machine; u32 version, entry, phoff, shoff, flags; uint16_t ehsize, phentsize, phnum, shentsize, shnum, shstrndx; };
struct Phdr { u32 type, offset, vaddr, paddr, filesz, memsz, flags, align; };
struct Dyn { int32_t tag; u32 val; };
struct Sym { u32 name, value, size; uint8_t info, other; uint16_t shndx; };
struct Rel { u32 offset, info; };

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
  fprintf(stderr, "[trace] viewport recording stopped (%s).\n", reason);
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
  if (ec) { fprintf(stderr, "[trace] nao consegui criar %s: %s\n", traceViewportOutDir.c_str(), ec.message().c_str()); return false; }
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  const fs::path path = fs::path(traceViewportOutDir) /
      ("viewport_trace_" + std::to_string(millis) + ".log");
  fs::path uniquePath = path;
  unsigned int suffix = 1;
  while (fs::exists(uniquePath, ec)) {
    if (ec) { fprintf(stderr, "[trace] erro verificando %s: %s\n", uniquePath.string().c_str(), ec.message().c_str()); return false; }
    uniquePath = fs::path(traceViewportOutDir) /
        ("viewport_trace_" + std::to_string(millis) + "_" + std::to_string(suffix++) + ".log");
  }
  if (ec) { fprintf(stderr, "[trace] erro verificando %s: %s\n", uniquePath.string().c_str(), ec.message().c_str()); return false; }
  traceViewportLog.open(uniquePath, std::ios::out | std::ios::trunc);
  if (!traceViewportLog) { fprintf(stderr, "[trace] nao consegui abrir %s\n", uniquePath.string().c_str()); return false; }
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
  fprintf(stderr, "[trace] viewport pronto: %s; pressione F9 para iniciar/parar o registro (limite %ld quadros por captura).\n",
          uniquePath.string().c_str(), traceViewportFrames);
  return true;
}

static u32 resolveImport(const std::string& name) {
  if (name == "__stack_chk_guard") return GUARD;
  auto it = importId.find(name);
  if (it != importId.end()) return stubAddr(it->second);
  u32 id = nextMissing++;
  stubSet(id, "FALTANDO:" + name, [name] { fprintf(stderr, "[host] a .so chamou a funcao nao implementada: %s\n", name.c_str()); fatal = true; uc_emu_stop(uc); });
  importId[name] = id;
  fprintf(stderr, "[host] aviso: importacao sem implementacao: %s\n", name.c_str());
  return stubAddr(id);
}

static bool loadSo(const std::string& path) {
  std::vector<uint8_t> file;
  if (!readFile(path, file)) { fprintf(stderr, "Nao consegui abrir %s\n", path.c_str()); return false; }
  if (file.size() < sizeof(Ehdr) || memcmp(file.data(), "\x7f" "ELF", 4) != 0) { fprintf(stderr, "Nao e um ELF.\n"); return false; }
  Ehdr eh; memcpy(&eh, file.data(), sizeof eh);
  if (eh.machine != 40) { fprintf(stderr, "ELF nao e ARM (machine=%d)\n", eh.machine); return false; }
  u32 span = 0;
  std::vector<Phdr> ph(eh.phnum);
  for (int i = 0; i < eh.phnum; i++) { memcpy(&ph[i], file.data() + eh.phoff + i * eh.phentsize, sizeof(Phdr)); if (ph[i].type == 1) span = std::max(span, ph[i].vaddr + ph[i].memsz); }
  span = (span + 0xFFF) & ~0xFFFu;
  std::vector<uint8_t> img(span, 0);
  u32 dynAddr = 0;
  for (auto& p : ph) {
    if (p.type == 1) memcpy(img.data() + p.vaddr, file.data() + p.offset, p.filesz);
    if (p.type == 2) dynAddr = p.vaddr;
  }
  if (!dynAddr) { fprintf(stderr, "Sem secao dinamica.\n"); return false; }
  u32 hash = 0, strtab = 0, symtab = 0, rel = 0, relsz = 0, jmprel = 0, pltrelsz = 0;
  for (Dyn d; ; dynAddr += 8) {
    memcpy(&d, img.data() + dynAddr, 8);
    if (d.tag == 0) break;
    if (d.tag == 4) hash = d.val; else if (d.tag == 5) strtab = d.val; else if (d.tag == 6) symtab = d.val;
    else if (d.tag == 17) rel = d.val; else if (d.tag == 18) relsz = d.val; else if (d.tag == 23) jmprel = d.val; else if (d.tag == 2) pltrelsz = d.val;
  }
  u32 nsyms; memcpy(&nsyms, img.data() + hash + 4, 4);  // DT_HASH: nchain = numero de simbolos
  auto sym = [&](u32 i) { Sym s; memcpy(&s, img.data() + symtab + i * 16, 16); return s; };
  auto symName = [&](const Sym& s) { return std::string((const char*)img.data() + strtab + s.name); };

  for (u32 i = 0; i < nsyms; i++) { Sym s = sym(i); if (s.shndx != 0 && s.name) exportsMap[symName(s)] = SO_BASE + s.value; }

  auto applyRel = [&](u32 off, u32 sz) {
    for (u32 i = 0; i < sz / 8; i++) {
      Rel r; memcpy(&r, img.data() + off + i * 8, 8);
      u32 type = r.info & 0xFF, si = r.info >> 8, *where = (u32*)(img.data() + r.offset);
      if (type == 23) *where += SO_BASE;                                           // R_ARM_RELATIVE
      else if (type == 21 || type == 22 || type == 2) {                            // GLOB_DAT / JUMP_SLOT / ABS32
        Sym s = sym(si); std::string nm = symName(s);
        u32 addr = s.shndx != 0 ? SO_BASE + s.value : resolveImport(nm);
        if (type == 2) *where += addr; else *where = addr;
      } else fprintf(stderr, "[host] aviso: relocacao tipo %u ignorada\n", type);
    }
  };
  applyRel(rel, relsz);
  applyRel(jmprel, pltrelsz);

  if (uc_mem_map(uc, SO_BASE, span, UC_PROT_ALL) != UC_ERR_OK) { fprintf(stderr, "Falha ao mapear a .so\n"); return false; }
  wr(SO_BASE, img.data(), img.size());
  loadedSoSpan = span;
  fprintf(stderr, "[host] .so carregada: %u KB, %zu simbolos exportados\n", span / 1024, exportsMap.size());
  return true;
}

// ---------------------------------------------------------------- execucao
static void dumpState(const char* why) {
  fprintf(stderr, "\n=== PROBLEMA: %s ===\n", why);
  u32 pc = regPC();
  fprintf(stderr, "pc=0x%08x (na .so: 0x%x, no Ghidra: 0x%x)  lr=0x%08x  sp=0x%08x\n", pc, pc - SO_BASE, pc - SO_BASE + 0x10000, regLR(), regSP());
  for (int i = 0; i < 13; i++) fprintf(stderr, "r%d=0x%08x%s", i, R(i), (i % 4 == 3) ? "\n" : "  ");
  fprintf(stderr, "\nultimas chamadas ao host:\n");
  for (auto& s : ring) fprintf(stderr, "  %s\n", s.c_str());
  fprintf(stderr, "Copie tudo isto e mande para o Claude.\n");
}

static void stubHook(uc_engine*, uint64_t address, uint32_t, void*) {
  if (address == MAGIC_RET) { hitMagic = true; uc_emu_stop(uc); return; }
  u32 id = ((u32)address - STUB_BASE) / 4;
  if (id < STUB_COUNT && handlers[id]) handlers[id]();
  else { fprintf(stderr, "[host] stub sem handler id=0x%x\n", id); fatal = true; uc_emu_stop(uc); }
}

static bool callFn(u32 addr, std::initializer_list<u32> args, u32* ret) {
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
  if (e != UC_ERR_OK) { dumpState(uc_strerror(e)); return false; }
  if (fatal) { dumpState("chamada fatal"); return false; }
  if (!hitMagic && (regPC() & ~1u) == MAGIC_RET) hitMagic = true;  // o Unicorn para ANTES de executar o endereco final
  if (!hitMagic) { dumpState("tempo esgotado (a .so ficou presa num laco ou parou)"); return false; }
  if (ret) *ret = R(0);
  return true;
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
    fprintf(stderr, "[trace] BMP ignorado: framebuffer incompleto (%zu de %u bytes).\n",
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
      fprintf(stderr, "[trace] nao consegui verificar destino do BMP %s: %s\n",
              path.string().c_str(), ec.message().c_str());
      return false;
    }
  }
  if (ec) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " framebuffer_bmp_error="
                     << ec.message() << "\n";
    fprintf(stderr, "[trace] nao consegui verificar destino do BMP %s: %s\n",
            path.string().c_str(), ec.message().c_str());
    return false;
  }
  if (!writeBmp(path.string(), frameBuffer.data())) {
    traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                     << " frame=" << frame << " framebuffer_bmp_error=write-failed path="
                     << path.string() << "\n";
    fprintf(stderr, "[trace] falha ao gravar BMP %s\n", path.string().c_str());
    return false;
  }

  traceViewportLog << "[trace] timestamp_ms=" << traceViewportTimestampMs()
                   << " frame=" << frame << " framebuffer_bmp=" << path.string() << "\n";
  fprintf(stderr, "[trace] framebuffer salvo: %s\n", path.string().c_str());
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
  fprintf(stderr, "[trace] viewport recording started at frame %ld.\n", frame);
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

// ---------------------------------------------------------------- principal
int main(int argc, char** argv) {
  bool fullscreen = false;
  int scale = 3, headlessFrames = 0; std::string outDir = "frames";
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    auto next = [&]() { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
    if (a == "--game") gameDir = next(); else if (a == "--so") soPath = next(); else if (a == "--save") saveDir = next();
    else if (a == "--scale") scale = atoi(next().c_str()); else if (a == "--headless") headlessFrames = atoi(next().c_str());
    else if (a == "--out") outDir = next(); else if (a == "-v") verbose = true;
    else if (a == "--fullscreen") fullscreen = true;
    else if (a == "--probe") probe = true;
    else if (a == "--trace-camera") traceCamera = true;
    else if (a == "--no-fix-map-edge") fixMapEdge = false;
    else if (a == "--profile") profile = true;
    else if (a == "--mute") muteAudio = true;
    else if (a == "--purchase-cancel-test") purchaseCancelTest = true;
    else if (a == "--lang" && next() == "pt") portugueseTextTest = true;
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
    else { printf("Uso: destinia_host [--game PASTA] [--so ARQUIVO] [--save PASTA] [--scale N] [--fullscreen] [--lang pt] [--headless QUADROS --out PASTA] [--trace-viewport --trace-frames N --trace-out PASTA] [-v] [--probe] [--profile] [--mute] [--autotap QUADRO,X,Y] [--autotap2 QUADRO,X,Y]\n"); return 0; }
  }
  if (soPath.empty()) soPath = gameDir + "/lib/armeabi/libdestinia_jni.so";
  assetsDir = gameDir + "/assets";
  fs::create_directories(saveDir);

  if (uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc) != UC_ERR_OK) { fprintf(stderr, "uc_open falhou\n"); return 1; }
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
    if (!handlers[id]) { stubSet(id, "JNI.#" + std::to_string(i), [i] { fprintf(stderr, "[host] JNI nao implementado: indice %d\n", i); fatal = true; uc_emu_stop(uc); }); }
    wr32(ENV_TAB + 4 * i, stubAddr(id));
  }
  wr32(VM_OBJ, VM_TAB);
  for (int i = 0; i < 8; i++) {
    u32 id = ID_VM + i;
    if (!handlers[id]) stubSet(id, "VM.#" + std::to_string(i), [i] { fprintf(stderr, "[host] JavaVM nao implementado: indice %d\n", i); fatal = true; uc_emu_stop(uc); });
    wr32(VM_TAB + 4 * i, stubAddr(id));
  }
  wr32(GUARD, 0xA5C3F00D);

  if (!loadSo(soPath)) return 1;
  uc_hook hk; uc_hook_add(uc, &hk, UC_HOOK_CODE, (void*)stubHook, nullptr, STUB_BASE, STUB_BASE + STUB_COUNT * 4 - 1);
  uc_hook hk2; uc_hook_add(uc, &hk2, UC_HOOK_CODE, (void*)stubHook, nullptr, MAGIC_RET, MAGIC_RET + 3);
  if (exportsMap["Game"] != SO_BASE + 0x77EE0 || exportsMap["curMapData"] != SO_BASE + 0x80B18) {
    fprintf(stderr, "[host] Layout ARM incompativel com a camera do zoom.\n"); return 1;
  }
  uc_hook cameraHook;
  if (uc_hook_add(uc, &cameraHook, UC_HOOK_MEM_READ, (void*)cameraReadHook, nullptr,
                  SO_BASE + 0x77EE0 + 18552, SO_BASE + 0x77EE0 + 18555) != UC_ERR_OK) {
    fprintf(stderr, "[host] Falha ao registrar centralizacao da camera.\n"); return 1;
  }
  if (fixMapEdge) {
    uc_hook hk3, hk4;
    uc_hook_add(uc, &hk3, UC_HOOK_CODE, (void*)mapEdgeHook, nullptr, SO_BASE + 0xBB90, SO_BASE + 0xBB90);
    uc_hook_add(uc, &hk4, UC_HOOK_CODE, (void*)mapEdgeHook, nullptr, SO_BASE + 0xB4BC, SO_BASE + 0xB4BC);
    uc_hook hk5;
    uc_hook_add(uc, &hk5, UC_HOOK_CODE, (void*)mapEdgeClearHook, nullptr, SO_BASE + 0x19308, SO_BASE + 0x19308);
  }

  for (const char* n : {"Java_game_destiniaeng_GameThread_initGame", "Java_game_destiniaeng_GameThread_jniRun", "Java_game_destiniaeng_GameThread_jniTouch"})
    if (!exportsMap.count(n)) { fprintf(stderr, "Simbolo nao encontrado: %s\n", n); return 1; }
  if (traceViewport && !startTraceViewport()) return 1;
  u32 fnInit = exportsMap["Java_game_destiniaeng_GameThread_initGame"];
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
  fprintf(stderr, "[host] initGame ok em %dx%d. Rodando o jogo...\n", SCREEN_W, SCREEN_H);
  fprintf(stderr, "[host] zoom out: mapas pequenos centralizados; fundo cinza; UI nativa.\n");
  if (portugueseTextTest) fprintf(stderr, "[host] traducao PT ativa: itens, equipamentos, habilidades, menus, Fabricacao e Missoes.\n");

  // [OK] jniTouch(env,obj, x, y, tipo): tipo 0=toque 1=solta 2=move 3=tecla de menu (Thumb decodificado)
  auto touch = [&](int x, int y, int type) {
    bool ok = callFn(fnTouch, {ENV_OBJ, 0x10, (u32)x, (u32)y, (u32)type});
    if (probe && type != 2) { fprintf(stderr, "[sonda] jniTouch(x=%d, y=%d, tipo=%d) -> %s\n", x, y, type, ok ? "ok" : "FALHOU"); probeState(type == 0 ? "logo apos PRESSIONAR" : type == 1 ? "logo apos SOLTAR" : "logo apos tecla"); }
  };

  SDL_Window* win = nullptr; SDL_Renderer* ren = nullptr; SDL_Texture* tex = nullptr; SDL_GameController* controller = nullptr;
  auto setFullscreen = [&](bool enabled) {
    if (SDL_SetWindowFullscreen(win, enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
      fprintf(stderr, "[host] Nao foi possivel alternar tela cheia: %s\n", SDL_GetError());
      return false;
    }
    fprintf(stderr, "[host] modo de exibicao: %s (proporcao original preservada).\n", enabled ? "tela cheia" : "janela");
    return true;
  };
  if (!headlessFrames) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
    if (!muteAudio) {
#ifdef HAS_SDL_MIXER
      Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
      if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == 0) { Mix_AllocateChannels(16); audioReady = true; }
      else fprintf(stderr, "[host] audio indisponivel: %s\n", Mix_GetError());
#else
      fprintf(stderr, "[host] audio indisponivel: compile com SDL2_mixer para habilitar o som.\n");
#endif
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // pixels nitidos
    win = SDL_CreateWindow("Destinia 0.4 Beta", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE);
    if (fullscreen) setFullscreen(true);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    for (int i = 0; i < SDL_NumJoysticks(); i++) { if (SDL_IsGameController(i)) { controller = SDL_GameControllerOpen(i); if (controller) { fprintf(stderr, "[host] Controle conectado: %s\n", SDL_GameControllerName(controller)); break; } } }
    if (traceViewport)
      fprintf(stderr, "[host] Teclado: WASD/setas mover, Enter/Z confirmar, Esc/X voltar, I menu, F5 save, F11 tela cheia, F9 iniciar/parar trace viewport. DualSense: analogico mover, X atacar, O voltar, triangulo menu, Share save, L1 acelerar.\n");
    else
      fprintf(stderr, "[host] Teclado: WASD/setas mover, Enter/Z confirmar, Esc/X voltar, I menu, F5 save, F11 tela cheia. DualSense: analogico mover, X atacar, O voltar, triangulo menu, Share save, L1 acelerar.\n");
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

  bool running = true, mouseDown = false; long frame = 0;
  auto profileWindowStart = std::chrono::steady_clock::now();
  long profileWindowFrames = 0;
  double profileRunMs = 0.0, profileRenderMs = 0.0, profileSleepMs = 0.0, profileOtherMs = 0.0;
  double profileMaxRunMs = 0.0;
  while (running) {
    traceViewportFrame = frame;
    auto t0 = std::chrono::steady_clock::now();
    double runMs = 0.0, renderMs = 0.0, sleepMeasuredMs = 0.0;
    if (!headlessFrames) {
      SDL_Event ev;
      while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) running = false;
        else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEMOTION) {
          int wx = ev.type == SDL_MOUSEMOTION ? ev.motion.x : ev.button.x, wy = ev.type == SDL_MOUSEMOTION ? ev.motion.y : ev.button.y;
          int x = 0, y = 0;
          const bool inside = mapMouseToGame(wx, wy, x, y, mouseDown && ev.type != SDL_MOUSEBUTTONDOWN);
          if (probe && ev.type != SDL_MOUSEMOTION) {
            int ww = 0, wh = 0, lw = 0, lh = 0;
            SDL_GetWindowSize(win, &ww, &wh);
            SDL_RenderGetLogicalSize(ren, &lw, &lh);
            int ow = 0, oh = 0;
            SDL_GetRendererOutputSize(ren, &ow, &oh);
            fprintf(stderr, "[sonda] SDL %s botao=%d janela_px=(%d,%d) tamanho_janela=%dx%d saida_render=%dx%d tamanho_logico=%dx%d -> jogo=(%d,%d)\n",
                    ev.type == SDL_MOUSEBUTTONDOWN ? "BAIXO" : "CIMA", (int)ev.button.button, wx, wy, ww, wh, ow, oh, lw, lh, x, y);
          }
          if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT && inside) { mouseDown = true; touch(x, y, 0); }
          else if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT && mouseDown) { mouseDown = false; touch(x, y, 1); }
          else if (ev.type == SDL_MOUSEMOTION && mouseDown) touch(x, y, 2);
        } else if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
          if (ev.key.repeat) continue;
          bool down = ev.type == SDL_KEYDOWN;
          switch (ev.key.keysym.sym) {
            case SDLK_F11:
              if (down) setFullscreen((SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN) == 0);
              break;
            case SDLK_F9:
              if (traceViewport && down) toggleTraceViewportRecording(frame, arrays[hScreen]);
              break;
            case SDLK_UP: case SDLK_w: gameKey(3, down); break;
            case SDLK_DOWN: case SDLK_s: gameKey(9, down); break;
            case SDLK_LEFT: case SDLK_a: gameKey(5, down); break;
            case SDLK_RIGHT: case SDLK_d: gameKey(7, down); break;
            case SDLK_RETURN: case SDLK_SPACE: case SDLK_z: gameKey(17, down); break;
            case SDLK_ESCAPE: case SDLK_x: gameKey(10, down); break;
            case SDLK_i: case SDLK_c: gameKey(27, down); break;
            case SDLK_TAB: if (down) gameKey(1, true); else gameKey(1, false); break;
            case SDLK_F5: if (down) touch(0, 0, 3); break;  // save rapido (tecla Android menu)
            case SDLK_LSHIFT: fastForward = down; break;
          }
        }
        else if (ev.type == SDL_CONTROLLERDEVICEADDED && !controller) {
          controller = SDL_GameControllerOpen(ev.cdevice.which);
          if (controller) fprintf(stderr, "[host] Controle plugado: %s\n", SDL_GameControllerName(controller));
        }
        else if (ev.type == SDL_CONTROLLERDEVICEREMOVED && controller) {
          fprintf(stderr, "[host] Controle desplugado.\n");
          SDL_GameControllerClose(controller); controller = nullptr;
        }
        else if (ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP) {
          int b = ev.cbutton.button; bool down = ev.type == SDL_CONTROLLERBUTTONDOWN;
          if (probe && down) fprintf(stderr, "[sonda] Controle BOTAO: %d\n", b);
          if (b == SDL_CONTROLLER_BUTTON_BACK) { if (down) touch(0, 0, 3); }
          else if (b == SDL_CONTROLLER_BUTTON_A) gameKey(17, down);
          else if (b == SDL_CONTROLLER_BUTTON_B) gameKey(10, down);
          else if (b == SDL_CONTROLLER_BUTTON_Y) gameKey(27, down);
          else if (b == SDL_CONTROLLER_BUTTON_X) gameKey(12, down);
          else if (b == SDL_CONTROLLER_BUTTON_START) gameKey(1, down);
          else if (b == SDL_CONTROLLER_BUTTON_DPAD_UP) gameKey(3, down);
          else if (b == SDL_CONTROLLER_BUTTON_DPAD_DOWN) gameKey(9, down);
          else if (b == SDL_CONTROLLER_BUTTON_DPAD_LEFT) gameKey(5, down);
          else if (b == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) gameKey(7, down);
          else if (b == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) fastForward = down;
          else if (b == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) gameKey(12, down);
        }
      }
      if (controller) {
        auto axisDir = [](int ax, int ay) {
          const int dz = 16000;
          if (std::abs(ax) < dz && std::abs(ay) < dz) return 0;
          if (std::abs(ax) > std::abs(ay)) return ax < 0 ? 5 : 7;
          return ay < 0 ? 3 : 9;
        };
        int nd = axisDir(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX),
                         SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));
        if (nd != analogDir) {
          if (analogDir) gameKey(analogDir, false);
          if (nd) gameKey(nd, true);
          analogDir = nd;
        }
        int skill = axisDir(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX),
                            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));
        int skillKey = skill == 5 ? 2 : skill == 7 ? 11 : skill == 3 ? 4 : skill == 9 ? 8 : 0;
        if (skillKey != rightStickKey) {
          if (rightStickKey) gameKey(rightStickKey, false);
          if (skillKey) gameKey(skillKey, true);
          rightStickKey = skillKey;
        }
      }
    }
    // [SONDA] toque sintetico de teste (so com --autotap): pressiona no quadro N e solta 3 quadros depois
    if (autoFrame >= 0 && frame == autoFrame) touch(autoX, autoY, 0);
    if (autoFrame >= 0 && frame == autoFrame + 3) touch(autoX, autoY, 1);
    if (autoFrame2 >= 0 && frame == autoFrame2) touch(autoX2, autoY2, 0);
    if (autoFrame2 >= 0 && frame == autoFrame2 + 3) touch(autoX2, autoY2, 1);
    // [OK] jniRun(env,obj, byte[] framebuffer, int contador) -> ms ate o proximo quadro (0 = sair)
    u32 wait = 0;
    const auto runStart = std::chrono::steady_clock::now();
    traceViewportFrame = frame;
    const bool runSucceeded = callFn(fnRun, {ENV_OBJ, 0x10, hScreen, (u32)frame}, &wait);
    runMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - runStart).count();
    centerSmallMapCamera();
    traceViewportWriteFrameSummary(frame);
    if (!runSucceeded) {
      if (traceViewportRecording) stopTraceViewportRecording("run-error");
      break;
    }
    if (pendingBuyCash) {
      pendingBuyCash = false;
      if (fnGetCash && fnSetCash) {
        u32 cash = 0;
        if (!callFn(fnGetCash, {}, &cash)) break;
        u32 next = cash >= 170000 || pendingCashAmount >= 170000 - cash
                       ? std::max<u32>(cash, 170000)
                       : cash + pendingCashAmount;
        if (!callFn(fnSetCash, {next})) break;
        fprintf(stderr, "[host] cash local %u -> %u.\n", cash, next);
        if (purchaseCancelTest) {
          if (!fnProcCommMsg) {
            fprintf(stderr, "[host] erro: procCommMsg nao foi exportada; teste de cancelamento indisponivel.\n");
            break;
          }
          fprintf(stderr, "[host] teste: enviando resposta nativa de compra cancelada (-10); pode surgir aviso ou ocorrer crash.\n");
          if (!callFn(fnProcCommMsg, {ENV_OBJ, 0x10, 1, static_cast<u32>(-10)})) break;
        } else {
          fprintf(stderr, "[host] resposta da loja nao simulada (spinner pode permanecer).\n");
        }
      } else {
        fprintf(stderr, "[host] erro: getMyCash/setMyCash nao exportadas; cash nao foi alterado.\n");
      }
      pendingCashAmount = 0;
    }
    frame++;
    probeTick(frame);
    cameraTick(frame);
    if (traceViewportRecording &&
        frame - traceViewportStartFrame >= traceViewportFrames)
      stopTraceViewportRecording("frame-limit");
    if (wait == 0) {
      fprintf(stderr, "[host] o jogo pediu para encerrar.\n");
      if (traceViewportRecording) stopTraceViewportRecording("game-exit");
      break;
    }

    auto& frameBuffer = arrays[hScreen];
    const int hudScale = getPlayerStatusHudScale(frameBuffer.data());
    if (hudScale != 0) {
      uint32_t currentExperience = 0, requiredExperience = 0;
      if (getCharacterExperience(currentExperience, requiredExperience))
        drawHudExperienceText(frameBuffer.data(), currentExperience, requiredExperience);
    }
    const uint8_t* fb = frameBuffer.data();
    if (headlessFrames) {
      if (frame == 1 || frame % 10 == 0) { char nm[64]; snprintf(nm, sizeof nm, "/frame_%05ld.bmp", frame); writeBmp(outDir + nm, fb); }
      if (frame >= headlessFrames) break;
    } else {
      const auto renderStart = std::chrono::steady_clock::now();
      SDL_UpdateTexture(tex, nullptr, fb, SCREEN_W * 2);
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
        fprintf(stderr,
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
  if (headlessFrames) fprintf(stderr, "[host] %ld quadros executados. Imagens em %s\n", frame, outDir.c_str());
  if (!headlessFrames) {
    if (audioReady) {
#ifdef HAS_SDL_MIXER
      Mix_HaltMusic();
      for (auto& p : sfxCache) Mix_FreeChunk(p.second);
      for (auto& p : musCache) Mix_FreeMusic(p.second);
      Mix_CloseAudio();
      Mix_Quit();
#endif
    }
    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyTexture(tex); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
  }
  uc_close(uc);
  return 0;
}
