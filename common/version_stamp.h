#pragma once

#include <cstdint>
#include <string>
#include "overlay_font.h"

#ifndef DESTINIA_VERSION
#define DESTINIA_VERSION "0.0.0"
#endif

namespace destinia {

inline void paintVersionStamp(uint8_t* pixels, int width, int height,
                              const std::string& version) {
  if (!pixels || width <= 0 || height <= 0 || version.empty()) return;

  const std::string label = "v" + version;
  const int x = width - overlayTextWidth(label) - 6;
  const int y = height - 13;
  paintOverlayText(pixels, width, height, x, y, label, 0x8410);
}

} // namespace destinia
