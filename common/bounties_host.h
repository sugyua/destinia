#pragma once
#include "bounties.h"
#include "bounties_ui.h"
#include <filesystem>
#include <functional>

namespace destinia {

class BountiesHost {
public:
  BountyManager manager;
  BountiesUI ui;
  bool open = false;
  bool ready = false;
  std::filesystem::path saveDirectory;

  void init(const std::filesystem::path& saveDir) {
    saveDirectory = saveDir;
    manager = loadBounties(saveDir);
    ready = true;
  }

  bool toggle() {
    open = !open;
    return open;
  }

  void onMonsterKilled(unsigned slot, uint16_t species) {
    if (!ready || slot >= 10) return;
    manager.onMonsterKilled(slot, species);
    saveBounties(saveDirectory, manager);
  }

  bool handleKey(int key, bool down, unsigned slot, std::function<void(uint32_t gold, uint16_t cash)> onClaim) {
    if (!open) return false;
    if (!down) return true;

    if (key == 27 || key == 'c' || key == 'C') { // ESC ou C fecha
      open = false;
      return true;
    }

    if (key == 5 || key == 1073741906) { // Cima
      if (ui.selectedContract > 0) ui.selectedContract--;
      return true;
    }
    if (key == 7 || key == 1073741905) { // Baixo
      if (ui.selectedContract < 2) ui.selectedContract++;
      return true;
    }

    // ENTER: Resgatar se completo
    if (key == 13 || key == 1073741912) {
      auto& list = manager.getContracts(slot);
      if (ui.selectedContract < int(list.size())) {
        auto& c = list[ui.selectedContract];
        if (c.completed && !c.claimed) {
          c.claimed = true;
          onClaim(c.rewardGold, c.rewardCash);
          saveBounties(saveDirectory, manager);
        }
      }
      return true;
    }

    return true;
  }

  bool handleTouch(int x, int y, int phase, int width, int height, unsigned slot, std::function<void(uint32_t gold, uint16_t cash)> onClaim) {
    if (!open) return false;
    if (phase == 0) {
      if (!ui.handleTouch(x, y, width, height, slot, manager, onClaim)) {
        open = false;
      } else {
        saveBounties(saveDirectory, manager);
      }
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height, unsigned slot, int uiPercent = 100) {
    if (!open) return;
    ui.paint(pixels, width, height, slot, manager, uiPercent);
  }
};

} // namespace destinia

static destinia::BountiesHost bountiesHost;
