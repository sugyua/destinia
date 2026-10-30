#pragma once
#include "overlay_font.h"
#include "original_ui.h"
#include "pets_expansion.h"
#include "abyss_tower.h"
#include "stat_reset.h"
#include "new_game_plus.h"
#include "titles.h"
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstdint>

namespace destinia {

struct BossRushEntry {
  int species;
  const char* namePt;
  const char* nameEn;
  int goldCost;
  int area;
  int map;
};

inline const std::vector<BossRushEntry>& getBossRushList() {
  static const std::vector<BossRushEntry> list = {
    {100, "Guardiao da Masmorra", "Dungeon Guardian", 500, 1, 5},
    {101, "Defcan", "Defcan", 1000, 1, 16},
    {102, "Salas", "Salas", 2500, 2, 10},
    {103, "Defcan Desperto", "Awakened Defcan", 5000, 1, 21},
    {104, "Sileth", "Sileth", 7500, 1, 31},
    {105, "Berserk Tamur", "Berserk Tamur", 10000, 1, 41},
    {106, "Imperador Kaiser", "Emperor Kaiser", 15000, 2, 28},
    {107, "Baramos", "Baramos", 25000, 2, 16},
    {108, "Diablo", "Diablo", 35000, 1, 51},
    {109, "Guardiao de Lava", "Lava Guardian", 50000, 2, 48},
    {999, "ARENA GAUNTLET (10 CHEFES)", "BOSS GAUNTLET (10 BOSSES)", 10000, 1, 5}
  };
  return list;
}

class BossRushUI {
public:
  bool open = false;
  int currentTab = 0; // 0 = Chefes, 1 = Torre Abissal, 2 = Mascotes, 3 = Santuario
  int selectedIndex = 0;
  int scrollOffset = 0;
  std::string statusMessage;
  
  std::function<std::string(int)> survivalAction;
  std::function<std::vector<std::string>()> survivalRows;
  std::function<int()> getGold;
  std::function<void(int)> setGold;
  std::function<void(int, int, int)> teleport; // area, map, entry
  std::function<bool()> calcHeroStat;
  std::function<uint16_t(uint32_t)> rd16;
  std::function<void(uint32_t, uint16_t)> wr16;
  uint32_t gameBase = 0;
  fs::path saveDir;
  uint32_t currentSlot = 0;

  PetsManager pets;
  AbyssTowerManager abyss;
  TitlesStore titlesStore;

  void openMenu() {
    open = true;
    currentTab = 0;
    selectedIndex = 0;
    scrollOffset = 0;
    statusMessage.clear();
    if (!saveDir.empty()) {
      pets.load(saveDir);
      abyss.load(saveDir);
      titlesStore.load(saveDir);
    }
  }

  void closeMenu() {
    open = false;
    statusMessage.clear();
  }

  int getItemsCountInCurrentTab() const {
    if (currentTab == 0) return int(getBossRushList().size());
    if (currentTab == 1) return abyss.maxUnlockedFloor;
    if (currentTab == 2) return 4;
    if (currentTab == 3) return 3; // Reset Atributos, Reset Habilidades, Avancar Novo Jogo+
    if (currentTab == 4) return int(allTitles().size());
    if (currentTab == 5) return survivalRows?int(survivalRows().size()):0;
    return 0;
  }

