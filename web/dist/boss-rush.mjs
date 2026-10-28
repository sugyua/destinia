import { ALL_TITLES, TitlesStorage, TitleId } from './titles.mjs';
import {overlayGlyph} from './overlay-font.mjs';

export const BOSS_RUSH_LIST = [
  {species: 100, namePt: "Guardiao da Masmorra", nameEn: "Dungeon Guardian", goldCost: 500, area: 1, map: 5},
  {species: 101, namePt: "Defcan", nameEn: "Defcan", goldCost: 1000, area: 1, map: 16},
  {species: 102, namePt: "Salas", nameEn: "Salas", goldCost: 2500, area: 2, map: 10},
  {species: 103, namePt: "Defcan Desperto", nameEn: "Awakened Defcan", goldCost: 5000, area: 1, map: 21},
  {species: 104, namePt: "Sileth", nameEn: "Sileth", goldCost: 7500, area: 1, map: 31},
  {species: 105, namePt: "Berserk Tamur", nameEn: "Berserk Tamur", goldCost: 10000, area: 1, map: 41},
  {species: 106, namePt: "Imperador Kaiser", nameEn: "Emperor Kaiser", goldCost: 15000, area: 2, map: 28},
  {species: 107, namePt: "Baramos", nameEn: "Baramos", goldCost: 25000, area: 2, map: 16},
  {species: 108, namePt: "Diablo", nameEn: "Diablo", goldCost: 35000, area: 1, map: 51},
  {species: 109, namePt: "Guardiao de Lava", nameEn: "Lava Guardian", goldCost: 50000, area: 2, map: 48},
  {species: 999, namePt: "ARENA GAUNTLET (10 CHEFES)", nameEn: "BOSS GAUNTLET (10 BOSSES)", goldCost: 10000, area: 1, map: 5}
];

export const ABYSS_FLOORS = Array.from({length: 50}, (_, i) => {
  const f = i + 1;
  const isBoss = (f % 5 === 0);
  return {
    floor: f,
    recommendedLevel: f * 2,
    goldCost: f * 150,
    area: (f % 2 === 0) ? 2 : 1,
    map: 5 + (f % 45),
    isBoss,
    titlePt: isBoss ? `Chefe Abissal (Andar ${f})` : `Camara dos Renegados (Andar ${f})`
  };
});

export const PETS_LIST = [
  {id: 0, namePt: "Lobo Glacial", descPt: "+10% Velocidade | +15% Critico", level: 1, xp: 0},
  {id: 1, namePt: "Fenix Ignea", descPt: "+20% Dano de Ataque | +10% Regen PV", level: 1, xp: 0},
  {id: 2, namePt: "Golem de Pedra", descPt: "+25% Defesa Fisica | +15% Vida Max", level: 1, xp: 0},
  {id: 3, namePt: "Mini-Diablo", descPt: "+15% Dano Geral | +5% Roubo de Vida", level: 1, xp: 0}
];

