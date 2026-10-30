#pragma once
#include "weapons_catalog.h"
#include "weapons_catalog_defs.h"
#include "ui_transform.h"
#include "overlay_font.h"
#include "original_ui.h"
#include <algorithm>
#include <string>
#include <vector>

namespace destinia {

struct CatalogRect {
  int x, y, w, h;
  bool contains(int px, int py) const {
    return px >= x && py >= y && px < x + w && py < y + h;
  }
};

class WeaponsCatalogUI {
public:
  uint8_t selectedClass = 1; // 1: Guerreiro, 2: Atirador, 3: Ladino
  CatalogItemCategory selectedCategory = CatalogItemCategory::Weapon; // 0: Armas, 1: Armaduras
  size_t page = 0;
  static constexpr size_t itemsPerPage = 4;

  inline CatalogRect categoryButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 190, p.y + 6, 85, 16 };
  }

  void toggleCategory() {
    selectedCategory = (selectedCategory == CatalogItemCategory::Weapon) ? CatalogItemCategory::Armor : CatalogItemCategory::Weapon;
    page = 0;
  }

  inline CatalogRect panelRect(int width, int height) const {
    return { (width - 360) / 2, (height - 240) / 2 + 15, 360, 220 };
  }

  inline CatalogRect tabRect(int index, int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 12 + index * 112, p.y + 26, 108, 18 };
  }

  inline CatalogRect prevButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 12, p.y + 194, 80, 18 };
  }

  inline CatalogRect nextButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + p.w - 92, p.y + 194, 80, 18 };
  }

  inline CatalogRect closeButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + p.w - 24, p.y + 6, 18, 16 };
  }

  void nextTab() {
    selectedClass = (selectedClass % 3) + 1;
    page = 0;
  }

  void prevTab() {
    selectedClass = (selectedClass == 1) ? 3 : (selectedClass - 1);
    page = 0;
  }

  void nextPage(size_t totalItems) {
    size_t totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;
    if (page + 1 < totalPages) page++;
  }

  void prevPage() {
    if (page > 0) page--;
  }

  bool handleTouch(int x, int y, int width, int height, size_t totalItems) {
    if (closeButton(width, height).contains(x, y)) {
      return false; // Sinal para fechar
    }
    if (categoryButton(width, height).contains(x, y)) {
      toggleCategory();
      return true;
    }
    for (int i = 0; i < 3; ++i) {
      if (tabRect(i, width, height).contains(x, y)) {
        selectedClass = uint8_t(i + 1);
        page = 0;
        return true;
      }
    }
    if (prevButton(width, height).contains(x, y)) {
      prevPage();
      return true;
    }
    if (nextButton(width, height).contains(x, y)) {
      nextPage(totalItems);
      return true;
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height, unsigned currentSlot, const WeaponsCatalog& catalog, int uiPercent = 100) const {
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

    auto rect = [&](int x, int y, int w, int h, uint16_t c) {
      for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
          pixel(x + dx, y + dy, c);
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

    auto frame = [&](int x, int y, int w, int h, uint16_t c) {
      for (const auto& r : originalRects) {
        if ((x == r.x && y == r.y && w == r.w && h == r.h) ||
            (x == r.x+1 && y == r.y+1 && w == r.w-2 && h == r.h-2)) return;
      }
      auto r = transform.rect(x, y, w, h);
      if (paintOriginalPanel(pixels,width,height,r.x,r.y,r.w,r.h)) return;

      for (int dx = 0; dx < w; ++dx) { pixel(x + dx, y, c); pixel(x + dx, y + h - 1, c); }
      for (int dy = 0; dy < h; ++dy) { pixel(x, y + dy, c); pixel(x + w - 1, y + dy, c); }
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

    // Paleta de cores RGB565
    const uint16_t bgDark    = 0x10a2; // Azul petróleo bem escuro
    const uint16_t panelBg   = 0x18e4;
    const uint16_t gold      = 0xe5b2;
    const uint16_t white     = 0xffff;
    const uint16_t muted     = 0x8c92;
    const uint16_t borderCol = 0x6b6b;
    const uint16_t tabActive = 0x2967;

    // Cores das Raridades
    const uint16_t rarityColors[5] = {
      0xd69a, // 0: Comum (Branca / Prata)
      0x07e0, // 1: Mágica (Verde)
      0x347f, // 2: Rara (Azul Ciano)
      0xffe0, // 3: Épica (Amarela)
      0xfb40  // 4: Lendária (Laranja Vívido)
    };
    const char* rarityLabels[5] = { "C", "M", "R", "E", "L" };

    // Fundo Principal do Painel
    panel(p.x, p.y, p.w, p.h, panelBg);
    frame(p.x, p.y, p.w, p.h, borderCol);
    frame(p.x + 1, p.y + 1, p.w - 2, p.h - 2, gold);

    // Cabeçalho e Botão de Categoria
    bool isWeapon = (selectedCategory == CatalogItemCategory::Weapon);
    text(p.x + 12, p.y + 8, isWeapon ? "CATALOGO: ARMAS" : "CATALOGO: ARMADURAS", gold, 1);

    auto catBtn = categoryButton(width, height);
    panel(catBtn.x, catBtn.y, catBtn.w, catBtn.h, tabActive);
    frame(catBtn.x, catBtn.y, catBtn.w, catBtn.h, gold);
    text(catBtn.x + 6, catBtn.y + 3, isWeapon ? "VER ARMADURAS" : "VER ARMAS", white, 1);

    // Botão Fechar [X]
    auto cb = closeButton(width, height);
    panel(cb.x, cb.y, cb.w, cb.h, 0x8000);
    text(cb.x + 5, cb.y + 3, "X", white, 1);

    // Cálculo do Progresso Geral
    auto allWeapons = getCatalogItems(selectedClass, selectedCategory);
    size_t totalExpected = allWeapons.size() * 5; // 5 raridades por item
    float pct = catalog.completionPercentage(currentSlot, selectedClass, totalExpected);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1f%%", pct);
    text(p.x + 285, p.y + 8, buf, (pct >= 100.0f) ? 0x07e0 : white, 1);

    // Barra de Progresso Gráfica
    int barX = p.x + 12, barY = p.y + 20, barW = p.w - 24, barH = 3;
    rect(barX, barY, barW, barH, bgDark);
    int fillW = int((barW * std::min(100.0f, pct)) / 100.0f);
    if (fillW > 0) {
      rect(barX, barY, fillW, barH, (pct >= 100.0f) ? 0x07e0 : gold);
    }

    // Abas de Classes
    const char* tabNames[3] = { "1. GUERREIRO", "2. ATIRADOR", "3. LADINO" };
    for (int i = 0; i < 3; ++i) {
      auto tr = tabRect(i, width, height);
      bool active = (selectedClass == (i + 1));
      panel(tr.x, tr.y, tr.w, tr.h, active ? tabActive : bgDark);
      frame(tr.x, tr.y, tr.w, tr.h, active ? gold : borderCol);
      text(tr.x + 14, tr.y + 4, tabNames[i], active ? gold : muted, 1);
    }

    // Lista de Armas na Página Atual
    size_t startIdx = page * itemsPerPage;
    size_t endIdx = std::min(startIdx + itemsPerPage, allWeapons.size());
    int cardY = p.y + 48;

    for (size_t i = startIdx; i < endIdx; ++i) {
      const auto& w = allWeapons[i];
      // Card da arma
      panel(p.x + 12, cardY, p.w - 24, 33, bgDark);
      frame(p.x + 12, cardY, p.w - 24, 33, borderCol);

      // Info da Arma: Nível e Nome
      char nameBuf[64];
      std::snprintf(nameBuf, sizeof(nameBuf), "Nv.%d %s", w.minLevel, w.name);
      text(p.x + 18, cardY + 5, nameBuf, white, 1);

      // Subtipo
      text(p.x + 18, cardY + 18, w.typeDesc, muted, 1);

      // 5 Badges de Raridades
      int badgeStartX = p.x + p.w - 140;
      for (uint8_t r = 0; r < 5; ++r) {
        int bx = badgeStartX + r * 22;
        int by = cardY + 9;
        int bw = 18, bh = 15;

        bool collected = catalog.isRecorded(currentSlot, w.baseId, r);
        if (collected) {
          panel(bx, by, bw, bh, rarityColors[r]);
          frame(bx, by, bw, bh, white);
          text(bx + 6, by + 3, rarityLabels[r], 0x0000, 1); // Letra preta sobre fundo brilhante
        } else {
          panel(bx, by, bw, bh, 0x2124); // Cinza apagado
          frame(bx, by, bw, bh, 0x39c7);
          text(bx + 5, by + 3, "-", muted, 1);
        }
      }

      cardY += 36;
    }

    // Rodapé de Navegação
    auto pb = prevButton(width, height);
    panel(pb.x, pb.y, pb.w, pb.h, (page > 0) ? bgDark : 0x0821);
    frame(pb.x, pb.y, pb.w, pb.h, (page > 0) ? borderCol : 0x2104);
    text(pb.x + 10, pb.y + 4, "< ANTERIOR", (page > 0) ? white : muted, 1);

    size_t totalPages = (allWeapons.size() + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;
    char pageBuf[32];
    std::snprintf(pageBuf, sizeof(pageBuf), "%zu / %zu", page + 1, totalPages);
    text(p.x + p.w / 2 - 16, pb.y + 4, pageBuf, gold, 1);

    auto nb = nextButton(width, height);
    bool hasNext = (page + 1 < totalPages);
    panel(nb.x, nb.y, nb.w, nb.h, hasNext ? bgDark : 0x0821);
    frame(nb.x, nb.y, nb.w, nb.h, hasNext ? borderCol : 0x2104);
    text(nb.x + 12, nb.y + 4, "PROXIMO >", hasNext ? white : muted, 1);
  }
};

} // namespace destinia
