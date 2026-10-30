#pragma once
#include "shared_stash.h"
#include "ui_transform.h"
#include "overlay_font.h"
#include "original_ui.h"
#include <algorithm>
#include <string>
#include <vector>

namespace destinia {

class SharedStashUI {
public:
  size_t page = 0;             // 0: itens 0..23, 1: itens 24..47
  int selectedIndex = 0;      // 0..23 na página atual
  static constexpr size_t PAGE_SIZE = 24;

  inline CatalogRect panelRect(int width, int height) const {
    return { (width - 380) / 2, (height - 240) / 2 + 10, 380, 226 };
  }

  inline CatalogRect closeButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + p.w - 24, p.y + 6, 18, 16 };
  }

  inline CatalogRect tabButton(int pageIdx, int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 12 + pageIdx * 90, p.y + 24, 84, 18 };
  }

  inline CatalogRect itemSlotRect(int slotIdx, int width, int height) const {
    auto p = panelRect(width, height);
    // Grid 6 colunas x 4 linhas = 24 slots
    int col = slotIdx % 6;
    int row = slotIdx / 6;
    int startX = p.x + 14;
    int startY = p.y + 48;
    return { startX + col * 42, startY + row * 34, 38, 30 };
  }

  inline CatalogRect actionButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 270, p.y + 192, 96, 22 };
  }

  void nextSlot() {
    selectedIndex = (selectedIndex + 1) % PAGE_SIZE;
  }

  void prevSlot() {
    selectedIndex = (selectedIndex + PAGE_SIZE - 1) % PAGE_SIZE;
  }

  void nextPage() {
    page = (page == 0) ? 1 : 0;
  }

  size_t getGlobalIndex() const {
    return page * PAGE_SIZE + selectedIndex;
  }

  bool handleTouch(int x, int y, int width, int height, size_t totalItems) {
    if (closeButton(width, height).contains(x, y)) {
      return false; // Fecha o baú
    }
    for (int p = 0; p < 2; ++p) {
      if (tabButton(p, width, height).contains(x, y)) {
        page = p;
        return true;
      }
    }
    for (int i = 0; i < 24; ++i) {
      if (itemSlotRect(i, width, height).contains(x, y)) {
        selectedIndex = i;
        return true;
      }
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height, const SharedStash& stash, int uiPercent = 100) const {
    const auto p = panelRect(width, height);
    const auto transform = UiTransform{width, height, uiPercent, UiAnchor::Center, p.w, p.h, p.x, p.y};

    auto rawPixel = [&](int x, int y, uint16_t c) {
      if (x >= 0 && x < width && y >= 0 && y < height) {
        pixels[(y * width + x) * 2] = c & 255;
        pixels[(y * width + x) * 2 + 1] = c >> 8;
      }
    };

    auto pixel = [&](int x, int y, uint16_t c) {
      auto r = transform.rect(x, y, 1, 1);
      for (int yy = std::max(0, r.y); yy < std::min(height, r.y + r.h); ++yy) {
        for (int xx = std::max(0, r.x); xx < std::min(width, r.x + r.w); ++xx) {
          rawPixel(xx, yy, c);
        }
      }
    };

    auto rect = [&](int rx, int ry, int rw, int rh, uint16_t c) {
      for (int dy = 0; dy < rh; ++dy) {
        for (int dx = 0; dx < rw; ++dx) {
          pixel(rx + dx, ry + dy, c);
        }
      }
    };

    std::vector<UiRect> originalRects;
    auto panel = [&](int x, int y, int w, int h, uint16_t c) {
      auto r = transform.rect(x, y, w, h);
      if (paintOriginalPanel(pixels, width, height, r.x, r.y, r.w, r.h)) {
        originalRects.push_back({x,y,w,h});
        return;
      }
      rect(x,y,w,h,c);
    };

    auto frame = [&](int rx, int ry, int rw, int rh, uint16_t c) {
      for (const auto& r : originalRects) {
        if ((rx == r.x && ry == r.y && rw == r.w && rh == r.h) ||
            (rx == r.x+1 && ry == r.y+1 && rw == r.w-2 && rh == r.h-2)) return;
      }
      auto r = transform.rect(rx, ry, rw, rh);
      if (paintOriginalPanel(pixels,width,height,r.x,r.y,r.w,r.h)) return;

      for (int x = 0; x < rw; ++x) { pixel(rx + x, ry, c); pixel(rx + x, ry + rh - 1, c); }
      for (int y = 0; y < rh; ++y) { pixel(rx, ry + y, c); pixel(rx + rw - 1, ry + y, c); }
    };

    auto text = [&](int x, int y, const std::string& value, uint16_t color, int scale = 1) {
      auto point = transform.point(x,y);
      if (paintOriginalText(pixels,width,height,point.first,point.second,value,color,0,
                            std::max(1,int(std::lround(scale*transform.scale()))))) return;

      for (size_t at = 0; at < value.size(); x += 6 * scale) {
        auto bits = overlayGlyph(overlayCodepoint(value, at));
        for (int i = 0; i < 5; ++i) {
          for (int row = 0; row < 11; ++row) {
            if (bits[i] & (1 << row)) {
              rect(x + i * scale, y + (row - 2) * scale, scale, scale, color);
            }
          }
        }
      }
    };

    uint16_t panelBg = 0x10a2;   // Fundo escuro azulado
    uint16_t borderCol = 0x2124; // Borda sutil
    uint16_t gold = 0xfe60;      // Dourado Destinia
    uint16_t tabActive = 0x18e5;
    uint16_t bgDark = 0x0841;
    uint16_t white = 0xffff;
    uint16_t green = 0x07e0;
    uint16_t muted = 0x7bef;

    static const uint16_t rankColors[5] = {
      0xffff, // 0: Comum (Branco)
      0x07e0, // 1: Mágico (Verde)
      0x347f, // 2: Raro (Azul)
      0xffe0, // 3: Épico (Amarelo)
      0xfb40  // 4: Lendário (Laranja)
    };

    // Fundo Principal do Baú
    panel(p.x, p.y, p.w, p.h, panelBg);
    frame(p.x, p.y, p.w, p.h, borderCol);
    frame(p.x + 1, p.y + 1, p.w - 2, p.h - 2, gold);

    // Cabeçalho
    text(p.x + 12, p.y + 8, "BAU COMPARTILHADO (48 SLOTS)", gold, 1);

    // Botão Fechar [X]
    auto cb = closeButton(width, height);
    panel(cb.x, cb.y, cb.w, cb.h, 0x8000);
    text(cb.x + 5, cb.y + 3, "X", white, 1);

    // Quantidade de itens
    char countBuf[64];
    std::snprintf(countBuf, sizeof(countBuf), "%zu / %zu itens", stash.count(), SharedStash::MAX_CAPACITY);
    text(p.x + 240, p.y + 8, countBuf, white, 1);

    // Abas de Página (Página 1: 1..24 | Página 2: 25..48)
    for (int pIdx = 0; pIdx < 2; ++pIdx) {
      auto tb = tabButton(pIdx, width, height);
      bool active = (page == size_t(pIdx));
      panel(tb.x, tb.y, tb.w, tb.h, active ? tabActive : bgDark);
      frame(tb.x, tb.y, tb.w, tb.h, active ? gold : borderCol);
      char tabTitle[32];
      std::snprintf(tabTitle, sizeof(tabTitle), "PAGINA %d", pIdx + 1);
      text(tb.x + 10, tb.y + 4, tabTitle, active ? gold : muted, 1);
    }

    // Grid de Itens (24 slots)
    for (int i = 0; i < 24; ++i) {
      auto sr = itemSlotRect(i, width, height);
      size_t gIdx = page * PAGE_SIZE + i;
      bool hasItem = (gIdx < stash.items.size());
      bool isSelected = (selectedIndex == i);

      panel(sr.x, sr.y, sr.w, sr.h, isSelected ? 0x2167 : bgDark);
      frame(sr.x, sr.y, sr.w, sr.h, isSelected ? gold : borderCol);

      if(isSelected){auto pos=transform.point(sr.x-5,sr.y+5);paintOriginalCursor(pixels,width,height,pos.first,pos.second);}
      if (hasItem) {
        const auto& it = stash.items[gIdx];
        uint16_t col = rankColors[std::min<uint8_t>(it.rank, 4)];
        // Indicador de Raridade / Miniatura no slot
        panel(sr.x + 4, sr.y + 4, sr.w - 8, sr.h - 8, 0x1102);
        frame(sr.x + 4, sr.y + 4, sr.w - 8, sr.h - 8, col);

        char lvText[16];
        std::snprintf(lvText, sizeof(lvText), "N%d", it.level);
        text(sr.x + 6, sr.y + 8, lvText, col, 1);

        if (it.count > 1) {
          char cText[16];
          std::snprintf(cText, sizeof(cText), "x%d", it.count);
          text(sr.x + 12, sr.y + 18, cText, white, 1);
        }
      }
    }

    // Rodapé de Detalhes do Item Selecionado
    size_t curIdx = getGlobalIndex();
    int detailY = p.y + 190;
    if (curIdx < stash.items.size()) {
      const auto& it = stash.items[curIdx];
      uint16_t col = rankColors[std::min<uint8_t>(it.rank, 4)];
      char dBuf[128];
      const char* typeNames[7] = { "Arma", "Armadura", "Elmo", "Luva", "Bota", "Acessorio", "Item" };
      const char* tName = (it.itemType < 7) ? typeNames[it.itemType] : "Item";
      std::snprintf(dBuf, sizeof(dBuf), "[%s Nv %d] %s (x%d)", tName, it.level, it.name.c_str(), it.count);
      text(p.x + 14, detailY, dBuf, col, 1);

      text(p.x + 14, detailY + 14, "Pressione [ENTER] para retirar para a mochila", green, 1);
    } else {
      text(p.x + 14, detailY + 6, "Slot Vazio. Selecione itens do inventario para guardar.", muted, 1);
    }
  }
};

} // namespace destinia