  bool handleKey(int key, bool down) {
    if (!open) return false;
    if (!down) return true;

    // Alternar abas: Esquerda (5), Direita (7)
    if (key == 5) {
      currentTab = (currentTab - 1 + 6) % 6;
      selectedIndex = 0;
      scrollOffset = 0;
      statusMessage.clear();
      return true;
    }
    if (key == 7) {
      currentTab = (currentTab + 1) % 6;
      selectedIndex = 0;
      scrollOffset = 0;
      statusMessage.clear();
      return true;
    }

    const int count = getItemsCountInCurrentTab();
    // Cima: key 3
    if (key == 3) {
      if (count > 0) {
        selectedIndex = (selectedIndex - 1 + count) % count;
        if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
        if (selectedIndex >= scrollOffset + 7) scrollOffset = selectedIndex - 6;
      }
      statusMessage.clear();
      return true;
    }
    // Baixo: key 9
    if (key == 9) {
      if (count > 0) {
        selectedIndex = (selectedIndex + 1) % count;
        if (selectedIndex >= scrollOffset + 7) scrollOffset = selectedIndex - 6;
        if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
      }
      statusMessage.clear();
      return true;
    }
    // Cancelar / Voltar: key 10 ou key 27
    if (key == 10 || key == 27) {
      closeMenu();
      return true;
    }
    // Ação / Confirmar: key 6 ou key 17
    if (key == 6 || key == 17) {
      int curGold = getGold ? getGold() : 999999;

      if(currentTab==5){if(survivalAction)statusMessage=survivalAction(selectedIndex);return true;}
      if (currentTab == 0) {
        // Tab 0: Boss Rush
        const auto& list = getBossRushList();
        if (selectedIndex >= 0 && selectedIndex < int(list.size())) {
          const auto& boss = list[selectedIndex];
          if (curGold < boss.goldCost) {
            statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (setGold) setGold(curGold - boss.goldCost);
            if (teleport) teleport(boss.area, boss.map, 1);
            closeMenu();
          }
        }
      } else if (currentTab == 1) {
        // Tab 1: Torre Abissal
        const auto& floors = getAbyssFloors();
        if (selectedIndex >= 0 && selectedIndex < int(floors.size())) {
          const auto& fl = floors[selectedIndex];
          if (curGold < fl.goldCost) {
            statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (setGold) setGold(curGold - fl.goldCost);
            if (teleport) teleport(fl.area, fl.map, 1);
            abyss.currentFloor = fl.floor;
            statusMessage = "DESAFIANDO ANDAR " + std::to_string(fl.floor) + "!";
            closeMenu();
          }
        }
      } else if (currentTab == 2) {
        // Tab 2: Mascotes
        if (selectedIndex >= 0 && selectedIndex < 4) {
          pets.selectPet(selectedIndex);
          if (!saveDir.empty()) pets.save(saveDir);
          statusMessage = "MASCOTE " + std::string(pets.pets[selectedIndex].namePt) + " INVOCADO!";
        }
      } else if (currentTab == 3) {
        // Tab 3: Santuario
        if (selectedIndex == 0) {
          // Reset Atributos (5000 G)
          auto res = resetHeroStats(gameBase, getGold, setGold, calcHeroStat, rd16, wr16, 5000);
          statusMessage = res.message;
        } else if (selectedIndex == 1) {
          // Reset Habilidades (5000 G)
          auto res = resetHeroSkills(gameBase, getGold, setGold, rd16, wr16, 5000);
          statusMessage = res.message;
        } else if (selectedIndex == 2) {
          // Avancar Novo Jogo+
          auto res = NewGamePlusManager::advanceCycle(currentSlot, saveDir, teleport);
          statusMessage = res.message;
          if (res.success) closeMenu();
        }
      } else if (currentTab == 4) {
        // Tab 4: Titulos & Conquistas (0.12.0)
        const auto& tList = allTitles();
        if (selectedIndex >= 0 && selectedIndex < int(tList.size())) {
          const auto& t = tList[selectedIndex];
          if (titlesStore.isUnlocked(currentSlot, t.id)) {
            titlesStore.setEquipped(currentSlot, t.id);
            titlesStore.save(saveDir);
            statusMessage = "TITULO EQUIPADO COM SUCESSO!";
            if (calcHeroStat) calcHeroStat();
          } else {
            statusMessage = "TITULO BLOQUEADO! COMPLETE O REQUISITO.";
          }
        }
      }
      return true;
    }
    return true;
  }

