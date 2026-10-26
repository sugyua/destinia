#pragma once
#include "shared_stash.h"
#include "shared_stash_ui.h"
#include <filesystem>
#include <functional>

namespace destinia {

class SharedStashHost {
public:
  SharedStash stash;
  SharedStashUI ui;
  bool open = false;
  bool ready = false;
  std::filesystem::path saveDirectory;

  void init(const std::filesystem::path& saveDir) {
    saveDirectory = saveDir;
    stash = loadSharedStash(saveDir);
    ready = true;
  }

  bool toggle() {
    open = !open;
    return open;
  }

  bool handleKey(int key, bool down, uint32_t gameBase, const std::function<uint16_t(uint32_t)>& rd16, const std::function<void(uint32_t, uint16_t)>& wr16) {
    if (!open) return false;
    if (!down) return true;

    if (key == 27 || key == 'b' || key == 'B') { // ESC ou B fecha
      open = false;
      return true;
    }

    if (key == 3 || key == 1073741904) { // Seta Esquerda
      ui.prevSlot();
      return true;
    }
    if (key == 9 || key == 1073741903) { // Seta Direita
      ui.nextSlot();
      return true;
    }
    if (key == 5 || key == 1073741906) { // Seta Cima
      if (ui.selectedIndex >= 6) ui.selectedIndex -= 6;
      return true;
    }
    if (key == 7 || key == 1073741905) { // Seta Baixo
      if (ui.selectedIndex + 6 < 24) ui.selectedIndex += 6;
      return true;
    }
    if (key == '\t') { // TAB alterna página
      ui.nextPage();
      return true;
    }

    // ENTER: Retira item selecionado para a mochila se houver vaga
    if (key == 13 || key == 1073741912) {
      size_t gIdx = ui.getGlobalIndex();
      if (gIdx < stash.items.size() && gameBase) {
        // Encontra primeira vaga na bolsa 0 (Itens Comuns/Equip)
        int count = rd16(gameBase + 0x433c);
        if (count < 24) {
          StashItem item;
          if (stash.withdraw(gIdx, item)) {
            // Adiciona na bolsa do personagem
            uint32_t rec = gameBase + 0x433e + 6 * count;
            wr16(rec, 0);
            wr16(rec + 2, item.pool);
            wr16(rec + 4, item.count);
            wr16(gameBase + 0x433c, count + 1);
            saveSharedStash(saveDirectory, stash);
          }
        }
      }
      return true;
    }

    return true; // Bloqueia outros comandos enquanto aberto
  }

  bool handleTouch(int x, int y, int phase, int width, int height) {
    if (!open) return false;
    if (phase == 0) {
      if (!ui.handleTouch(x, y, width, height, stash.count())) {
        open = false;
      }
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height, int uiPercent = 100) const {
    if (!open) return;
    ui.paint(pixels, width, height, stash, uiPercent);
  }
};

} // namespace destinia

static destinia::SharedStashHost sharedStashHost;
