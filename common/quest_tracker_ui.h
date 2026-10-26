#pragma once
#include "overlay_font.h"
#include <string>
#include <cstdint>
#include <algorithm>

namespace destinia {

class QuestTrackerUI {
public:
  bool visible = true;
  bool minimized = false;
  std::string questTitle = "Caca: Soldados Imperiais";
  std::string questObjective = "Abates: 0 / 15";
  std::string destination = "Estrada Imperial";

  void setQuest(const std::string& title, const std::string& objective, const std::string& dest = "") {
    questTitle = title;
    questObjective = objective;
    destination = dest;
  }

  void toggle() {
    minimized = !minimized;
  }

  bool handleKey(int key, bool down) {
    if (!down) return false;
    if (key == 'q' || key == 'Q') {
      toggle();
      return true;
    }
    return false;
  }

  bool handleTouch(int x, int y, int type, int screenWidth, int screenHeight) {
    if (type != 0) return false;
    // Hitbox no canto superior direito
    int boxW = (screenWidth >= 800) ? 220 : 155;
    int boxH = minimized ? 20 : ((screenWidth >= 800) ? 52 : 40);
    int boxX = screenWidth - boxW - 8;
    int boxY = (screenWidth >= 800) ? 10 : 6;

    if (x >= boxX && x <= boxX + boxW && y >= boxY && y <= boxY + boxH) {
      toggle();
      return true;
    }
    return false;
  }

  void paint(uint8_t* pixels, int width, int height) {
    if (!visible || !pixels) return;

    int boxW = (width >= 800) ? 220 : 155;
    int boxH = minimized ? 20 : ((width >= 800) ? 52 : 40);
    int boxX = width - boxW - 8;
    int boxY = (width >= 800) ? 10 : 6;

    auto setPixel = [&](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
      if (x < 0 || x >= width || y < 0 || y >= height) return;
      size_t at = (size_t(y) * width + size_t(x)) * 2;
      uint16_t c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      pixels[at] = uint8_t(c & 255);
      pixels[at + 1] = uint8_t(c >> 8);
    };

    auto fillRect = [&](int rx, int ry, int rw, int rh, uint8_t r, uint8_t g, uint8_t b) {
      for (int dy = 0; dy < rh; ++dy) {
        for (int dx = 0; dx < rw; ++dx) {
          setPixel(rx + dx, ry + dy, r, g, b);
        }
      }
    };

    auto drawText = [&](int tx, int ty, const std::string& str, uint8_t r, uint8_t g, uint8_t b) {
      size_t at = 0;
      int curX = tx;
      while (at < str.size()) {
        char32_t cp = overlayCodepoint(str, at);
        auto glyph = overlayGlyph(cp);
        for (int col = 0; col < 5; ++col) {
          uint16_t mask = glyph[col];
          for (int row = 0; row < 12; ++row) {
            if (mask & (1 << row)) {
              setPixel(curX + col, ty + row, r, g, b);
            }
          }
        }
        curX += 6;
      }
    };

    // Fundo escuro translúcido com moldura estilo Gamevil (paleta do Destinia)
    fillRect(boxX, boxY, boxW, boxH, 18, 22, 28);

    // Borda superior e inferior
    for (int x = boxX; x < boxX + boxW; ++x) {
      setPixel(x, boxY, 210, 170, 40);      // Borda dourada superior
      setPixel(x, boxY + boxH - 1, 140, 110, 25); // Sombra dourada inferior
    }
    // Borda lateral esquerda e direita
    for (int y = boxY; y < boxY + boxH; ++y) {
      setPixel(boxX, y, 210, 170, 40);
      setPixel(boxX + boxW - 1, y, 140, 110, 25);
    }

    // Ícone de Pergaminho / Missão (10x10 estilizado Gamevil)
    int iconX = boxX + 5, iconY = boxY + 4;
    fillRect(iconX + 1, iconY, 8, 2, 220, 190, 80);
    fillRect(iconX, iconY + 2, 10, 6, 240, 220, 140);
    fillRect(iconX + 1, iconY + 8, 8, 2, 220, 190, 80);
    setPixel(iconX + 3, iconY + 4, 140, 90, 30);
    setPixel(iconX + 5, iconY + 4, 140, 90, 30);
    setPixel(iconX + 7, iconY + 4, 140, 90, 30);
    setPixel(iconX + 3, iconY + 6, 140, 90, 30);
    setPixel(iconX + 5, iconY + 6, 140, 90, 30);

    if (minimized) {
      drawText(boxX + 18, boxY + 4, "MISSAO [Q]", 240, 210, 50);
      return;
    }

    // Linha 1: Título da Missão (Amarelo Ouro)
    drawText(boxX + 18, boxY + 4, questTitle.substr(0, (width >= 800) ? 22 : 16), 240, 210, 50);

    // Linha 2: Objetivo da Missão (Branco Prateado)
    int line2Y = (width >= 800) ? (boxY + 20) : (boxY + 16);
    drawText(boxX + 18, line2Y, questObjective.substr(0, (width >= 800) ? 26 : 19), 230, 230, 240);

    // Linha 3 (somente em 800x480): Destino
    if (width >= 800 && !destination.empty()) {
      drawText(boxX + 18, boxY + 34, "Destino: " + destination.substr(0, 18), 120, 200, 255);
    }
  }
};

} // namespace destinia
