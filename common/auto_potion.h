#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>
#include "overlay_font.h"

namespace destinia {

// TODO: Substitua pelos IDs reais das poções usando o dump da tecla F12
constexpr uint16_t HP_POTION_ID = 0x0001; // Exemplo - Troque para o ID real
constexpr uint16_t MP_POTION_ID = 0x0002; // Exemplo - Troque para o ID real

struct AutoPotionConfig {
  // Desativado por padrao: HP_POTION_ID/MP_POTION_ID ainda sao placeholders
  // (nao sao os IDs reais dos itens). Com isso ativo, consumeItem() pode
  // decrementar por engano a quantidade de um item real que coincida com
  // esses IDs. So reative depois de confirmar os IDs reais via dump F12.
  bool enabled = false;
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
  bool update(uint32_t gameBase,
              const std::function<uint16_t(uint32_t)>& rd16,
              const std::function<void(uint32_t, uint16_t)>& wr16,
              const std::function<void(int)>& playSound = {}) {
    tick();
    if (!config.enabled || !gameBase || !rd16 || !wr16) return false;

    // Verificar se o jogador esta vivo e em gameplay ativo (0x4854 == 0)
    uint16_t gameState = rd16(gameBase + 0x4854);
    if (gameState != 0) return false;

    uint16_t curHp = rd16(gameBase + 0x48f2);
    uint16_t maxHp = rd16(gameBase + 0x48c4); // stat 5 (PV maximos)

    uint16_t curMp = rd16(gameBase + 0x48f6);
    uint16_t maxMp = rd16(gameBase + 0x48c6); // stat 6
    if (maxMp == 0) maxMp = rd16(gameBase + 0x48f8);

    if (curHp == 0 || maxHp == 0) return false;

    bool used = false;

    // Funcao auxiliar para buscar e consumir um item pelo ID na Bolsa 1
    auto consumeItem = [&](uint16_t targetId) -> bool {
      uint32_t bag1 = gameBase + 0x433c + 0x92; // bolsa 1 (consumiveis)
      uint16_t count = rd16(bag1);
      for (int i = 0; i < count && i < 24; ++i) {
        uint32_t rec = bag1 + 2 + 6 * i;
        uint16_t id = rd16(rec + 0);     // Offset 0: ID do item (pool)
        uint16_t qty = rd16(rec + 4);    // Offset 4: Quantidade
        if (id == targetId && qty > 0) {
          wr16(rec + 4, qty - 1);        // Consome 1 unidade
          return true;
        }
      }
      return false;
    };

    // 1. HP Potion check
    if (cooldownHpTimer == 0 && curHp < maxHp &&
        (uint32_t(curHp) * 100 <= uint32_t(maxHp) * uint32_t(config.hpThresholdPercent))) {
      if (consumeItem(HP_POTION_ID)) {
        uint16_t heal = std::max<uint16_t>(50, maxHp * 40 / 100);
        uint16_t newHp = std::min<uint16_t>(maxHp, curHp + heal);
        wr16(gameBase + 0x48f2, newHp);
        cooldownHpTimer = config.cooldownFrames;
        totalHpUsed++;
        used = true;
        lastNotice = "AUTO-POCAO: PV RECUPERADO!";
        noticeTimer = 60;
        if (playSound) playSound(1); // eff01.ogg
      }
    }

    // 2. MP Potion check
    if (cooldownMpTimer == 0 && maxMp > 0 && 
        (uint32_t(curMp) * 100 <= uint32_t(maxMp) * uint32_t(config.mpThresholdPercent))) {
      if (consumeItem(MP_POTION_ID)) {
        uint16_t healMp = std::max<uint16_t>(30, maxMp * 35 / 100);
        uint16_t newMp = std::min<uint16_t>(maxMp, curMp + healMp);
        wr16(gameBase + 0x48f6, newMp);
        cooldownMpTimer = config.cooldownFrames;
        totalMpUsed++;
        used = true;
        lastNotice = "AUTO-POCAO: PM RECUPERADO!";
        noticeTimer = 60;
        if (playSound) playSound(1);
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
