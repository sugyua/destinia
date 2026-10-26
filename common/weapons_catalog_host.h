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

    // Varre as 4 bolsas de itens (0 a 3)
    for (int b = 0; b < 4; ++b) {
      int count = rd16(gameBase + 0x433c + b * 0x92);
      if (count > 24) continue;

      for (int i = 0; i < count; ++i) {
        uint32_t record = gameBase + 0x433e + b * 0x92 + 6 * i;
        if (!rd16(record + 4) || rd16(record) != 0) continue;

        int pool = rd16(record + 2);
        if (pool >= 200) continue;

        uint16_t header = rd16(gameBase + 0x2ed6 + 20 * pool);
        uint8_t itemType = (header >> 5) & 7; // 0 = Arma, 1 = Armadura (Peitoral)
        if (itemType != 0 && itemType != 1) continue;

        uint8_t rank = (header >> 8) & 7;     // 0..4 (Raridade)
        if (rank > 4) continue;

        uint8_t minLevel = uint8_t(header & 0x1f); // bitmask de level base
        CatalogItemCategory cat = (itemType == 0) ? CatalogItemCategory::Weapon : CatalogItemCategory::Armor;

        for (const auto& item : allCatalogEquipment()) {
          if (item.category == cat && item.minLevel <= minLevel) {
            if (catalog.record(slot, item.baseId, item.minLevel, rank, item.jobClass)) {
              changed = true;
            }
          }
        }
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
    if (key == 9 || key == 1073741903) { // Direita
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