  bool handleTouch(int x, int y, int type, int width, int height) {
    if (!open) return false;
    const int pw = std::min(360, width - 20), ph = std::min(220, height - 20);
    const int px = (width - pw) / 2, py = (height - ph) / 2;

    if (x < px || x >= px + pw || y < py || y >= py + ph) {
      if (type == 0) closeMenu();
      return true;
    }

    // Botão Fechar no canto superior direito
    if (x >= px + pw - 24 && y <= py + 20) {
      if (type == 0) closeMenu();
      return true;
    }

    // Abas no topo: py + 22 a py + 36
    if (y >= py + 20 && y < py + 52) {
      if (type == 0) {
        int tabW = (pw - 16) / 3;
        int t = (x - (px + 8)) / tabW + (y>=py+36?3:0);
        if (x>=px+8 && t >= 0 && t < 6) {
          currentTab = t;
          selectedIndex = 0;
          scrollOffset = 0;
          statusMessage.clear();
        }
      }
      return true;
    }

    // Linhas da lista
    int rowY = py + 60;
    const int count = getItemsCountInCurrentTab();
    for (int i = scrollOffset; i < count && i < scrollOffset + 7; i++) {
      if (y >= rowY - 2 && y < rowY + 14) {
        if (type == 0) {
          if (selectedIndex == i) {
            handleKey(17, true);
          } else {
            selectedIndex = i;
            statusMessage.clear();
          }
        }
        return true;
      }
      rowY += 16;
    }
    return true;
  }

