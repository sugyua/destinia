#pragma once

#include <cstdint>
#include <string>
#include "overlay_font.h"

#ifndef DESTINIA_VERSION
#define DESTINIA_VERSION "0.0.0"
#endif

namespace destinia {

inline void paintVersionStamp(uint8_t* pixels, int width, int height,
                              const std::string& version, int right = -1, int bottom = -1) {
  if (!pixels || width <= 0 || height <= 0 || version.empty()) return;

  const std::string label = "v" + version;
  const int x = (right < 0 ? width : right) - overlayTextWidth(label) - 6;
  const int y = (bottom < 0 ? height : bottom) - 13;
  paintOverlayText(pixels, width, height, x, y, label, 0x8410);
}

} // namespace destinia
