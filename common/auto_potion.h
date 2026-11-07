#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>
#include "overlay_font.h"

namespace destinia {

// IDs reais das poções devem ser confirmados via dump de memória (F12 no host).
// Valores abaixo são placeholders e NAO devem ser usados em producao sem validacao.
// Ativar o auto-pocao com IDs incorretos pode consumir itens errados do inventario.
constexpr uint16_t HP_POTION_ID = 0x0000; // Placeholder - confirme o ID real antes de ativar
constexpr uint16_t MP_POTION_ID = 0x0000; // Placeholder - confirme o ID real antes de ativar

struct AutoPotionConfig {
  // Desativado por padrao: HP_POTION_ID/MP_POTION_ID ainda sao placeholders
  // (nao sao os IDs reais dos itens). Com isso ativo, consumeItem() pode
  // decrementar por engano a quantidade de um item real que coincida com
  // esses IDs. So reative depois de confirmar os IDs reais via dump F12.
  bool enabled = false;
  int hpThresholdPercent = 50; // auto cura se HP <= 50%
  int mpThresholdPercent = 25; // auto mana se MP < 25%
  int cooldownFrames = 60;     // 60 ticks (3,75 segundos a 16 FPS)
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

  // Use the game's original item-selection/consumption function via the
  // checked native ARM32 adapter. Never decrement a guessed inventory ID or
  // write synthetic HP/MP: unknown IDs are dangerous for real saves.
  bool update(uint32_t gameBase,
              const std::function<uint16_t(uint32_t)>& rd16,
              const std::function<void(uint32_t,uint16_t)>& wr16,
              const std::function<bool(int type)>& useNative,
              const std::function<void(int)>& playSound = {}) {
    tick();
    if (!config.enabled || !gameBase || !rd16 || !wr16 || !useNative) return false;
    if (rd16(gameBase+0x4854) != 0) return false;
    const uint16_t curHp=rd16(gameBase+0x48f4);
    const uint16_t maxHp=rd16(gameBase+0x48f8);
    const uint16_t curMp=rd16(gameBase+0x48f2);
    const uint16_t maxMp=rd16(gameBase+0x48f6);
    if(curHp==0 || curHp==UINT16_MAX) return false;
    bool used=false;
    if(cooldownHpTimer==0 && maxHp>0 && curHp<maxHp &&
       uint32_t(curHp)*100<=uint32_t(maxHp)*uint32_t(config.hpThresholdPercent)) {
      const bool consumed=useNative(0);
      cooldownHpTimer=config.cooldownFrames; // throttle unsuccessful attempts
      if(consumed) {
        ++totalHpUsed;used=true;
        lastNotice="AUTO-POCAO: PV RECUPERADO!";noticeTimer=60;
        if(playSound)playSound(1);
      }
    }
    // The native callback may disable automatic use on an ARM access failure.
    if(!config.enabled)return used;
    if(cooldownMpTimer==0 && maxMp>0 && curMp<maxMp &&
       uint32_t(curMp)*100<=uint32_t(maxMp)*uint32_t(config.mpThresholdPercent)) {
      const bool consumed=useNative(1);
      cooldownMpTimer=config.cooldownFrames; // throttle unsuccessful attempts
      if(consumed) {
        ++totalMpUsed;used=true;
        lastNotice="AUTO-POCAO: PM RECUPERADO!";noticeTimer=60;
        if(playSound)playSound(1);
      }
    }
    return used;
  }

  void paintNotice(uint8_t* pixels, int width, int height) const {
    if (noticeTimer <= 0 || lastNotice.empty() || !pixels) return;
    int tw = overlayTextWidth(lastNotice);
    int x = (width - tw) / 2;
    int y = height - 42;
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
