#pragma once
#include <algorithm>
#include <cstdint>

namespace destinia {
struct ZoomSource { int x, y, w, h; };

inline ZoomSource zoomMenuSource(int menuState, bool overlayVisible = false) {
  return menuState == 1 && !overlayVisible ? ZoomSource{200, 120, 400, 240}
                        : ZoomSource{0, 0, 800, 480};
}
inline int zoomMapCoordinate(int position, int viewportSize, int sourceStart, int sourceSize) {
  if (viewportSize <= 0 || sourceSize <= 0) return sourceStart;
  return sourceStart + std::clamp(int(int64_t(position) * sourceSize / viewportSize), 0, sourceSize - 1);
}
}