export class BossRushUI {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.currentTab = 0; // 0 = Chefes, 1 = Torre Abissal, 2 = Mascotes, 3 = Santuario
    this.selectedIndex = 0;
    this.scrollOffset = 0;
    this.statusMessage = "";
    this.maxUnlockedFloor = 5;
    this.activePetId = 0;
    this.ngCycle = 0;
    this.titlesStore = new TitlesStorage();
  }

  rd16(addr) {
    if (!this.engine) return 0;
    if (typeof this.engine.rd16 === 'function') return this.engine.rd16(addr);
    if (typeof this.engine.get16 === 'function') return this.engine.get16(addr);
    return 0;
  }

  wr16(addr, val) {
    if (!this.engine) return;
    if (typeof this.engine.wr16 === 'function') return this.engine.wr16(addr, val);
    if (typeof this.engine.set16 === 'function') return this.engine.set16(addr, val);
  }

  openMenu() {
    this.open = true;
    this.currentTab = 0;
    this.selectedIndex = 0;
    this.scrollOffset = 0;
    this.statusMessage = "";
  }

  closeMenu() {
    this.open = false;
    this.statusMessage = "";
  }

  getItemsCount() {
    if (this.currentTab === 0) return BOSS_RUSH_LIST.length;
    if (this.currentTab === 1) return this.maxUnlockedFloor;
    if (this.currentTab === 2) return PETS_LIST.length;
    if (this.currentTab === 3) return 3;
    if (this.currentTab === 4) return ALL_TITLES.length;
    if(this.currentTab===5)return this.engine?.features0130?.rows().length||0;
    return 0;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    // Tabs: 5 (Left), 7 (Right)
    if (key === 5) {
      this.currentTab = (this.currentTab - 1 + 6) % 6;
      this.selectedIndex = 0;
      this.scrollOffset = 0;
      this.statusMessage = "";
      return true;
    }
    if (key === 7) {
      this.currentTab = (this.currentTab + 1) % 6;
      this.selectedIndex = 0;
      this.scrollOffset = 0;
      this.statusMessage = "";
      return true;
    }

    const count = this.getItemsCount();
    // Cima: 3
    if (key === 3) {
      if (count > 0) {
        this.selectedIndex = (this.selectedIndex - 1 + count) % count;
        if (this.selectedIndex < this.scrollOffset) this.scrollOffset = this.selectedIndex;
        if (this.selectedIndex >= this.scrollOffset + 7) this.scrollOffset = this.selectedIndex - 6;
      }
      this.statusMessage = "";
      return true;
    }
    // Baixo: 9
    if (key === 9) {
      if (count > 0) {
        this.selectedIndex = (this.selectedIndex + 1) % count;
        if (this.selectedIndex >= this.scrollOffset + 7) this.scrollOffset = this.selectedIndex - 6;
        if (this.selectedIndex < this.scrollOffset) this.scrollOffset = this.selectedIndex;
      }
      this.statusMessage = "";
      return true;
    }
    // Voltar / Fechar: 10 ou 27
    if (key === 10 || key === 27) {
      this.closeMenu();
      return true;
    }
    // Confirmar / Ação: 6 ou 17
    if (key === 6 || key === 17) {
      const curGold = this.engine && this.engine.exports ? this.engine.call(this.engine.exports.get('getMyGold'), []) : 999999;

      if (this.currentTab === 0) {
        const boss = BOSS_RUSH_LIST[this.selectedIndex];
        if (boss) {
          if (curGold < boss.goldCost) {
            this.statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - boss.goldCost]);
              this.engine.call(this.engine.exports.get('Map_loadData'), [boss.area, boss.map, 1]);
            }
            this.closeMenu();
          }
        }
      } else if (this.currentTab === 1) {
        const fl = ABYSS_FLOORS[this.selectedIndex];
        if (fl) {
          if (curGold < fl.goldCost) {
            this.statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - fl.goldCost]);
              this.engine.call(this.engine.exports.get('Map_loadData'), [fl.area, fl.map, 1]);
            }
            this.statusMessage = `DESAFIANDO ANDAR ${fl.floor}!`;
            this.closeMenu();
          }
        }
      } else if (this.currentTab === 2) {
        this.activePetId = this.selectedIndex;
        this.statusMessage = `MASCOTE ${PETS_LIST[this.selectedIndex].namePt} INVOCADO!`;
      } else if (this.currentTab === 3) {
        if (this.selectedIndex === 0) {
          if (curGold < 5000) {
            this.statusMessage = "OURO INSUFICIENTE (REQUER 5000 G)";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - 5000]);
              const g = this.engine.exports.get('Game');
              const lvl = this.rd16(g + 0x48fa) || 1;
              const refund = (lvl > 1) ? (lvl - 1) * 3 : 0;
              this.wr16(g + 0x48ba, 10);
              this.wr16(g + 0x48bc, 10);
              this.wr16(g + 0x48be, 10);
              this.wr16(g + 0x48c0, 10);
              this.wr16(g + 0x48f0, (this.rd16(g + 0x48f0) || 0) + refund);
              this.engine.call(this.engine.exports.get('GameUI_calcHeroStat'), []);
            }
            this.statusMessage = "ATRIBUTOS RESETADOS! PONTOS DEVOLVIDOS";
          }
        } else if (this.selectedIndex === 1) {
          if (curGold < 5000) {
            this.statusMessage = "OURO INSUFICIENTE (REQUER 5000 G)";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - 5000]);
              const g = this.engine.exports.get('Game');
              const lvl = this.rd16(g + 0x48fa) || 1;
              const refund = (lvl > 1) ? lvl - 1 : 0;
              this.wr16(g + 0x48ee, (this.rd16(g + 0x48ee) || 0) + refund);
            }
            this.statusMessage = "HABILIDADES RESETADAS! PONTOS DEVOLVIDOS";
          }
        } else if (this.selectedIndex === 2) {
          if (this.ngCycle >= 5) {
            this.statusMessage = "CICLO MAXIMO ATINGIDO (NG+5)!";
          } else {
            this.ngCycle++;
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('Map_loadData'), [0, 1, 1]);
            }
            this.statusMessage = `NOVO JOGO+ ATIVADO (NG+${this.ngCycle})!`;
            this.closeMenu();
          }
        }
      }
      return true;
    }
    return true;
  }

  handleTouch(x, y, phase) {
    if (!this.open) return false;
    const w = this.engine.width, h = this.engine.height;
    const pw = Math.min(360, w - 20), ph = Math.min(220, h - 20);
    const px = Math.floor((w - pw) / 2), py = Math.floor((h - ph) / 2);

    if (x < px || x >= px + pw || y < py || y >= py + ph) {
      if (phase === 0) this.closeMenu();
      return true;
    }

    // Botão [X]
    if (x >= px + pw - 24 && y <= py + 20) {
      if (phase === 0) this.closeMenu();
      return true;
    }

    // Duas linhas de abas: py + 20 a py + 52.
    if (y >= py + 20 && y < py + 52) {
      if (phase === 0) {
        const tabW = Math.floor((pw - 16) / 3);
        const row = y >= py + 36 ? 1 : 0;
        const col = Math.floor((x - (px + 8)) / tabW);
        const t = row * 3 + col;
        if (x >= px + 8 && x < px + pw - 8 && col >= 0 && col < 3) {
          this.currentTab = t;
          this.selectedIndex = 0;
          this.scrollOffset = 0;
          this.statusMessage = "";
        }
      }
      return true;
    }

    // Linhas
    let rowY = py + 60;
    const count = this.getItemsCount();
    for (let i = this.scrollOffset; i < count && i < this.scrollOffset + 7; i++) {
      if (y >= rowY - 2 && y < rowY + 14) {
        if (phase === 0) {
          if (this.selectedIndex === i) {
            this.handleKey(17, true);
          } else {
            this.selectedIndex = i;
            this.statusMessage = "";
          }
        }
        return true;
      }
      rowY += 16;
    }
    return true;
  }

  paint(fb, width, height) {
    if (!this.open || !fb) return;

    const pw = Math.min(360, width - 20), ph = Math.min(220, height - 20);
    const px = Math.floor((width - pw) / 2), py = Math.floor((height - ph) / 2);

    // Fundo escuro com borda dourada
    for (let y = py; y < py + ph; y++) {
      for (let x = px; x < px + pw; x++) {
        const at = (y * width + x) * 2;
        const border = (x === px || x === px + 1 || x === px + pw - 1 || x === px + pw - 2 ||
                        y === py || y === py + 1 || y === py + ph - 1 || y === py + ph - 2);
        if (border) {
          fb[at] = 0xbf; fb[at + 1] = 0xd7;
        } else {
          const c = fb[at] | (fb[at + 1] << 8);
          const dark = (c >> 2) & 0x39e7;
          fb[at] = dark & 0xff;
          fb[at + 1] = (dark >> 8) & 0xff;
        }
      }
    }

    const title = "MAGO DOS BOSSES -- SALAO ANCESTRAL";
    this.drawText(fb, width, height, px + Math.floor((pw - this.textWidth(title)) / 2), py + 6, title, 0xd7bf);
    this.drawText(fb, width, height, px + pw - 24, py + 6, "[X]", 0xf800);

    // Abas
    const tabNames = ["[CHEFES]", "[TORRE]", "[MASCOTES]", "[SANTUARIO]", "[TITULOS]", "[SOBREVIVENCIA]"];
    const tabW = Math.floor((pw - 16) / 3);
    for (let t = 0; t < 6; t++) {
      const tx = px + 8 + (t%3) * tabW;
      const tabY=py+(t<3?20:36);
      const sel = (t === this.currentTab);
      this.drawText(fb, width, height, tx + 4, tabY + 2, tabNames[t], sel ? 0x07e0 : 0x8c51);
    }

    // Linha divisória
    for (let x = px + 8; x < px + pw - 8; x++) {
      const at = ((py + 52) * width + x) * 2;
      fb[at] = 0xbf; fb[at + 1] = 0xd7;
    }

    let rowY = py + 60;
    if (this.currentTab === 0) {
      for (let i = this.scrollOffset; i < BOSS_RUSH_LIST.length && i < this.scrollOffset + 7; i++) {
        const b = BOSS_RUSH_LIST[i];
        const sel = (i === this.selectedIndex);
        const line = (sel ? "> " : "  ") + (i + 1) + ". " + b.namePt;
        const cost = b.goldCost + " G";
        this.drawText(fb, width, height, px + 12, rowY, line, sel ? 0xffff : 0xce59);
        this.drawText(fb, width, height, px + pw - 16 - this.textWidth(cost), rowY, cost, sel ? 0x07e0 : 0xa510);
        rowY += 15;
      }
    } else if (this.currentTab === 1) {
      for (let i = this.scrollOffset; i < this.maxUnlockedFloor && i < this.scrollOffset + 7; i++) {
        const fl = ABYSS_FLOORS[i];
        const sel = (i === this.selectedIndex);
        const line = (sel ? "> " : "  ") + "Andar " + fl.floor + ": " + fl.titlePt;
        const info = "(Lv" + fl.recommendedLevel + ") " + fl.goldCost + " G";
        this.drawText(fb, width, height, px + 12, rowY, line, fl.isBoss ? (sel ? 0xfd00 : 0xf800) : (sel ? 0xffff : 0xce59));
        this.drawText(fb, width, height, px + pw - 16 - this.textWidth(info), rowY, info, sel ? 0x07e0 : 0xa510);
        rowY += 15;
      }
    } else if (this.currentTab === 2) {
      for (let i = 0; i < PETS_LIST.length; i++) {
        const pt = PETS_LIST[i];
        const sel = (i === this.selectedIndex);
        const active = (this.activePetId === i);
        const line = (sel ? "> " : "  ") + (active ? "[ATIVO] " : "[GUARDADO] ") + pt.namePt;
        this.drawText(fb, width, height, px + 12, rowY, line, active ? 0x07e0 : (sel ? 0xffff : 0xce59));
        this.drawText(fb, width, height, px + 24, rowY + 11, pt.descPt, 0xd7bf);
        rowY += 24;
      }
    } else if (this.currentTab === 3) {
      const titles = [
        "Resetar Atributos (FOR/DES/CON/INT)",
        "Resetar Arvore de Habilidades",
        `Avancar Ciclo Novo Jogo+ [NG+${this.ngCycle}]`
      ];
      for (let i = 0; i < 3; i++) {
        const sel = (i === this.selectedIndex);
        const line = (sel ? "> " : "  ") + titles[i];
        this.drawText(fb, width, height, px + 12, rowY, line, sel ? 0xffff : 0xce59);
        rowY += 24;
      }
    }

    // Rodapé
    for (let x = px + 8; x < px + pw - 8; x++) {
      const at = ((py + ph - 22) * width + x) * 2;
      fb[at] = 0xbf; fb[at + 1] = 0xd7;
    }

    const curGold = this.engine && this.engine.exports ? this.engine.call(this.engine.exports.get('getMyGold'), []) : 0;
    let footer = "OURO: " + curGold + " G  [< >]: ABAS  [ENTER]: CONFIRMAR  [ESC]: SAIR";
    if (this.statusMessage) footer = this.statusMessage;
    this.drawText(fb, width, height, px + 12, py + ph - 15, footer, this.statusMessage ? 0xf800 : 0xd7bf);
  }

  textWidth(str) {
    let w = 0;
    for (const ch of str) {
      const g = overlayGlyph(ch);
      w += g ? g.width + 1 : 6;
    }
    return w;
  }

  drawText(fb, width, height, x, y, str, color) {
    let cx = x;
    for (const ch of str) {
      const glyph = overlayGlyph(ch);
      if (glyph) {
        for (let gy = 0; gy < glyph.height; gy++) {
          for (let gx = 0; gx < glyph.width; gx++) {
            if (glyph.rows[gy] & (1 << (glyph.width - 1 - gx))) {
              const tx = cx + gx, ty = y + gy;
              if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
                const at = (ty * width + tx) * 2;
                fb[at] = color & 0xff;
                fb[at + 1] = (color >> 8) & 0xff;
              }
            }
          }
        }
        cx += glyph.width + 1;
      } else {
        cx += 6;
      }
    }
  }
}
