#pragma once

#include "combat_power.h"
#include "overlay_font.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace destinia {

struct AttributeRow { std::string label; std::string value; };

inline std::string tenthsPercent(int value) {
  const int64_t n = value;
  const auto absolute = n < 0 ? -n : n;
  return std::string(n < 0 ? "-" : "") + std::to_string(absolute / 10) + "." + std::to_string(absolute % 10) + "%";
}

inline std::array<AttributeRow, 8> secondaryAttributeRows(const CombatStats& stats) {
  return {{{"PV MAX", std::to_string(stats[5])}, {"PM MAX", std::to_string(stats[6])},
           {"ATK", std::to_string(stats[7])}, {"DEF", std::to_string(stats[8])},
           {"HIT", tenthsPercent(stats[9])}, {"EVA", tenthsPercent(stats[10])},
           {"CRI", tenthsPercent(stats[11])}, {"BLD", tenthsPercent(stats[12])}}};
}

class AttributeSheet {
 public:
  bool visible() const { return shown; }
  void toggle() { shown = !shown; }

  void paint(uint8_t* pixels, int width, int height, const CombatStats& stats) const {
    if (!shown || !pixels || width < 220 || height < 110) return;
    const int panelW = std::min(300, width - 12), panelH = 112;
    const int x = (width - panelW) / 2, y = (height - panelH) / 2;
    if(!paintOriginalPanel(pixels,width,height,x,y,panelW,panelH)) {
    fill(pixels, width, height, x, y, panelW, panelH, 0x0841);
    border(pixels, width, height, x, y, panelW, panelH, 0xe5b2);
    }
    paintOverlayText(pixels, width, height, x + 10, y + 8, "ATRIBUTOS SECUNDARIOS", 0xffc0);
    const auto rows = secondaryAttributeRows(stats);
    for (size_t i = 0; i < rows.size(); ++i) {
      const int col = int(i / 4), row = int(i % 4);
      const int px = x + 12 + col * (panelW / 2);
      const int py = y + 28 + row * 15;
      paintOverlayText(pixels, width, height, px, py, rows[i].label, 0x7bef);
      paintOverlayText(pixels, width, height, px + 56, py, rows[i].value, 0xffff);
    }
    paintOverlayText(pixels, width, height, x + 10, y + panelH - 12, "VALORES DO CALCULO NATIVO ATIVO", 0x7bef);
  }

 private:
  bool shown = false;

  static void put(uint8_t* pixels, int width, int height, int px, int py, uint16_t color) {
    if (px < 0 || py < 0 || px >= width || py >= height) return;
    const size_t at = (static_cast<size_t>(py) * width + px) * 2;
    pixels[at] = uint8_t(color & 0xff);
    pixels[at + 1] = uint8_t(color >> 8);
  }
  static void fill(uint8_t* pixels, int width, int height, int x, int y, int w, int h, uint16_t color) {
    for (int py = y; py < y + h; ++py)
      for (int px = x; px < x + w; ++px) put(pixels, width, height, px, py, color);
  }
  static void border(uint8_t* pixels, int width, int height, int x, int y, int w, int h, uint16_t color) {
    for (int px = x; px < x + w; ++px) { put(pixels, width, height, px, y, color); put(pixels, width, height, px, y + h - 1, color); }
    for (int py = y; py < y + h; ++py) { put(pixels, width, height, x, py, color); put(pixels, width, height, x + w - 1, py, color); }
  }
};

}  // namespace destinia
