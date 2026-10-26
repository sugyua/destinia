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
  {species: 109, namePt: "Guardiao de Lava", nameEn: "Lava Guardian", goldCost: 50000, area: 2, map: 48}
];

export class BossRushUI {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.selectedIndex = 0;
    this.statusMessage = "";
  }

  openMenu() {
    this.open = true;
    this.selectedIndex = 0;
    this.statusMessage = "";
  }

  closeMenu() {
    this.open = false;
    this.statusMessage = "";
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    // Cima: 3
    if (key === 3) {
      this.selectedIndex = (this.selectedIndex - 1 + 10) % 10;
      this.statusMessage = "";
      return true;
    }
    // Baixo: 9
    if (key === 9) {
      this.selectedIndex = (this.selectedIndex + 1) % 10;
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
      const boss = BOSS_RUSH_LIST[this.selectedIndex];
      if (boss) {
        const curGold = this.engine.call(this.engine.exports.get('getMyGold'), []);
        if (curGold < boss.goldCost) {
          this.statusMessage = "OURO INSUFICIENTE!";
        } else {
          this.engine.call(this.engine.exports.get('setMyGold'), [curGold - boss.goldCost]);
          this.engine.call(this.engine.exports.get('Map_loadData'), [boss.area, boss.map, 1]);
          this.closeMenu();
        }
      }
      return true;
    }
    return true;
  }

  handleTouch(x, y, phase) {
    if (!this.open) return false;
    const w = this.engine.width, h = this.engine.height;
    const pw = Math.min(340, w - 20), ph = Math.min(220, h - 20);
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

    // Linhas
    let rowY = py + 34;
    for (let i = 0; i < BOSS_RUSH_LIST.length; i++) {
      if (y >= rowY - 2 && y < rowY + 13) {
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
      rowY += 15;
    }
    return true;
  }

  paint(pixels) {
    if (!this.open || !pixels) return;
    const width = this.engine.width, height = this.engine.height;
    const pw = Math.min(340, width - 20), ph = Math.min(220, height - 20);
    const px = Math.floor((width - pw) / 2), py = Math.floor((height - ph) / 2);

    // Fundo escuro com borda dourada
    for (let y = py; y < py + ph; y++) {
      for (let x = px; x < px + pw; x++) {
        const at = (y * width + x) * 2;
        const c = pixels[at] | (pixels[at + 1] << 8);
        const border = (x === px || x === px + 1 || x === px + pw - 1 || x === px + pw - 2 ||
                        y === py || y === py + 1 || y === py + ph - 1 || y === py + ph - 2);
        if (border) {
          pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
        } else {
          const dark = (c >> 2) & 0x39E7;
          pixels[at] = dark & 255; pixels[at + 1] = dark >>> 8;
        }
      }
    }

    const text = (dx, dy, str, color) => {
      for (let n = 0; n < str.length; n++) {
        const glyph = overlayGlyph(str[n]);
        for (let col = 0; col < 5; col++) {
          for (let row = 0; row < 11; row++) {
            if (glyph[col] & (1 << row)) {
              const rx = dx + n * 6 + col, ry = dy + row - 2;
              if (rx >= 0 && ry >= 0 && rx < width && ry < height) {
                const at = (ry * width + rx) * 2;
                pixels[at] = color & 255;
                pixels[at + 1] = color >>> 8;
              }
            }
          }
        }
      }
    };

    const textW = (str) => str.length * 6;

    // Título
    const title = "MAGO DOS BOSSES -- BOSS RUSH";
    const tx = px + Math.floor((pw - textW(title)) / 2);
    text(tx + 1, py + 8 + 1, title, 0);
    text(tx, py + 8, title, 0xd7bf);

    // Botão [X]
    text(px + pw - 24, py + 8, "[X]", 0xf800);

    // Divisória
    for (let x = px + 8; x < px + pw - 8; x++) {
      const at = ((py + 24) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    // Lista de Chefes
    let rowY = py + 34;
    for (let i = 0; i < BOSS_RUSH_LIST.length; i++) {
      const b = BOSS_RUSH_LIST[i];
      const sel = (i === this.selectedIndex);

      if (sel) {
        for (let y = rowY - 1; y < rowY + 13; y++) {
          for (let x = px + 8; x < px + pw - 8; x++) {
            const at = (y * width + x) * 2;
            const c = pixels[at] | (pixels[at + 1] << 8);
            const hl = ((c >> 1) & 0x7BEF) + 0x2104;
            pixels[at] = hl & 255; pixels[at + 1] = hl >>> 8;
          }
        }
      }

      const marker = sel ? "> " : "  ";
      const bossLine = marker + (i + 1) + ". " + b.namePt;
      const costStr = b.goldCost + " G";

      const colText = sel ? 0xffff : 0xce59;
      const colCost = sel ? 0x07e0 : 0xa510;

      text(px + 12 + 1, rowY + 1, bossLine, 0);
      text(px + 12, rowY, bossLine, colText);

      const cx = px + pw - 16 - textW(costStr);
      text(cx + 1, rowY + 1, costStr, 0);
      text(cx, rowY, costStr, colCost);

      rowY += 15;
    }

    // Rodapé
    for (let x = px + 8; x < px + pw - 8; x++) {
      const at = ((py + ph - 24) * width + x) * 2;
      pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
    }

    let myGoldVal = 0;
    try {
      myGoldVal = this.engine.call(this.engine.exports.get('getMyGold'), []);
    } catch (_) {}

    let footer = "SEU OURO: " + myGoldVal + " G  [ENTER]: DESAFIAR  [ESC]: SAIR";
    if (this.statusMessage) footer = this.statusMessage + " -- " + footer;

    text(px + 12 + 1, py + ph - 16 + 1, footer, 0);
    text(px + 12, py + ph - 16, footer, this.statusMessage ? 0xf800 : 0xd7bf);
  }
}
