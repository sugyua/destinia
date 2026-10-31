#include <SDL.h>
#include <cstdio>
static int menuState();
static int frames = 0, sent = 0;
static bool sawMenu = false, sawOverlay = false, sawOptions = false;
static int checkedCopy(SDL_Renderer* r, SDL_Texture* t, const SDL_Rect* src, const SDL_Rect* dst) {
  ++frames;
  if (src && src->x == 200 && src->y == 120 && src->w == 400 && src->h == 240) sawMenu = true;
  if (sawMenu && sent >= 3 && menuState() == 1 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawOverlay = true;
  if (sawMenu && sent >= 6 && menuState() == 4 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawOptions = true;
  return SDL_RenderCopy(r,t,src,dst);
}
static int controlledEvent(SDL_Event* e) {
  int threshold[] = {110,111,160,170,200,201,250};
  if (sent >= 7 || frames < threshold[sent]) return 0;
  SDL_zero(*e);
  if (sent == 6) { e->type = SDL_QUIT; ++sent; return 1; }
  if (sent == 2 || sent == 3) { e->type = SDL_KEYDOWN; e->key.keysym.sym = SDLK_F12; ++sent; return 1; }
  e->type = sent % 2 == 0 ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
  e->button.button = SDL_BUTTON_LEFT;
  e->button.x = 400; e->button.y = sent < 2 ? 240 : 274;
  ++sent; return 1;
}
#define SDL_RenderCopy checkedCopy
#define SDL_PollEvent controlledEvent
#undef main
#define main destiniaMainForTest
#include "../host_zoom/main.cpp"
#undef main
#undef SDL_PollEvent
#undef SDL_RenderCopy
int main(int argc, char** argv) {
  int result = destiniaMainForTest(argc,argv);
  if (result) return result;
  if (!sawMenu || !sawOverlay || !sawOptions) {
    std::fprintf(stderr,"menu=%d overlay=%d options=%d frames=%d events=%d\n",sawMenu,sawOverlay,sawOptions,frames,sent);
    return 1;
  }
  return 0;
}
