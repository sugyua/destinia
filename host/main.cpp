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
static const int SCREEN_W = 400, SCREEN_H = 240;
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
static u32 gMMain = 0, gMenu = 0, gGrp = 0;
static int autoFrame = -1, autoX = 0, autoY = 0;   // --autotap FRAME,X,Y
static int autoFrame2 = -1, autoX2 = 0, autoY2 = 0; // --autotap2 FRAME,X,Y
static u32 fnLogoPress = 0, fnMenuPress = 0, fnMenuRelease = 0, fnGamePress = 0, fnGameRelease = 0;
static u32 fnGetCash = 0, fnSetCash = 0;
static bool pendingBuyCash = false, audioReady = false, fastForward = false;
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
      std::string nm = arrText(R(3)); std::error_code ec; auto sz = fs::file_size(assetsDir + "/" + nm, ec);
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
          note("buyCash(" + std::to_string((int)a0) + ") -> cash local de " + std::to_string(pendingCashAmount) + "; callback nativo desativado para evitar crash");
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
static void writeBmp(const std::string& path, const uint8_t* fb565) {
  std::vector<uint8_t> rgba(SCREEN_W * SCREEN_H * 4); rgb565ToRgba(fb565, rgba.data());
  std::ofstream f(path, std::ios::binary);
  u32 rowBytes = SCREEN_W * 3, size = 54 + rowBytes * SCREEN_H, zero = 0, off = 54, hdr = 40, planesBpp = (24u << 16) | 1u;
  int w = SCREEN_W, h = SCREEN_H;
  f.write("BM", 2); f.write((char*)&size, 4); f.write((char*)&zero, 4); f.write((char*)&off, 4);
  f.write((char*)&hdr, 4); f.write((char*)&w, 4); f.write((char*)&h, 4); f.write((char*)&planesBpp, 4);
  for (int i = 0; i < 6; i++) f.write((char*)&zero, 4);
  for (int y = SCREEN_H - 1; y >= 0; y--)
    for (int x = 0; x < SCREEN_W; x++) { const uint8_t* p = &rgba[4 * (y * SCREEN_W + x)]; uint8_t bgr[3] = {p[2], p[1], p[0]}; f.write((char*)bgr, 3); }
}

static uint16_t read565(const uint8_t* pixels, int x, int y) {
  const size_t offset = 2 * (size_t)(y * SCREEN_W + x);
  return (uint16_t)pixels[offset] | ((uint16_t)pixels[offset + 1] << 8);
}

