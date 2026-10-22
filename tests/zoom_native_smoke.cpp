// Teste de integracao: usa a biblioteca ARM real com save temporario.
// Os wrappers abaixo pertencem apenas ao teste, nunca ao executavel distribuido.
#include <SDL.h>
#include <unicorn/unicorn.h>
#include <cstdio>
#include <cstdlib>
static uc_err keepEmulatorForTest(uc_engine*) { return UC_ERR_OK; }
static int testWindowWidth = 0, testWindowHeight = 0;
static int testTextureWidth = 0, testTextureHeight = 0;
static SDL_Window* checkedWindow(const char* title, int x, int y, int w, int h, Uint32 flags) {
  testWindowWidth = w; testWindowHeight = h;
  return SDL_CreateWindow(title, x, y, w, h, flags);
}
static SDL_Texture* checkedTexture(SDL_Renderer* renderer, Uint32 format, int access, int w, int h) {
  testTextureWidth = w; testTextureHeight = h;
  return SDL_CreateTexture(renderer, format, access, w, h);
}
static bool fullscreenTest() { return std::getenv("DESTINIA_TEST_FULLSCREEN") != nullptr; }
static int fullscreenCalls = 0;
static Uint32 lastFullscreenFlags = 0;
static bool correctUltrawideViewport = false;
static int checkedFullscreen(SDL_Window* window, Uint32 flags) {
  ++fullscreenCalls; lastFullscreenFlags = flags;
  return SDL_SetWindowFullscreen(window, flags);
}
static int checkedOutputSize(SDL_Renderer* renderer, int* width, int* height) {
  if (fullscreenTest()) { *width = 3440; *height = 1440; return 0; }
  return SDL_GetRendererOutputSize(renderer, width, height);
}
static int checkedWindowSize(SDL_Window* window, int* width, int* height) {
  if (fullscreenTest()) { *width = 3440; *height = 1440; return 0; }
  SDL_GetWindowSize(window, width, height); return 0;
}
static int checkedCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* src, const SDL_Rect* dst) {
  if (fullscreenTest() && dst) correctUltrawideViewport = dst->x == 520 && dst->y == 0 && dst->w == 2400 && dst->h == 1440;
  return SDL_RenderCopy(renderer, texture, src, dst);
}
static int controlledEvent(SDL_Event* event) {
  static int count = 0;
  ++count;
  SDL_zero(*event);
  if (fullscreenTest() && count <= 2) {
    event->type = SDL_KEYDOWN; event->key.keysym.sym = SDLK_F11; return 1;
  }
  const int step = count - (fullscreenTest() ? 2 : 0);
  if (step == 1 || step == 2) {
    event->type = step == 1 ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    event->button.button = SDL_BUTTON_LEFT;
    event->button.x = fullscreenTest() ? 1720 : 400; event->button.y = fullscreenTest() ? 720 : 240;
    return 1;
  }
  if (step == 3) { event->type = SDL_QUIT; return 1; }
  return 0;
}
#define SDL_SetWindowFullscreen checkedFullscreen
#define SDL_GetRendererOutputSize checkedOutputSize
#define SDL_GetWindowSize checkedWindowSize
#define SDL_RenderCopy checkedCopy
#define main destiniaMainForTest
#define uc_close keepEmulatorForTest
#define SDL_CreateWindow checkedWindow
#define SDL_CreateTexture checkedTexture
#define SDL_PollEvent controlledEvent
#include "../host_zoom/main.cpp"
#undef SDL_SetWindowFullscreen
#undef SDL_GetRendererOutputSize
#undef SDL_GetWindowSize
#undef SDL_RenderCopy
#undef main
#undef uc_close
#undef SDL_CreateWindow
#undef SDL_CreateTexture
#undef SDL_PollEvent

