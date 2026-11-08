#pragma once
#include "weapons_catalog.h"
#include "weapons_catalog_ui.h"
#include "weapons_catalog_defs.h"
#include <filesystem>
#include <memory>
#include <functional>

namespace destinia {

class WeaponsCatalogHost {
public:
  WeaponsCatalog catalog;
  WeaponsCatalogUI ui;
  bool open = false;
  bool ready = false;
  std::filesystem::path saveDirectory;

  void init(const std::filesystem::path& saveDir) {
    saveDirectory = saveDir;
    catalog = loadWeaponsCatalog(saveDir);
    ready = true;
  }

  void scanInventory(unsigned slot, uint32_t gameBase, const std::function<uint16_t(uint32_t)>& rd16) {
    if (!ready || slot >= 10 || !gameBase) return;
    bool changed = false;

    auto registerItem = [&](uint16_t header) {
      uint8_t itemType = (header >> 5) & 7;
      // 0: Espada 2M (Guerreiro)
      // 1: Espada 1M (Ladino / Guerreiro)
      // 2: Arma de Fogo (Atirador)
      // 3: Cajado / Mago
      // 4: Armadura (Peitoral)
      CatalogItemCategory cat = (itemType == 4) ? CatalogItemCategory::Armor : CatalogItemCategory::Weapon;
      uint8_t job = (itemType == 0) ? 1 : (itemType == 2) ? 2 : (itemType == 1) ? 3 : (itemType == 3) ? 4 : 0;
      if (cat == CatalogItemCategory::Weapon && job == 0) return;

      uint8_t rank = (header >> 8) & 7; // 0..4
      if (rank > 4) return;

      uint8_t levelIdx = uint8_t(header & 0x1f);
      int equipLevel = (levelIdx * 5 > 0) ? (levelIdx * 5) : 1;

      for (const auto& item : allCatalogEquipment()) {
        if (item.category == cat && (cat == CatalogItemCategory::Armor || item.jobClass == job) && item.minLevel == equipLevel) {
          if (catalog.record(slot, item.baseId, item.minLevel, rank, item.jobClass)) {
            changed = true;
          }
        }
      }
    };

    // 1. Escaneia equipamentos atualmente equipados no personagem (6 slots)
    for (int w = 0; w < 6; ++w) {
      uint32_t low = rd16(gameBase + 0x4584 + 4 * w);
      uint32_t high = rd16(gameBase + 0x4584 + 4 * w + 2);
      uint32_t p = low | (high << 16);
      if (p >= gameBase + 0x2ed6 && p < gameBase + 0x2ed6 + 200 * 20) {
        uint16_t header = rd16(p);
        registerItem(header);
      }
    }

    // 2. Escaneia as 4 bolsas de itens (0 a 3)
    for (int b = 0; b < 4; ++b) {
      int count = rd16(gameBase + 0x433c + b * 0x92);
      if (count > 24) continue;

      for (int i = 0; i < count; ++i) {
        uint32_t record = gameBase + 0x433e + b * 0x92 + 6 * i;
        if (!rd16(record + 4) || rd16(record) != 0) continue;

        int pool = rd16(record + 2);
        if (pool >= 200) continue;

        uint16_t header = rd16(gameBase + 0x2ed6 + 20 * pool);
        registerItem(header);
      }
    }

    if (changed) {
      saveWeaponsCatalog(saveDirectory, catalog);
    }
  }

  bool toggle(unsigned slot, uint32_t gameBase, const std::function<uint16_t(uint32_t)>& rd16) {
    open = !open;
    if (open) {
      scanInventory(slot, gameBase, rd16);
    }
    return open;
  }

  bool handleKey(int key, bool down) {
    if (!open) return false;
    if (!down) return true; // Bloqueia propagação de keyup

    if (key == 27 || key == 'w' || key == 'W') { // ESC ou W
      open = false;
      return true;
    }
    if (key == 3 || key == 1073741904) { // Esquerda
      ui.prevPage();
      return true;
    }
    if (key == 1073741903) { // Seta para direita (9 e TAB sao troca de classe)
      auto list = getCatalogItems(ui.selectedClass, ui.selectedCategory);
      ui.nextPage(list.size());
      return true;
    }
    if (key == 9 || key == '\t') { // TAB
      ui.nextTab();
      return true;
    }
    if (key == 't' || key == 'T') {
      ui.toggleCategory();
      return true;
    }
    if (key == '1') { ui.selectedClass = 1; ui.page = 0; return true; }
    if (key == '2') { ui.selectedClass = 2; ui.page = 0; return true; }
    if (key == '3') { ui.selectedClass = 3; ui.page = 0; return true; }
    if (key == '4') { ui.selectedClass = 4; ui.page = 0; return true; }

    return true; // Consome todas as outras teclas enquanto aberto
  }

  bool handleTouch(int x, int y, int phase, int width, int height) {
    if (!open) return false;
    if (phase == 0) { // Click / Touch Down
      auto list = getCatalogItems(ui.selectedClass, ui.selectedCategory);
      if (!ui.handleTouch(x, y, width, height, list.size())) {
        open = false; // Botão de fechar pressionado
      }
    }
    return true; // Consome evento
  }

  void paint(uint8_t* pixels, int width, int height, unsigned currentSlot, int uiPercent = 100) const {
    if (!open) return;
    ui.paint(pixels, width, height, currentSlot, catalog, uiPercent);
  }
};

} // namespace destinia

static destinia::WeaponsCatalogHost weaponsCatalogHost;