static bool hasPlayerStatusHud(const uint8_t* pixels) {
  int redPixels = 0, bluePixels = 0;
  for (int y = 8; y <= 18; ++y) {
    for (int x = 19; x <= 51; ++x) {
      const uint16_t color = read565(pixels, x, y);
      const int red = ((color >> 11) & 31) * 255 / 31;
      const int green = ((color >> 5) & 63) * 255 / 63;
      const int blue = (color & 31) * 255 / 31;
      if (y <= 12 && red > 90 && red * 10 > green * 13 && red * 10 > blue * 12)
        ++redPixels;
      if (y >= 13 && blue > 90 && blue * 10 > red * 12 && blue * 10 > green * 9)
        ++bluePixels;
    }
  }
  return redPixels >= 4 && bluePixels >= 4;
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

  const int scale = 2;
  const int textWidth = (int)strlen(text) * 6 * scale - scale;
  const int textHeight = 7 * scale;
  const int left = std::max(2, SCREEN_W - textWidth - 6);
  const int top = SCREEN_H - 22;
  const uint16_t panel = (uint16_t)((2u << 11) | (5u << 5) | 7u);
  const uint16_t border = (uint16_t)((8u << 11) | (40u << 5) | 12u);
  const uint16_t ink = (uint16_t)((31u << 11) | (61u << 5) | 28u);
  for (int y = top - 2; y < top + textHeight + 2; ++y) {
    for (int x = left - 2; x < left + textWidth + 2; ++x) {
      if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H - 2) continue;
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

static void drawEnlargedStatusHud(uint8_t* pixels) {
  constexpr int sourceWidth = 52;
  constexpr int sourceHeight = 21;
  constexpr int scale = 2;
  uint16_t source[sourceWidth * sourceHeight];
  for (int y = 0; y < sourceHeight; ++y)
    for (int x = 0; x < sourceWidth; ++x)
      source[y * sourceWidth + x] = read565(pixels, x, y);

  for (int y = 0; y < sourceHeight * scale; ++y) {
    for (int x = 0; x < sourceWidth * scale; ++x) {
      const uint16_t under = read565(pixels, x, y);
      const uint16_t dark = (uint16_t)((((under >> 11) & 31) / 3u << 11) |
                                      (((under >> 5) & 63) / 3u << 5) |
                                      ((under & 31) / 3u));
      write565(pixels, x, y, dark);
    }
  }

  for (int y = 0; y < sourceHeight; ++y) {
    for (int x = 0; x < sourceWidth; ++x) {
      const uint16_t color = source[y * sourceWidth + x];
      const int red = ((color >> 11) & 31) * 255 / 31;
      const int green = ((color >> 5) & 63) * 255 / 63;
      const int blue = (color & 31) * 255 / 31;
      const bool warmIconPixel = x < 21 && y < 21 && red > 70 &&
                                 red * 10 > green * 12 && red * 10 > blue * 13;
      const bool brightText = red > 160 && green > 160 && blue > 160;
      const bool redBar = red > 90 && red * 10 > green * 13 && red * 10 > blue * 12;
      const bool blueBar = blue > 90 && blue * 10 > red * 12 && blue * 10 > green * 9;
      const bool yellowText = red > 130 && green > 100 && blue < 120 && red > green;
      if (!(warmIconPixel || brightText || redBar || blueBar || yellowText)) continue;
      const int destX = x * scale;
      const int destY = y * scale;
      for (int dy = 0; dy < scale; ++dy)
        for (int dx = 0; dx < scale; ++dx)
          write565(pixels, destX + dx, destY + dy, color);
    }
  }
}

static bool getCharacterExperience(uint32_t& current, uint32_t& required) {
  auto game = exportsMap.find("Game");
  if (game == exportsMap.end()) return false;

  // GameCanvas_gainExpProc stores the current level at +0x48FA and current-level XP at +0x48FC.
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
  int scale = 3, headlessFrames = 0; std::string outDir = "frames";
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    auto next = [&]() { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
    if (a == "--game") gameDir = next(); else if (a == "--so") soPath = next(); else if (a == "--save") saveDir = next();
    else if (a == "--scale") scale = atoi(next().c_str()); else if (a == "--headless") headlessFrames = atoi(next().c_str());
    else if (a == "--out") outDir = next(); else if (a == "-v") verbose = true;
    else if (a == "--probe") probe = true;
    else if (a == "--mute") muteAudio = true;
    else if (a == "--profile") profile = true;
    else if (a == "--autotap") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame, &autoX, &autoY) != 3) { printf("--autotap precisa de QUADRO,X,Y\n"); return 0; } }
    else if (a == "--autotap2") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame2, &autoX2, &autoY2) != 3) { printf("--autotap2 precisa de QUADRO,X,Y\n"); return 0; } }
    else { printf("Uso: destinia_host [--game PASTA] [--so ARQUIVO] [--save PASTA] [--scale N] [--headless QUADROS --out PASTA] [-v] [--probe] [--mute] [--profile] [--autotap QUADRO,X,Y] [--autotap2 QUADRO,X,Y]\n"); return 0; }
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

  for (const char* n : {"Java_game_destiniaeng_GameThread_initGame", "Java_game_destiniaeng_GameThread_jniRun", "Java_game_destiniaeng_GameThread_jniTouch"})
    if (!exportsMap.count(n)) { fprintf(stderr, "Simbolo nao encontrado: %s\n", n); return 1; }
  u32 fnInit = exportsMap["Java_game_destiniaeng_GameThread_initGame"];
  u32 fnRun = exportsMap["Java_game_destiniaeng_GameThread_jniRun"];
  u32 fnTouch = exportsMap["Java_game_destiniaeng_GameThread_jniTouch"];
  auto grab = [&](const char* n) { auto it = exportsMap.find(n); return it == exportsMap.end() ? 0u : it->second; };
  fnLogoPress = grab("LogoCanvas_mKeyPressed");
  fnMenuPress = grab("MenuCanvas_mKeyPressed"); fnMenuRelease = grab("MenuCanvas_mKeyReleased");
  fnGamePress = grab("GameCanvas_mKeyPressed"); fnGameRelease = grab("GameCanvas_mKeyReleased");
  fnGetCash = grab("getMyCash"); fnSetCash = grab("setMyCash");

  // [OK] initGame(env,obj, byte[] deviceID,int len, byte[] modelo,int len, w, h, isKorean)
  // isKorean = 0 -> a .so escolhe os arquivos *_en (ingles).
  std::string devId = "DESTINIA-PC-0001", model = "PC";
  u32 hId = nextArr++, hModel = nextArr++, hScreen = nextArr++;
  arrays[hId].assign(devId.begin(), devId.end());
  arrays[hModel].assign(model.begin(), model.end());
  arrays[hScreen].assign(FB_BYTES, 0);
  if (!callFn(fnInit, {ENV_OBJ, 0x10, hId, (u32)devId.size(), hModel, (u32)model.size(), SCREEN_W, SCREEN_H, 0})) return 2;
  fprintf(stderr, "[host] initGame ok. Rodando o jogo...\n");

  // [OK] jniTouch(env,obj, x, y, tipo): tipo 0=toque 1=solta 2=move 3=tecla de menu (Thumb decodificado)
  auto touch = [&](int x, int y, int type) {
    bool ok = callFn(fnTouch, {ENV_OBJ, 0x10, (u32)x, (u32)y, (u32)type});
    if (probe && type != 2) { fprintf(stderr, "[sonda] jniTouch(x=%d, y=%d, tipo=%d) -> %s\n", x, y, type, ok ? "ok" : "FALHOU"); probeState(type == 0 ? "logo apos PRESSIONAR" : type == 1 ? "logo apos SOLTAR" : "logo apos tecla"); }
  };

  SDL_Window* win = nullptr; SDL_Renderer* ren = nullptr; SDL_Texture* tex = nullptr; SDL_GameController* controller = nullptr;
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
    win = SDL_CreateWindow("Destinia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 720, SDL_WINDOW_RESIZABLE);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    for (int i = 0; i < SDL_NumJoysticks(); i++) { if (SDL_IsGameController(i)) { controller = SDL_GameControllerOpen(i); if (controller) { fprintf(stderr, "[host] Controle conectado: %s\n", SDL_GameControllerName(controller)); break; } } }
    fprintf(stderr, "[host] Teclado: WASD/setas mover, Enter/Z confirmar, Esc/X voltar, I menu, F5 save. DualSense: analogico mover, X atacar, O voltar, triangulo menu, Share save, L1 acelerar.\n");
  } else fs::create_directories(outDir);

  bool running = true, mouseDown = false; long frame = 0;
  // --profile: acumuladores de tempo por quadro (emulacao, present, sleep) para achar o gargalo de CPU.
  double profEmulMs = 0, profPresentMs = 0, profSleepMs = 0, profWallMs = 0, profMaxEmulMs = 0;
  long profFrames = 0, profOverBudget = 0, profWait1000 = 0; double profWaitSum = 0;
  while (running) {
    auto t0 = std::chrono::steady_clock::now();
    if (!headlessFrames) {
      SDL_Event ev;
      while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) running = false;
        else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEMOTION) {
          int wx = ev.type == SDL_MOUSEMOTION ? ev.motion.x : ev.button.x, wy = ev.type == SDL_MOUSEMOTION ? ev.motion.y : ev.button.y;
          int ow = 0, oh = 0; SDL_GetRendererOutputSize(ren, &ow, &oh); if (ow <= 0) ow = 1; if (oh <= 0) oh = 1;
          
          int x = std::clamp((wx * SCREEN_W) / ow, 0, SCREEN_W - 1);
          int y = std::clamp((wy * SCREEN_H) / oh, 0, SCREEN_H - 1);
          if (probe && ev.type != SDL_MOUSEMOTION) {
            int ww = 0, wh = 0, lw = 0, lh = 0, ow = 0, oh = 0; SDL_GetWindowSize(win, &ww, &wh); SDL_RenderGetLogicalSize(ren, &lw, &lh); SDL_GetRendererOutputSize(ren, &ow, &oh);
            fprintf(stderr, "[sonda] SDL %s botao=%d janela_px=(%d,%d) tamanho_janela=%dx%d saida_render=%dx%d tamanho_logico=%dx%d -> jogo=(%d,%d)\n",
                    ev.type == SDL_MOUSEBUTTONDOWN ? "BAIXO" : "CIMA", (int)ev.button.button, wx, wy, ww, wh, ow, oh, lw, lh, x, y);
          }
          if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) { mouseDown = true; touch(x, y, 0); }
          else if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT) { mouseDown = false; touch(x, y, 1); }
          else if (ev.type == SDL_MOUSEMOTION && mouseDown) touch(x, y, 2);
        } else if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
          if (ev.key.repeat) continue;
          bool down = ev.type == SDL_KEYDOWN;
          switch (ev.key.keysym.sym) {
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
    auto tEmul0 = std::chrono::steady_clock::now();
    if (!callFn(fnRun, {ENV_OBJ, 0x10, hScreen, (u32)frame}, &wait)) break;
    double emulMs = (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tEmul0).count() / 1000.0;
    if (pendingBuyCash) {
      pendingBuyCash = false;
      if (fnGetCash && fnSetCash) {
        u32 cash = 0;
        if (!callFn(fnGetCash, {}, &cash)) break;
        u32 next = cash >= 170000 || pendingCashAmount >= 170000 - cash
                       ? std::max<u32>(cash, 170000)
                       : cash + pendingCashAmount;
        if (!callFn(fnSetCash, {next})) break;
        fprintf(stderr, "[host] cash local %u -> %u; resposta da loja nao simulada (spinner pode permanecer).\n", cash, next);
      } else {
        fprintf(stderr, "[host] erro: getMyCash/setMyCash nao exportadas; cash nao foi alterado.\n");
      }
      pendingCashAmount = 0;
    }
    frame++;
    probeTick(frame);
    if (wait == 0) { fprintf(stderr, "[host] o jogo pediu para encerrar.\n"); break; }

    auto& frameBuffer = arrays[hScreen];
    if (hasPlayerStatusHud(frameBuffer.data())) {
      drawEnlargedStatusHud(frameBuffer.data());
      uint32_t currentExperience = 0, requiredExperience = 0;
      if (getCharacterExperience(currentExperience, requiredExperience))
        drawHudExperienceText(frameBuffer.data(), currentExperience, requiredExperience);
    }
    const uint8_t* fb = frameBuffer.data();
    if (headlessFrames) {
      if (frame == 1 || frame % 10 == 0) { char nm[64]; snprintf(nm, sizeof nm, "/frame_%05ld.bmp", frame); writeBmp(outDir + nm, fb); }
      if (frame >= headlessFrames) break;
    } else {
      auto tPres0 = std::chrono::steady_clock::now();
      SDL_UpdateTexture(tex, nullptr, fb, SCREEN_W * 2);
      int ow = 0, oh = 0; SDL_GetRendererOutputSize(ren, &ow, &oh); SDL_Rect dst = {0, 0, ow, oh}; SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, nullptr, &dst); SDL_RenderPresent(ren);
      profPresentMs += (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tPres0).count() / 1000.0;
    }
    // [?] Java: se jniRun devolve 1000 (houve carga de arquivo) faz GC e dorme 100 ms.
    // Ritmo do quadro: honra o 'wait' que o PROPRIO JOGO pediu. O Destinia e de passo fixo
    // (~16 FPS); respeitar o 'wait' mantem a velocidade original e evita CPU 100% / aquecimento.
    long elapsed = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    double budgetMs = wait >= 1000 ? 100.0 : (double)wait;
    long sleepMs = (long)std::max(0.0, budgetMs - (double)elapsed);
    if (fastForward) {
      sleepMs = 0;
    } else if (sleepMs > 0 && !headlessFrames) {
      SDL_Delay((Uint32)sleepMs);
    }
    if (profile) {
      double wallMs = (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count() / 1000.0;
      profEmulMs += emulMs; profSleepMs += sleepMs; profWallMs += wallMs; profWaitSum += wait; profFrames++;
      if (emulMs > profMaxEmulMs) profMaxEmulMs = emulMs;
      if (wait < 1000 && emulMs > budgetMs) profOverBudget++;
      if (wait >= 1000) profWait1000++;
      if (profFrames % 120 == 0) {
        double n = profFrames;
        fprintf(stderr, "[perfil] %ld quadros | emul=%.2fms (max %.2fms) presente=%.2fms sleep=%.2fms wall=%.2fms | wait medio=%.1f | FPS efetivo=%.1f | %.0f%% quadros emul acima do budget | wait=1000 em %.0f%%\n",
                profFrames, profEmulMs / n, profMaxEmulMs, profPresentMs / n, profSleepMs / n, profWallMs / n,
                profWaitSum / n, profWallMs > 0 ? 1000.0 / (profWallMs / n) : 0.0,
                100.0 * profOverBudget / n, 100.0 * profWait1000 / n);
        profEmulMs = profPresentMs = profSleepMs = profWallMs = profWaitSum = profMaxEmulMs = 0; profFrames = profOverBudget = profWait1000 = 0;
      }
    }
  }
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
