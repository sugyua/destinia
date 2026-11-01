#include <initializer_list>
#include "../common/zoom_menu_view.h"
int main() {
  const auto menu = destinia::zoomMenuSource(1);
  if (menu.x != 200 || menu.y != 120 || menu.w != 400 || menu.h != 240) return 1;
  const auto overlay = destinia::zoomMenuSource(1, true);
  if (overlay.x != 0 || overlay.y != 0 || overlay.w != 800 || overlay.h != 480) return 4;
  for (int state : {0,2,4}) {
    const auto game = destinia::zoomMenuSource(state);
    if (game.x != 0 || game.y != 0 || game.w != 800 || game.h != 480) return 2;
  }
  if (destinia::zoomMapCoordinate(0, 1600, 200, 400) != 200 ||
      destinia::zoomMapCoordinate(800, 1600, 200, 400) != 400 ||
      destinia::zoomMapCoordinate(1599, 1600, 200, 400) != 599 ||
      destinia::zoomMapCoordinate(-1, 1600, 200, 400) != 200 ||
      destinia::zoomMapCoordinate(1600, 1600, 200, 400) != 599) return 3;
  return 0;
}