  void paint(uint8_t* pixels, int width, int height) const {
    if (!open || !pixels) return;

    const int pw = std::min(360, width - 20), ph = std::min(220, height - 20);
    const int px = (width - pw) / 2, py = (height - ph) / 2;

    auto text = [&](uint8_t* target,int w,int h,int x,int y,const std::string& value,uint16_t color) {
      if (!paintOriginalText(target,w,h,x,y,value,color)) paintOverlayText(target,w,h,x,y,value,color);
    };

    // Fundo semitransparente escuro com moldura dourada
    if (!paintOriginalPanel(pixels,width,height,px,py,pw,ph)) for (int y = py; y < py + ph; ++y) {
      for (int x = px; x < px + pw; ++x) {
        const size_t at = (size_t(y) * width + x) * 2;
        uint16_t c = pixels[at] | (pixels[at + 1] << 8);
        bool border = (x == px || x == px + 1 || x == px + pw - 1 || x == px + pw - 2 ||
                       y == py || y == py + 1 || y == py + ph - 1 || y == py + ph - 2);
        if (border) {
          pixels[at] = 0xbf; pixels[at + 1] = 0xd7; // Dourado
        } else {
          uint16_t dark = (c >> 2) & 0x39E7; // 25% brilho
          pixels[at] = uint8_t(dark);
          pixels[at + 1] = uint8_t(dark >> 8);
        }
      }
    }

    // Título Principal
    const std::string title = "MAGO DOS BOSSES -- SALAO ANCESTRAL";
    text(pixels, width, height, px + (pw - overlayTextWidth(title)) / 2 + 1, py + 6 + 1, title, 0);
    text(pixels, width, height, px + (pw - overlayTextWidth(title)) / 2, py + 6, title, 0xd7bf);

    // Botão [X]
    text(pixels, width, height, px + pw - 24, py + 6, "[X]", 0xf800);

    // Barra de Abas
    static const char* tabNames[6] = {"[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]", "[TITULOS]", "[SOBREVIVENCIA]"};
    int tabW = (pw - 16) / 3;
    for (int t = 0; t < 6; ++t) {
      int tx = px + 8 + (t%3) * tabW;
      int tabY=py+(t<3?20:36);
      bool selTab = (t == currentTab);
      uint16_t tabCol = selTab ? 0x07e0 : 0x8c51;
      if (!paintOriginalPanel(pixels,width,height,tx,tabY,tabW-2,14) && selTab) {
        for (int y = tabY; y < tabY + 14; ++y) {
          for (int x = tx; x < tx + tabW - 2; ++x) {
            size_t at = (size_t(y) * width + x) * 2;
            pixels[at] = 0x20; pixels[at + 1] = 0x20;
          }
        }
      }
      text(pixels, width, height, tx + 4, tabY + 2, tabNames[t], tabCol);
    }

    // Linha divisória
    for (int x = px + 8; x < px + pw - 8; ++x) {
      const size_t at = (size_t(py + 54) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    // Renderizar conteúdo da aba ativa
    int rowY = py + 60;

    if(currentTab==5&&survivalRows){const auto rows=survivalRows();for(int i=scrollOffset;i<int(rows.size())&&i<scrollOffset+7;++i){text(pixels,width,height,px+12,rowY,(i==selectedIndex?"> ":"  ")+rows[i],i==selectedIndex?0xffff:0xd7bf);rowY+=16;}}
    else if (currentTab == 0) {
      // TAB 0: Chefes
      const auto& list = getBossRushList();
      for (int i = scrollOffset; i < int(list.size()) && i < scrollOffset + 7; i++) {
        const auto& b = list[i];
        const bool sel = (i == selectedIndex);
        if (sel && !paintOriginalPanel(pixels,width,height,px+8,rowY-1,pw-16,14)) {
          for (int y = rowY - 1; y < rowY + 13; ++y) {
            for (int x = px + 8; x < px + pw - 8; ++x) {
              const size_t at = (size_t(y) * width + x) * 2;
              uint16_t c = pixels[at] | (pixels[at + 1] << 8);
              uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
              pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
            }
          }
        }
        std::string line = (sel ? "> " : "  ") + std::to_string(i + 1) + ". " + b.namePt;
        std::string cost = std::to_string(b.goldCost) + " G";
        text(pixels, width, height, px + 12, rowY, line, sel ? 0xffff : 0xce59);
        text(pixels, width, height, px + pw - 16 - overlayTextWidth(cost), rowY, cost, sel ? 0x07e0 : 0xa510);
        rowY += 15;
      }
    } else if (currentTab == 1) {
      // TAB 1: Torre Abissal
      const auto& floors = getAbyssFloors();
      for (int i = scrollOffset; i < abyss.maxUnlockedFloor && i < scrollOffset + 7; i++) {
        const auto& fl = floors[i];
        const bool sel = (i == selectedIndex);
        if (sel && !paintOriginalPanel(pixels,width,height,px+8,rowY-1,pw-16,14)) {
          for (int y = rowY - 1; y < rowY + 13; ++y) {
            for (int x = px + 8; x < px + pw - 8; ++x) {
              const size_t at = (size_t(y) * width + x) * 2;
              uint16_t c = pixels[at] | (pixels[at + 1] << 8);
              uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
              pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
            }
          }
        }
        std::string line = (sel ? "> " : "  ") + std::string("Andar ") + std::to_string(fl.floor) + ": " + fl.titlePt;
        std::string info = "(Lv" + std::to_string(fl.recommendedLevel) + ") " + std::to_string(fl.goldCost) + " G";
        text(pixels, width, height, px + 12, rowY, line, fl.isBoss ? (sel ? 0xfd00 : 0xf800) : (sel ? 0xffff : 0xce59));
        text(pixels, width, height, px + pw - 16 - overlayTextWidth(info), rowY, info, sel ? 0x07e0 : 0xa510);
        rowY += 15;
      }
    } else if (currentTab == 2) {
      // TAB 2: Mascotes & Companheiros
      for (int i = 0; i < 4; ++i) {
        const auto& pt = pets.pets[i];
        const bool sel = (i == selectedIndex);
        const bool active = (pets.activePetId == i);
        if (sel && !paintOriginalPanel(pixels,width,height,px+8,rowY-1,pw-16,14)) {
          for (int y = rowY - 1; y < rowY + 13; ++y) {
            for (int x = px + 8; x < px + pw - 8; ++x) {
              const size_t at = (size_t(y) * width + x) * 2;
              uint16_t c = pixels[at] | (pixels[at + 1] << 8);
              uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
              pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
            }
          }
        }
        std::string statusMark = active ? "[ATIVO] " : "[GUARDADO] ";
        std::string line = (sel ? "> " : "  ") + statusMark + pt.namePt + " (Lv." + std::to_string(pt.level) + ")";
        text(pixels, width, height, px + 12, rowY, line, active ? 0x07e0 : (sel ? 0xffff : 0xce59));
        text(pixels, width, height, px + 24, rowY + 11, pt.descPt, 0xd7bf);
        rowY += 24;
      }
    } else if (currentTab == 3) {
      // TAB 3: Santuario do Poder
      static const char* optTitles[3] = {
        "Resetar Atributos (FOR/DES/CON/INT)",
        "Resetar Arvore de Habilidades",
        "Avancar Ciclo Novo Jogo+ (NG+)"
      };
      static const char* optSub[3] = {
        "Devolve pontos ao saldo por 5.000 G",
        "Devolve pontos ao saldo por 5.000 G",
        "Reinicia mapa preservando Lv/Itens/Mascotes"
      };

      uint32_t curCycle = NewGamePlusManager::getCycle(currentSlot, saveDir);

      for (int i = 0; i < 3; ++i) {
        const bool sel = (i == selectedIndex);
        if (sel && !paintOriginalPanel(pixels,width,height,px+8,rowY-1,pw-16,14)) {
          for (int y = rowY - 1; y < rowY + 13; ++y) {
            for (int x = px + 8; x < px + pw - 8; ++x) {
              const size_t at = (size_t(y) * width + x) * 2;
              uint16_t c = pixels[at] | (pixels[at + 1] << 8);
              uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
              pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
            }
          }
        }
        std::string line = (sel ? "> " : "  ") + std::string(optTitles[i]);
        if (i == 2) line += " [Atual: NG+" + std::to_string(curCycle) + "]";
        std::string sub = "   " + std::string(optSub[i]);
        text(pixels, width, height, px + 12, rowY, line, sel ? 0xffff : 0xce59);
        text(pixels, width, height, px + 12, rowY + 11, sub, 0xa510);
        rowY += 24;
      }
    } else if (currentTab == 4) {
      // TAB 4: Titulos & Conquistas (0.12.0)
      const auto& tList = allTitles();
      TitleId equipped = titlesStore.getEquipped(currentSlot);
      for (int i = scrollOffset; i < int(tList.size()) && i < scrollOffset + 7; i++) {
        const auto& t = tList[i];
        const bool sel = (i == selectedIndex);
        const bool unlocked = titlesStore.isUnlocked(currentSlot, t.id);
        const bool isEquipped = (equipped == t.id);
        if (sel && !paintOriginalPanel(pixels,width,height,px+8,rowY-1,pw-16,14)) {
          for (int y = rowY - 1; y < rowY + 13; ++y) {
            for (int x = px + 8; x < px + pw - 8; ++x) {
              const size_t at = (size_t(y) * width + x) * 2;
              uint16_t c = pixels[at] | (pixels[at + 1] << 8);
              uint16_t hl = ((c >> 1) & 0x7BEF) + 0x2104;
              pixels[at] = uint8_t(hl); pixels[at + 1] = uint8_t(hl >> 8);
            }
          }
        }
        std::string badge = isEquipped ? "[EQUIPADO] " : (unlocked ? "[LIBERADO] " : "[BLOQUEADO] ");
        std::string line = (sel ? "> " : "  ") + badge + t.nome;
        std::string bonus = t.atkBonusPct > 0 ? ("+" + std::to_string(t.atkBonusPct) + "% ALL") : "-";
        uint16_t col = isEquipped ? 0xd7bf : (unlocked ? (sel ? 0xffff : 0xce59) : 0x7bef);
        text(pixels, width, height, px + 12, rowY, line, col);
        text(pixels, width, height, px + pw - 16 - overlayTextWidth(bonus), rowY, bonus, unlocked ? 0x07e0 : 0x7bef);
        rowY += 15;
      }
    }

    // Rodapé
    for (int x = px + 8; x < px + pw - 8; ++x) {
      const size_t at = (size_t(py + ph - 22) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    int myGoldVal = getGold ? getGold() : 0;
    std::string footer = "OURO: " + std::to_string(myGoldVal) + " G  [< >]: ABAS  [ENTER]: CONFIRMAR  [ESC]: SAIR";
    if (!statusMessage.empty()) footer = statusMessage;

    text(pixels, width, height, px + 12 + 1, py + ph - 15 + 1, footer, 0);
    text(pixels, width, height, px + 12, py + ph - 15, footer, statusMessage.empty() ? 0xd7bf : 0xf800);
  }
};

} // namespace destinia
