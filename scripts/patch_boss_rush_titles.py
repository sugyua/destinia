#!/usr/bin/env python3
from pathlib import Path

# 1. Update common/boss_rush_ui.h
p_brui = Path('common/boss_rush_ui.h')
text = p_brui.read_text(encoding='utf-8')

if '#include "titles.h"' not in text:
    text = text.replace('#include "new_game_plus.h"', '#include "new_game_plus.h"\n#include "titles.h"')

# Update titlesStore member
if 'TitlesStore titlesStore;' not in text:
    text = text.replace('AbyssTowerManager abyss;', 'AbyssTowerManager abyss;\n  TitlesStore titlesStore;')

# Update openMenu()
if 'titlesStore.load(saveDir);' not in text:
    text = text.replace('abyss.load(saveDir);', 'abyss.load(saveDir);\n      titlesStore.load(saveDir);')

# Update getItemsCountInCurrentTab()
if 'if (currentTab == 4)' not in text:
    text = text.replace(
        'if (currentTab == 3) return 3; // Reset Atributos, Reset Habilidades, Avancar Novo Jogo+\n    return 0;',
        'if (currentTab == 3) return 3; // Reset Atributos, Reset Habilidades, Avancar Novo Jogo+\n    if (currentTab == 4) return int(allTitles().size());\n    return 0;'
    )

# Update tab cycling (% 4 -> % 5)
text = text.replace('(currentTab - 1 + 4) % 4', '(currentTab - 1 + 5) % 5')
text = text.replace('(currentTab + 1) % 4', '(currentTab + 1) % 5')

# Update tab names and width
old_tab_names = 'static const char* tabNames[4] = {"[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]"};\n    int tabW = (pw - 16) / 4;\n    for (int t = 0; t < 4; ++t) {'
new_tab_names = 'static const char* tabNames[5] = {"[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]", "[TITULOS]"};\n    int tabW = (pw - 16) / 5;\n    for (int t = 0; t < 5; ++t) {'
text = text.replace(old_tab_names, new_tab_names)

old_touch_tab = 'int tabW = (pw - 16) / 4;\n    if (x >= px + 8 && x < px + 8 + 4 * tabW && y >= py + 20 && y <= py + 34) {\n      if (type == 0) {\n        int t = (x - (px + 8)) / tabW;\n        if (t >= 0 && t < 4) {'
new_touch_tab = 'int tabW = (pw - 16) / 5;\n    if (x >= px + 8 && x < px + 8 + 5 * tabW && y >= py + 20 && y <= py + 34) {\n      if (type == 0) {\n        int t = (x - (px + 8)) / tabW;\n        if (t >= 0 && t < 5) {'
text = text.replace(old_touch_tab, new_touch_tab)

# Add Tab 4 action handling
old_action_tab3 = '}\n      }\n      return true;\n    }\n    return true;\n  }'
new_action_tab4 = """}
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
  }"""
if 'else if (currentTab == 4)' not in text:
    text = text.replace(old_action_tab3, new_action_tab4)

# Add Tab 4 painting
paint_tab4_block = """    } else if (currentTab == 4) {
      // TAB 4: Titulos & Conquistas (0.12.0)
      const auto& tList = allTitles();
      TitleId equipped = titlesStore.getEquipped(currentSlot);
      for (int i = scrollOffset; i < int(tList.size()) && i < scrollOffset + 7; i++) {
        const auto& t = tList[i];
        const bool sel = (i == selectedIndex);
        const bool unlocked = titlesStore.isUnlocked(currentSlot, t.id);
        const bool isEquipped = (equipped == t.id);
        if (sel) {
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
        paintOverlayText(pixels, width, height, px + 12, rowY, line, col);
        paintOverlayText(pixels, width, height, px + pw - 16 - overlayTextWidth(bonus), rowY, bonus, unlocked ? 0x07e0 : 0x7bef);
        rowY += 15;
      }
    }
"""

if 'paint_tab4_block' not in text and 'TAB 4: Titulos' not in text:
    text = text.replace(
        'rowY += 24;\n      }\n    }\n\n    // Rodapé',
        'rowY += 24;\n      }\n' + paint_tab4_block + '\n    // Rodapé'
    )

p_brui.write_text(text, encoding='utf-8')
print('common/boss_rush_ui.h updated with 5th tab (Titulos)!')

# 2. Update web/dist/boss-rush.mjs
p_web_br = Path('web/dist/boss-rush.mjs')
web_text = p_web_br.read_text(encoding='utf-8')

if 'import { ALL_TITLES' not in web_text:
    web_text = "import { ALL_TITLES, TitlesStorage, TitleId } from './titles.mjs';\n" + web_text

if 'this.titlesStore = new TitlesStorage();' not in web_text:
    web_text = web_text.replace(
        'this.ngCycle = 0;\n  }',
        'this.ngCycle = 0;\n    this.titlesStore = new TitlesStorage();\n  }'
    )

if 'if (this.currentTab === 4) return ALL_TITLES.length;' not in web_text:
    web_text = web_text.replace(
        'if (this.currentTab === 3) return 3;\n    return 0;',
        'if (this.currentTab === 3) return 3;\n    if (this.currentTab === 4) return ALL_TITLES.length;\n    return 0;'
    )

# Tab cycling
web_text = web_text.replace('(this.currentTab - 1 + 4) % 4', '(this.currentTab - 1 + 5) % 5')
web_text = web_text.replace('(this.currentTab + 1) % 4', '(this.currentTab + 1) % 5')

# Tab names
web_text = web_text.replace('["[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]"]', '["[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]", "[TITULOS]"]')

# Action
web_tab4_action = """} else if (this.currentTab === 4) {
        const title = ALL_TITLES[this.selectedIndex];
        if (title) {
          if (this.titlesStore.isUnlocked(0, title.id)) {
            this.titlesStore.setEquipped(0, title.id);
            this.statusMessage = "TITULO EQUIPADO COM SUCESSO!";
          } else {
            this.statusMessage = "TITULO BLOQUEADO! COMPLETE O REQUISITO.";
          }
        }
      }"""

if 'else if (this.currentTab === 4)' not in web_text:
    web_text = web_text.replace(
        'this.statusMessage = `CICLO AVANCADO PARA NG+${this.ngCycle}! DIFICULDADE +35%`;\n      }',
        'this.statusMessage = `CICLO AVANCADO PARA NG+${this.ngCycle}! DIFICULDADE +35%`;\n      ' + web_tab4_action
    )

p_web_br.write_text(web_text, encoding='utf-8')
print('web/dist/boss-rush.mjs updated with 5th tab (Titulos)!')
