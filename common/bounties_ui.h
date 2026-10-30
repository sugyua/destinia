#pragma once
#include "bounties.h"
#include "ui_transform.h"
#include "overlay_font.h"
#include "original_ui.h"
#include <algorithm>
#include <string>
#include <vector>

namespace destinia {

class BountiesUI {
public:
  int selectedContract = 0; // 0, 1 ou 2

  inline CatalogRect panelRect(int width, int height) const {
    return { (width - 370) / 2, (height - 230) / 2 + 10, 370, 215 };
  }

  inline CatalogRect closeButton(int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + p.w - 24, p.y + 6, 18, 16 };
  }

  inline CatalogRect contractCardRect(int idx, int width, int height) const {
    auto p = panelRect(width, height);
    return { p.x + 12, p.y + 30 + idx * 56, p.w - 24, 50 };
  }

  inline CatalogRect claimButton(int idx, int width, int height) const {
    auto c = contractCardRect(idx, width, height);
    return { c.x + c.w - 85, c.y + 12, 78, 24 };
  }

  bool handleTouch(int x, int y, int width, int height, unsigned slot, BountyManager& mgr, std::function<void(uint32_t gold, uint16_t cash)> onClaim) {
    if (closeButton(width, height).contains(x, y)) {
      return false; // Fecha a UI
    }
    auto& list = mgr.getContracts(slot);
    for (size_t i = 0; i < list.size(); ++i) {
      if (claimButton(int(i), width, height).contains(x, y)) {
        selectedContract = int(i);
        auto& c = list[i];
        if (c.completed && !c.claimed) {
          c.claimed = true;
          onClaim(c.rewardGold, c.rewardCash);
        }
        return true;
      }
      if (contractCardRect(int(i), width, height).contains(x, y)) {
        selectedContract = int(i);
        return true;
      }
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height, unsigned slot, BountyManager& mgr, int uiPercent = 100) const {
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

    uint16_t panelBg = 0x1082;
    uint16_t borderCol = 0x2945;
    uint16_t gold = 0xfe60;
    uint16_t cardBg = 0x0841;
    uint16_t white = 0xffff;
    uint16_t green = 0x07e0;
    uint16_t muted = 0x7bef;
    uint16_t orange = 0xfb40;

    panel(p.x, p.y, p.w, p.h, panelBg);
    frame(p.x, p.y, p.w, p.h, borderCol);
    frame(p.x + 1, p.y + 1, p.w - 2, p.h - 2, gold);

    text(p.x + 12, p.y + 8, "CONTRATOS OFFLINE (BOUNTIES)", gold, 1);

    auto cb = closeButton(width, height);
    panel(cb.x, cb.y, cb.w, cb.h, 0x8000);
    text(cb.x + 5, cb.y + 3, "X", white, 1);

    auto& list = mgr.getContracts(slot);
    for (size_t i = 0; i < list.size() && i < 3; ++i) {
      auto& c = list[i];
      auto card = contractCardRect(int(i), width, height);
      bool isSelected = (selectedContract == int(i));

      panel(card.x, card.y, card.w, card.h, isSelected ? 0x1925 : cardBg);
      frame(card.x, card.y, card.w, card.h, isSelected ? gold : borderCol);

      // Título e Descrição
      text(card.x + 8, card.y + 6, c.title.c_str(), gold, 1);
      text(card.x + 8, card.y + 18, c.description.c_str(), muted, 1);

      // Recompensas e Progresso
      char rewBuf[64];
      std::snprintf(rewBuf, sizeof(rewBuf), "+%u Ouro  +%u Cash", c.rewardGold, c.rewardCash);
      text(card.x + 8, card.y + 32, rewBuf, orange, 1);

      // Botão de Status / Resgate
      auto btn = claimButton(int(i), width, height);
      if (c.claimed) {
        panel(btn.x, btn.y, btn.w, btn.h, 0x2104);
        frame(btn.x, btn.y, btn.w, btn.h, 0x4208);
        text(btn.x + 8, btn.y + 8, "RESGATADO", muted, 1);
      } else if (c.completed) {
        panel(btn.x, btn.y, btn.w, btn.h, 0x0540);
        frame(btn.x, btn.y, btn.w, btn.h, green);
        text(btn.x + 10, btn.y + 8, "RESGATAR", white, 1);
      } else {
        panel(btn.x, btn.y, btn.w, btn.h, 0x10a2);
        frame(btn.x, btn.y, btn.w, btn.h, borderCol);
        char progBuf[32];
        std::snprintf(progBuf, sizeof(progBuf), "%u/%u", c.currentCount, c.targetCount);
        text(btn.x + 18, btn.y + 8, progBuf, white, 1);
      }
    }
  }
};

} // namespace destinia
