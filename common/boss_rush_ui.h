#pragma once
#include "overlay_font.h"
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstdint>

namespace destinia {

struct BossRushEntry {
  int species;
  const char* namePt;
  const char* nameEn;
  int goldCost;
  int area;
  int map;
};

inline const std::vector<BossRushEntry>& getBossRushList() {
  static const std::vector<BossRushEntry> list = {
    {100, "Guardiao da Masmorra", "Dungeon Guardian", 500, 1, 5},
    {101, "Defcan", "Defcan", 1000, 1, 16},
    {102, "Salas", "Salas", 2500, 2, 10},
    {103, "Defcan Desperto", "Awakened Defcan", 5000, 1, 21},
    {104, "Sileth", "Sileth", 7500, 1, 31},
    {105, "Berserk Tamur", "Berserk Tamur", 10000, 1, 41},
    {106, "Imperador Kaiser", "Emperor Kaiser", 15000, 2, 28},
    {107, "Baramos", "Baramos", 25000, 2, 16},
    {108, "Diablo", "Diablo", 35000, 1, 51},
    {109, "Guardiao de Lava", "Lava Guardian", 50000, 2, 48}
  };
  return list;
}

class BossRushUI {
public:
  bool open = false;
  int selectedIndex = 0;
  std::string statusMessage;
  
  std::function<int()> getGold;
  std::function<void(int)> setGold;
  std::function<void(int, int, int)> teleport; // area, map, entry

  void openMenu() {
    open = true;
    selectedIndex = 0;
    statusMessage.clear();
  }

  void closeMenu() {
    open = false;
    statusMessage.clear();
  }

  bool handleKey(int key, bool down) {
    if (!open) return false;
    if (!down) return true;

    // Cima: key 3
    if (key == 3) {
      selectedIndex = (selectedIndex - 1 + 10) % 10;
      statusMessage.clear();
      return true;
    }
    // Baixo: key 9
    if (key == 9) {
      selectedIndex = (selectedIndex + 1) % 10;
      statusMessage.clear();
      return true;
    }
    // Cancelar / Voltar: key 10 ou key 27
    if (key == 10 || key == 27) {
      closeMenu();
      return true;
    }
    // Ação / Confirmar: key 6 ou key 17
    if (key == 6 || key == 17) {
      const auto& list = getBossRushList();
      if (selectedIndex >= 0 && selectedIndex < int(list.size())) {
        const auto& boss = list[selectedIndex];
        int curGold = getGold ? getGold() : 999999;
        if (curGold < boss.goldCost) {
          statusMessage = "OURO INSUFICIENTE!";
        } else {
          if (setGold) setGold(curGold - boss.goldCost);
          if (teleport) teleport(boss.area, boss.map, 1);
          closeMenu();
        }
      }
      return true;
    }
    return true;
  }

  bool handleTouch(int x, int y, int type, int width, int height) {
    if (!open) return false;
    const int pw = std::min(340, width - 20), ph = std::min(220, height - 20);
    const int px = (width - pw) / 2, py = (height - ph) / 2;

    if (x < px || x >= px + pw || y < py || y >= py + ph) {
      if (type == 0) closeMenu();
      return true;
    }

    // Botão Fechar no canto superior direito
    if (x >= px + pw - 24 && y <= py + 20) {
      if (type == 0) closeMenu();
      return true;
    }

    // Linhas da lista
    int rowY = py + 34;
    const auto& list = getBossRushList();
    for (int i = 0; i < int(list.size()); i++) {
      if (y >= rowY - 2 && y < rowY + 13) {
        if (type == 0) {
          if (selectedIndex == i) {
            handleKey(17, true);
          } else {
            selectedIndex = i;
            statusMessage.clear();
          }
        }
        return true;
      }
      rowY += 15;
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height) const {
    if (!open || !pixels) return;

    const int pw = std::min(340, width - 20), ph = std::min(220, height - 20);
    const int px = (width - pw) / 2, py = (height - ph) / 2;

    // Fundo semitransparente escuro com moldura dourada
    for (int y = py; y < py + ph; ++y) {
      for (int x = px; x < px + pw; ++x) {
        const size_t at = (size_t(y) * width + x) * 2;
        uint16_t c = pixels[at] | (pixels[at + 1] << 8);
        bool border = (x == px || x == px + 1 || x == px + pw - 1 || x == px + pw - 2 ||
                       y == py || y == py + 1 || y == py + ph - 1 || y == py + ph - 2);
        if (border) {
          pixels[at] = 0xbf; pixels[at + 1] = 0xd7; // Dourado
        } else {
          uint16_t dark = (c >> 2) & 0x39E7; // 25% brilho
          pixels[at] = uint8_t(dark);
          pixels[at + 1] = uint8_t(dark >> 8);
        }
      }
    }

    // Título
    const std::string title = "MAGO DOS BOSSES -- BOSS RUSH";
    paintOverlayText(pixels, width, height, px + (pw - overlayTextWidth(title)) / 2 + 1, py + 8 + 1, title, 0);
    paintOverlayText(pixels, width, height, px + (pw - overlayTextWidth(title)) / 2, py + 8, title, 0xd7bf);

    // Botão [X]
    paintOverlayText(pixels, width, height, px + pw - 24, py + 8, "[X]", 0xf800);

    // Linha divisória
    for (int x = px + 8; x < px + pw - 8; ++x) {
      const size_t at = (size_t(py + 24) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    // Lista de Chefes
    const auto& list = getBossRushList();
    int rowY = py + 34;
    for (int i = 0; i < int(list.size()); i++) {
      const auto& b = list[i];
      const bool sel = (i == selectedIndex);

      if (sel) {
        for (int y = rowY - 1; y < rowY + 13; ++y) {
          for (int x = px + 8; x < px + pw - 8; ++x) {
            const size_t at = (size_t(y) * width + x) * 2;
            uint16_t c = pixels[at] | (pixels[at + 1] << 8);
            uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
            pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
          }
        }
      }

      std::string marker = sel ? "> " : "  ";
      std::string bossLine = marker + std::to_string(i + 1) + ". " + b.namePt;
      std::string costStr = std::to_string(b.goldCost) + " G";

      uint16_t colText = sel ? 0xffff : 0xce59;
      uint16_t colCost = sel ? 0x07e0 : 0xa510;

      paintOverlayText(pixels, width, height, px + 12 + 1, rowY + 1, bossLine, 0);
      paintOverlayText(pixels, width, height, px + 12, rowY, bossLine, colText);

      int costX = px + pw - 16 - overlayTextWidth(costStr);
      paintOverlayText(pixels, width, height, costX + 1, rowY + 1, costStr, 0);
      paintOverlayText(pixels, width, height, costX, rowY, costStr, colCost);

      rowY += 15;
    }

    // Rodapé
    for (int x = px + 8; x < px + pw - 8; ++x) {
      const size_t at = (size_t(py + ph - 24) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    int myGoldVal = getGold ? getGold() : 0;
    std::string footer = "SEU OURO: " + std::to_string(myGoldVal) + " G  [ENTER]: DESAFIAR  [ESC]: SAIR";
    if (!statusMessage.empty()) footer = statusMessage + " -- " + footer;

    paintOverlayText(pixels, width, height, px + 12 + 1, py + ph - 16 + 1, footer, 0);
    paintOverlayText(pixels, width, height, px + 12, py + ph - 16, footer, statusMessage.empty() ? 0xd7bf : 0xf800);
  }
};

} // namespace destinia
