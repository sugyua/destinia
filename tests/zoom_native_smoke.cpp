// Teste de integracao: usa a biblioteca ARM real com save temporario.
// Os wrappers abaixo pertencem apenas ao teste, nunca ao executavel distribuido.
#include <SDL.h>
#include <unicorn/unicorn.h>
#include <cstdio>
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
static int controlledEvent(SDL_Event* event) {
  static int count = 0;
  ++count;
  SDL_zero(*event);
  if (count == 1 || count == 2) {
    event->type = count == 1 ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    event->button.button = SDL_BUTTON_LEFT;
    event->button.x = 400; event->button.y = 240;
    return 1;
  }
  if (count == 3) { event->type = SDL_QUIT; return 1; }
  return 0;
}
#define main destiniaMainForTest
#define uc_close keepEmulatorForTest
#define SDL_CreateWindow checkedWindow
#define SDL_CreateTexture checkedTexture
#define SDL_PollEvent controlledEvent
#include "../host_zoom/main.cpp"
#undef main
#undef uc_close
#undef SDL_CreateWindow
#undef SDL_CreateTexture
#undef SDL_PollEvent

int main(int argc, char** argv) {
  int result = destiniaMainForTest(argc, argv);
  if (result) return result;
  if (testWindowWidth != 800 || testWindowHeight != 480 || testTextureWidth != 400 || testTextureHeight != 240) {
    fprintf(stderr, "FALHA: janela=%dx%d textura=%dx%d\n", testWindowWidth, testWindowHeight, testTextureWidth, testTextureHeight);
    return 1;
  }
  // Testa a rotina original com o tamanho de mapa observado no relatorio.
  const uint16_t dims[] = {30, 30};
  wr(exportsMap.at("curMapData"), dims, sizeof dims);
  for (u32 position : {0u, 240u, 480u}) {
    if (!callFn(exportsMap.at("Map_setMapView"), {position, position})) return 1;
    int x = (int16_t)rd16(exportsMap.at("Game") + 18552);
    int y = (int16_t)rd16(exportsMap.at("Game") + 18554);
    int expectedX = std::clamp((int)position - 200, 0, 80);
    int expectedY = std::clamp((int)position - 120, 0, 240);
    if (x != expectedX || y != expectedY) {
      fprintf(stderr, "FALHA: camera=(%d,%d), esperado=(%d,%d)\n", x, y, expectedX, expectedY);
      return 1;
    }
  }
  uc_close(uc);
  fprintf(stderr, "OK: janela 800x480, textura 400x240 e camera ARM no mapa 480x480.\n");
  return 0;
}
