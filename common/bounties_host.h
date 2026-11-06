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
    try {
      manager = loadBounties(saveDir);
      ready = true;
    } catch (...) {
      // Never reset/overwrite a corrupt or unrecognized user's sidecar.
      manager = BountyManager{};
      ready = false;
      open = false;
    }
  }

  bool toggle() {
    if (!ready) return false;
    open = !open;
    return open;
  }

  bool onMonsterKilled(unsigned slot, uint16_t species) {
    if (!ready || slot >= 10) return false;
    const auto previous=manager;
    manager.onMonsterKilled(slot, species);
    if (saveBounties(saveDirectory, manager)) return true;
    manager=previous; // no unpersisted progress after a write failure
    return false;
  }

  // Commit claim flag first. Refuse the reward if persistence failed;
  // this provides at-most-once credit but not a cross-file exactly-once
  // transaction with the original ARM32 gold/cash values.
  bool claim(unsigned slot, size_t index,
             const std::function<void(uint32_t gold, uint16_t cash)>& onClaim) {
    if (slot >= 10 || !onClaim) return false;
    auto& list = manager.getContracts(slot);
    if (index >= list.size()) return false;
    auto& c = list[index];
    if (!c.completed || c.claimed) return false;
    c.claimed = true;
    if (!saveBounties(saveDirectory, manager)) {
      c.claimed = false;
      return false;
    }
    onClaim(c.rewardGold, c.rewardCash);
    return true;
  }

  bool handleKey(int key, bool down, unsigned slot, std::function<void(uint32_t gold, uint16_t cash)> onClaim) {
    if (!open || !ready) return false;
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
      if (ui.selectedContract >= 0)
        claim(slot, static_cast<size_t>(ui.selectedContract), onClaim);
      return true;
    }

    return true;
  }

  bool handleTouch(int x, int y, int phase, int width, int height, unsigned slot, std::function<void(uint32_t gold, uint16_t cash)> onClaim) {
    if (!open || !ready) return false;
    if (phase == 0) {
      if (slot >= 10) return true;
      if (!ui.handleTouch(x, y, width, height, slot, manager,
                         [&](size_t index) { claim(slot, index, onClaim); })) {
        open = false;
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
