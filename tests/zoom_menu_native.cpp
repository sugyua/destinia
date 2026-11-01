#include <SDL.h>
#include <cstdio>
static int menuState();
static void setGuideHeld(bool);
static int frames = 0, sent = 0;
static bool sawMenu = false, sawOverlay = false, sawNotice = false, sawHeld = false, sawRelease = false, sawGuide = false, sawOptions = false;
static int checkedCopy(SDL_Renderer* r, SDL_Texture* t, const SDL_Rect* src, const SDL_Rect* dst) {
  ++frames;
  if (src && src->x == 200 && src->y == 120 && src->w == 400 && src->h == 240) sawMenu = true;
  if (sawMenu && sent == 4 && menuState() == 1 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawOverlay = true;
  if (sawMenu && sent == 7 && menuState() == 1 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawNotice = true;
  if (sawMenu && sent == 8 && frames >= 280 && src && src->x == 200 && src->w == 400) sawHeld = true;
  if (sawMenu && sent == 9 && src && src->x == 200 && src->w == 400) sawRelease = true;
  if (sawMenu && sent == 10 && menuState() == 1 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawGuide = true;
  if (sawMenu && sent >= 13 && menuState() == 4 && src && src->x == 0 && src->y == 0 && src->w == 800 && src->h == 480) sawOptions = true;
  return SDL_RenderCopy(r,t,src,dst);
}
static int controlledEvent(SDL_Event* e) {
  int threshold[] = {110,111,159,159,162,170,180,200,285,290,295,310,311,360};
  if (sent >= 14 || frames < threshold[sent]) return 0;
  SDL_zero(*e);
  if (sent == 13) { e->type = SDL_QUIT; ++sent; return 1; }
  if (sent == 2 || sent == 5) { e->type = SDL_KEYDOWN; e->key.keysym.sym = SDLK_F12; ++sent; return 1; }
  if (sent == 6) { e->type = SDL_KEYDOWN; e->key.keysym.sym = SDLK_h; ++sent; return 1; }
  if (sent == 9 || sent == 10) { setGuideHeld(sent == 9); ++sent; return 0; }
  e->type = (sent == 0 || sent == 3 || sent == 7 || sent == 11) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
  e->button.button = SDL_BUTTON_LEFT;
  e->button.x = (sent == 3 || sent == 4 || sent == 7 || sent == 8) ? 10 : 400;
  e->button.y = sent < 2 ? 240 : ((sent == 3 || sent == 4 || sent == 7 || sent == 8) ? 10 : 274);
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
static void setGuideHeld(bool held) { controllerBackHeld = held; }
int main(int argc, char** argv) {
  int result = destiniaMainForTest(argc,argv);
  if (result) return result;
  if (!sawMenu || !sawOverlay || !sawNotice || !sawHeld || !sawRelease || !sawGuide || !sawOptions) {
    std::fprintf(stderr,"menu=%d overlay=%d notice=%d held=%d release=%d guide=%d options=%d frames=%d events=%d\n",sawMenu,sawOverlay,sawNotice,sawHeld,sawRelease,sawGuide,sawOptions,frames,sent);
    return 1;
  }
  return 0;
}
