#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>
#include "overlay_font.h"

namespace destinia {

struct AutoPotionConfig {
  bool enabled = true;
  int hpThresholdPercent = 50; // auto cura se HP <= 50%
  int mpThresholdPercent = 25; // auto mana se MP < 25%
  int cooldownFrames = 60;     // 1 segundo a 60fps entre consumos
};

class AutoPotionController {
public:
  AutoPotionConfig config;
  int cooldownHpTimer = 0;
  int cooldownMpTimer = 0;
  int totalHpUsed = 0;
  int totalMpUsed = 0;
  std::string lastNotice;
  int noticeTimer = 0;

  void toggle() {
    config.enabled = !config.enabled;
    lastNotice = config.enabled ? "AUTO-POCAO: ATIVADO" : "AUTO-POCAO: DESATIVADO";
    noticeTimer = 90;
  }

  void tick() {
    if (cooldownHpTimer > 0) --cooldownHpTimer;
    if (cooldownMpTimer > 0) --cooldownMpTimer;
    if (noticeTimer > 0) --noticeTimer;
  }

  // Verifica condicoes e executa consumo
  // gameBase: base da estrutura Game
  // rd16 / wr16: leitor e escritor de 16 bits na memoria ARM
  // playSound: callback de efeito sonoro
  bool update(uint32_t gameBase,
              const std::function<uint16_t(uint32_t)>& rd16,
              const std::function<void(uint32_t, uint16_t)>&,
              const std::function<bool(int)>& useNative,
              const std::function<void(int)>& = {}) {
    tick();
    if (!config.enabled || !gameBase || !rd16 || !useNative || rd16(gameBase + 0x4854) != 0) return false;
    const uint16_t curMp = rd16(gameBase + 0x48f2), curHp = rd16(gameBase + 0x48f4);
    const uint16_t maxMp = rd16(gameBase + 0x48f6), maxHp = rd16(gameBase + 0x48f8);
    if (int16_t(curHp) <= 0) return false;
    bool used = false;
    if (cooldownHpTimer == 0 && maxHp && curHp < maxHp &&
        uint32_t(curHp) * 100 <= uint32_t(maxHp) * uint32_t(config.hpThresholdPercent)) {
      // Sempre define cooldown — sucesso ou falha — para evitar nova tentativa no próximo frame.
      if (useNative(0)) {
        cooldownHpTimer = config.cooldownFrames; ++totalHpUsed; used = true;
        lastNotice = "AUTO-POCAO: PV RECUPERANDO!"; noticeTimer = 60;
      } else {
        cooldownHpTimer = config.cooldownFrames; // sem poção disponível; aguarda ciclo completo
      }
    }
    if (config.enabled && cooldownMpTimer == 0 && maxMp && curMp < maxMp &&
        uint32_t(curMp) * 100 <= uint32_t(maxMp) * uint32_t(config.mpThresholdPercent)) {
      if (useNative(1)) {
        cooldownMpTimer = config.cooldownFrames; ++totalMpUsed; used = true;
        lastNotice = "AUTO-POCAO: PM RECUPERANDO!"; noticeTimer = 60;
      } else {
        cooldownMpTimer = config.cooldownFrames;
      }
    }
    return used;
  }

  void paintNotice(uint8_t* pixels, int width, int height) const {
    if (noticeTimer <= 0 || lastNotice.empty() || !pixels) return;
    int tw = overlayTextWidth(lastNotice);
    int x = (width - tw) / 2;
    int y = height - 42;
    if(paintOriginalPanel(pixels,width,height,x-6,y-3,tw+12,18)){if(!paintOriginalText(pixels,width,height,x,y,lastNotice,config.enabled?0x07e0:0xf800,tw))paintOverlayText(pixels,width,height,x,y,lastNotice,config.enabled?0x07e0:0xf800);return;}
    // Fundo escuro
    for (int dy = -2; dy < 12; ++dy) {
      for (int dx = -6; dx < tw + 6; ++dx) {
        int px = x + dx, py = y + dy;
        if (px >= 0 && px < width && py >= 0 && py < height) {
          size_t at = (size_t(py) * width + px) * 2;
          pixels[at] = 0x00;
          pixels[at + 1] = 0x10; // Dark Gamevil Navy
        }
      }
    }
    paintOverlayText(pixels, width, height, x + 1, y + 1, lastNotice, 0x0000);
    paintOverlayText(pixels, width, height, x, y, lastNotice, config.enabled ? 0x07e0 : 0xf800);
  }
};

} // namespace destinia
