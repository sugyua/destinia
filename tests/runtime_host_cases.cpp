// Falhas artificiais pertencem apenas a este teste, nunca ao jogo distribuido.
#include <SDL.h>
#include <unicorn/unicorn.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>
static bool failAt(const char* step) {
  const char* value = std::getenv("DESTINIA_TEST_SDL_FAIL");
  return value && std::strcmp(value, step) == 0;
}
static SDL_Window* testWindow(const char* t, int x, int y, int w, int h, Uint32 f) {
  if (failAt("window")) { SDL_SetError("falha injetada window"); return nullptr; }
  return SDL_CreateWindow(t,x,y,w,h,f);
}
static SDL_Renderer* testRenderer(SDL_Window* w, int i, Uint32 f) {
  if (failAt("renderer")) { SDL_SetError("falha injetada renderer"); return nullptr; }
  return SDL_CreateRenderer(w,i,f);
}
static SDL_Texture* testTexture(SDL_Renderer* r, Uint32 f, int a, int w, int h) {
  if (failAt("texture")) { SDL_SetError("falha injetada texture"); return nullptr; }
  return SDL_CreateTexture(r,f,a,w,h);
}
static int testPoll(SDL_Event* e) {
  static bool quit = false;
  if (!quit) { quit=true; SDL_zero(*e); e->type=SDL_QUIT; return 1; }
  return 0;
}
static uc_err testEmulate(uc_engine* engine, uint64_t begin, uint64_t end, uint64_t timeout, size_t count) {
  // Offset confirmado apenas na biblioteca original usada pela fixture.
  if (std::getenv("DESTINIA_TEST_EMU_FAIL") && begin == 0x10007c5d) return UC_ERR_EXCEPTION;
  return uc_emu_start(engine,begin,end,timeout,count);
}
static bool ucFail(const char* operation) {
  const char* value=std::getenv("DESTINIA_TEST_UC_FAIL");
  return value && std::strcmp(value,operation)==0;
}
static uc_err testUcOpen(uc_arch arch, uc_mode mode, uc_engine** engine) {
  if(ucFail("open")) return UC_ERR_NOMEM;
  return uc_open(arch,mode,engine);
}
static uc_err testUcMap(uc_engine* engine, uint64_t address, size_t size, uint32_t permissions) {
  char label[40]; std::snprintf(label,sizeof label,"map:%llx",static_cast<unsigned long long>(address));
  if(ucFail(label)) return UC_ERR_MAP;
  return uc_mem_map(engine,address,size,permissions);
}
static uc_err testUcHook(uc_engine* engine, uc_hook* hook, int type, void* callback, void* user, uint64_t begin, uint64_t end, ...) {
  // Only core code hooks are injected; forward variadic instruction hooks unchanged below.
  if(type==UC_HOOK_CODE && ((begin==0x30000000 && ucFail("hook:stubs")) || (begin==0x30100000 && ucFail("hook:return")))) return UC_ERR_HOOK;
  if(type==UC_HOOK_INSN) { va_list args; va_start(args,end); int instruction=va_arg(args,int); va_end(args); return uc_hook_add(engine,hook,type,callback,user,begin,end,instruction); }
  return uc_hook_add(engine,hook,type,callback,user,begin,end);
}
static uc_err testUcClose(uc_engine* engine) {
  if(std::getenv("DESTINIA_TEST_UC_FAIL")) std::fprintf(stderr,"TEST_UC_CLOSED\n");
  return uc_close(engine);
}
#define uc_open testUcOpen
#define uc_mem_map testUcMap
#define uc_hook_add testUcHook
#define uc_close testUcClose
#define SDL_CreateWindow testWindow
#define SDL_CreateRenderer testRenderer
#define SDL_CreateTexture testTexture
#define SDL_PollEvent testPoll
#define uc_emu_start testEmulate
#define main destiniaHostMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_open
#undef uc_mem_map
#undef uc_hook_add
#undef uc_close
#undef uc_emu_start
#undef SDL_CreateWindow
#undef SDL_CreateRenderer
#undef SDL_CreateTexture
#undef SDL_PollEvent

int main(int argc, char** argv) {
  if(argc==2 && std::strcmp(argv[1],"test-stub-exception")==0) {
    if(uc_open(UC_ARCH_ARM,UC_MODE_ARM,&uc)!=UC_ERR_OK) return 10;
    handlers[0]=[] { throw std::runtime_error("falha de arquivo simulada"); };
    stubHook(uc,STUB_BASE,4,nullptr);
    bool stopped=fatal; uc_close(uc); return stopped?0:14;
  }
  if(argc==3 && std::strcmp(argv[1],"test-native-log")==0) {
    if(uc_open(UC_ARCH_ARM,UC_MODE_ARM,&uc)!=UC_ERR_OK) return 10;
    uc_mem_map(uc,HEAP_BASE,4096,UC_PROT_ALL); setupStubs();
    destinia::sessionLog.start(argv[2],"fixture");
    const char tag[]="jogo-original",message[]="mensagem nativa observada";
    wr(HEAP_BASE,tag,sizeof tag); wr(HEAP_BASE+128,message,sizeof message);
    W(0,6); W(1,HEAP_BASE); W(2,HEAP_BASE+128);
    handlers[importId.at("__android_log_print")]();
    destinia::sessionLog.close(); uc_close(uc); return 0;
  }
  if (argc == 2 && (std::strcmp(argv[1], "test-fread-zero") == 0 || std::strcmp(argv[1], "test-fwrite-zero") == 0)) {
    if (uc_open(UC_ARCH_ARM,UC_MODE_ARM,&uc) != UC_ERR_OK) return 10;
    setupStubs();
    FILE* file = std::tmpfile();
    if (!file) return 11;
    std::fputs("original",file); std::rewind(file);
    files[123] = file;
    W(0,0xdeadbeef); W(1,0); W(2,3); W(3,123);
    const char* name = std::strcmp(argv[1],"test-fread-zero")==0 ? "fread" : "fwrite";
    handlers[importId.at(name)]();
    bool valid = R(0)==0 && std::ftell(file)==0;
    std::fclose(file); files.clear(); uc_close(uc);
    if (!valid) std::fprintf(stderr,"Operacao de tamanho zero alterou posicao/retorno.\n");
    return valid ? 0 : 12;
  }
  return destiniaHostMain(argc,argv);
}