int main(int argc, char** argv) {
  int result = destiniaMainForTest(argc, argv);
  if (result) return result;
  if (testWindowWidth != 800 || testWindowHeight != 480 || testTextureWidth != 800 || testTextureHeight != 480) {
    fprintf(stderr, "FALHA: janela=%dx%d textura=%dx%d\n", testWindowWidth, testWindowHeight, testTextureWidth, testTextureHeight);
    return 1;
  }
  if (fullscreenTest() && (fullscreenCalls != 3 || lastFullscreenFlags != SDL_WINDOW_FULLSCREEN_DESKTOP || !correctUltrawideViewport)) {
    fprintf(stderr, "FALHA: tela cheia, alternancia F11 ou viewport ultrawide incorretos. Chamadas=%d\n", fullscreenCalls);
    return 1;
  }
  // Native API: all small-map axes are centered when the writer returns.
  const u32 camera = exportsMap.at("Game") + 18552;
  for (const auto dims : {std::pair<uint16_t,uint16_t>{30,30}, {60,60}, {20,10}}) {
    const uint16_t map[] = {dims.first, dims.second};
    wr(exportsMap.at("curMapData"), map, sizeof map);
    for (u32 position : {0u, 240u, 480u}) {
      if (!callFn(exportsMap.at("Map_setMapView"), {position, position})) return 1;
      int x = (int16_t)rd16(camera), y = (int16_t)rd16(camera + 2);
      int width = dims.first * 16, height = dims.second * 16;
      int expectedX = width < 800 ? (width - 800)/2 : std::clamp((int)position - 400, 0, width - 800);
      int expectedY = height < 480 ? (height - 480)/2 : std::clamp((int)position - 240, 0, height - 480);
      if (x != expectedX || y != expectedY) {
        fprintf(stderr, "FALHA: camera=(%d,%d), esperado=(%d,%d)\n", x,y,expectedX,expectedY); return 1;
      }
    }
  }
  // Execute each original STRH, then stop at its completion PC before another
  // ARM read can mask a missing writer callback. Covers inline movement/script/shake.
  struct Writer {u32 pc;uint16_t opcode;int source,base,index;char axis;};
  const Writer writers[] = {
{0x11b94,0x5338,0,7,4,'X'}, // strh r0, [r7, r4]; completion PC 0x11b96
{0x11bc6,0x52c1,1,0,3,'Y'}, // strh r1, [r0, r3]; completion PC 0x11bc8
{0x14e62,0x53de,6,3,7,'X'}, // strh r6, [r3, r7]; completion PC 0x14e64
{0x14eb6,0x53c8,0,1,7,'Y'}, // strh r0, [r1, r7]; completion PC 0x14eb8
{0x14ee2,0x5382,2,0,6,'X'}, // strh r2, [r0, r6]; completion PC 0x14ee4
{0x14f0a,0x5253,3,2,1,'Y'}, // strh r3, [r2, r1]; completion PC 0x14f0c
{0x14f78,0x53de,6,3,7,'X'}, // strh r6, [r3, r7]; completion PC 0x14f7a
{0x14f8c,0x53c8,0,1,7,'Y'}, // strh r0, [r1, r7]; completion PC 0x14f8e
{0x19664,0x5299,1,3,2,'X'}, // strh r1, [r3, r2]; completion PC 0x19666
{0x1966c,0x5299,1,3,2,'Y'}, // strh r1, [r3, r2]; completion PC 0x1966e
{0x19d30,0x52f2,2,6,3,'X'}, // strh r2, [r6, r3]; completion PC 0x19d32
{0x19d3a,0x52f2,2,6,3,'Y'}, // strh r2, [r6, r3]; completion PC 0x19d3c
{0x1e3a8,0x5262,2,4,1,'X'}, // strh r2, [r4, r1]; completion PC 0x1e3aa
{0x1e3aa,0x52e0,0,4,3,'Y'}, // strh r0, [r4, r3]; completion PC 0x1e3ac
{0x1f92c,0x5258,0,3,1,'X'}, // strh r0, [r3, r1]; completion PC 0x1f92e
{0x1f94a,0x5258,0,3,1,'Y'}, // strh r0, [r3, r1]; completion PC 0x1f94c
  };
  for(const auto dims : {std::pair<uint16_t,uint16_t>{30,30},{60,60},{20,10}}) {
    const uint16_t map[]={dims.first,dims.second};wr(exportsMap.at("curMapData"),map,sizeof map);
    for(const auto& writer:writers) {
      if(rd16(SO_BASE+writer.pc)!=writer.opcode) {fprintf(stderr,"FALHA: instrucao ARM de camera incompativel.\n");return 1;}
      const int16_t before[]={37,51};wr(camera,before,sizeof before);
      W(writer.source,123);W(writer.base,exportsMap.at("Game"));W(writer.index,writer.axis=='X'?0x4878:0x487a);
      uc_hook stop;
      auto stopped=[](uc_engine* engine,uint64_t,uint32_t,void*){uc_emu_stop(engine);};
      if(uc_hook_add(uc,&stop,UC_HOOK_CODE,reinterpret_cast<void*>(+stopped),nullptr,SO_BASE+writer.pc+2,SO_BASE+writer.pc+2)!=UC_ERR_OK)return 1;
      auto error=uc_emu_start(uc,(SO_BASE+writer.pc)|1u,SO_BASE+writer.pc+4,5000000,0);
      uc_hook_del(uc,stop);
      const int expectedX=dims.first*16<800?(int(dims.first)*16-800)/2:(writer.axis=='X'?123:37);
      const int expectedY=dims.second*16<480?(int(dims.second)*16-480)/2:(writer.axis=='Y'?123:51);
      if(error!=UC_ERR_OK || (int16_t)rd16(camera)!=expectedX || (int16_t)rd16(camera+2)!=expectedY) {
        fprintf(stderr,"FALHA: escritor ARM %x nao centralizou/preservou camera.\n",writer.pc);return 1;
      }
    }
  }
  const uint16_t map[] = {30,30}; wr(exportsMap.at("curMapData"), map, sizeof map);
  if (!callFn(exportsMap.at("Map_setMapView"), {240,240})) return 1;
  centerSmallMapCamera();
  u32 pixels = hmalloc(800*480*2), buffer = hmalloc(12);
  std::vector<uint16_t> scene(800*480, 0xF81F); wr(pixels,scene.data(),scene.size()*2);
  wr32(buffer,800); wr32(buffer+4,480); wr32(buffer+8,pixels); wr32(SO_BASE+0x4EF5C,buffer);
  W(0,0); mapEdgeClearHook(uc,0,0,nullptr);
  if (rd16(pixels) != MAP_BACKGROUND || rd16(pixels+159*2) != MAP_BACKGROUND ||
      rd16(pixels+160*2) != 0xF81F || rd16(pixels+639*2) != 0xF81F || rd16(pixels+640*2) != MAP_BACKGROUND) {
    fprintf(stderr,"FALHA: fundo cinza ou preservacao da area do mapa.\n"); return 1;
  }
  std::vector<uint8_t> hud(800*480*2,0);
  drawHudExperienceText(hud.data(),100,200);
  int first = 480, last = -1;
  for (int y=0;y<480;++y) for(int x=0;x<800;++x) if(read565(hud.data(),x,y)==0xFFBC) {
    first=std::min(first,y);last=std::max(last,y);
  }
  if(last-first+1 != 14) { fprintf(stderr,"FALHA: tamanho do texto XP alterado.\n"); return 1; }
  uc_close(uc);
  fprintf(stderr, "OK: janela 800x480, textura 800x480, camera ARM centralizada e fundo cinza; mapa preservado.\n");
  return 0;
}
