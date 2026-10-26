// Difficulty Engine e Dificuldades Canônicas (0.10.3)
export const DifficultyTier = Object.freeze({
  Normal: 0,
  Dificil: 1,
  Pesadelo: 2,
  Inferno: 3,
  Abissal: 4
});

export const DifficultyLabels = ["Normal", "Difícil", "Pesadelo", "Inferno", "Abissal"];

export const DifficultyMultipliers = Object.freeze({
  0: { hp: 1.0,  dmg: 1.0, gold: 1.0, xp: 1.0 },
  1: { hp: 1.5,  dmg: 1.3, gold: 1.3, xp: 1.3 },
  2: { hp: 2.5,  dmg: 1.8, gold: 1.8, xp: 1.8 },
  3: { hp: 4.0,  dmg: 2.5, gold: 2.5, xp: 2.5 },
  4: { hp: 6.0,  dmg: 4.0, gold: 3.5, xp: 3.5 }
});

export class DifficultyWeb {
  constructor(engine) {
    this.engine = engine;
    this.currentTier = DifficultyTier.Normal;
    this.unlockedTier = DifficultyTier.Normal;
    this.open = false;
    this.selected = 0;
    this.statusMessage = "";
  }

  install() {
    // No ambiente Web, mantemos a seleção e interface leves sem instalar hooks de ARM
    // pesados em 0x12444 / 0x16b7c que degradam o compilador JIT do Unicorn.js em navegadores móveis.
  }

  onBossDefeated(species) {
    if (species === 108 && this.unlockedTier < DifficultyTier.Abissal) {
      if (this.currentTier === this.unlockedTier) {
        this.unlockedTier++;
        this.statusMessage = "NOVA DIFICULDADE DESBLOQUEADA!";
      }
    }
  }

  setTier(tier) {
    if (tier >= 0 && tier <= this.unlockedTier) {
      this.currentTier = tier;
      return true;
    }
    return false;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    if (key === 10 || key === 27) {
      this.open = false;
      return true;
    }
    if (key === 3 || key === 5) {
      this.selected = (this.selected + 4) % 5;
      return true;
    }
    if (key === 9 || key === 7) {
      this.selected = (this.selected + 1) % 5;
      return true;
    }
    if (key === 6 || key === 17) {
      if (this.selected <= this.unlockedTier) {
        this.currentTier = this.selected;
        this.open = false;
      } else {
        this.statusMessage = "BLOQUEADA: DERROTE DIABLO";
      }
      return true;
    }
    return true;
  }

  handleTouch(x, y, type) {
    if (!this.open) return false;
    if (type === 0) {
      const startY = 80;
      for (let i = 0; i < 5; ++i) {
        const itemY = startY + i * 26;
        if (x >= 240 && x <= 560 && y >= itemY && y <= itemY + 22) {
          this.selected = i;
          if (this.selected <= this.unlockedTier) {
            this.currentTier = this.selected;
            this.open = false;
          } else {
            this.statusMessage = "BLOQUEADA: DERROTE DIABLO";
          }
          return true;
        }
      }
      this.open = false;
    }
    return true;
  }

  paint(pixels) {
    if (!this.open) return;
    const w = 800, h = 480;

    const setPixel = (x, y, r, g, b) => {
      if (x < 0 || x >= w || y < 0 || y >= h) return;
      const at = (y * w + x) * 2;
      const c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      pixels[at] = c & 255;
      pixels[at + 1] = c >> 8;
    };

    const fillRect = (rx, ry, rw, rh, r, g, b) => {
      for (let dy = 0; dy < rh; ++dy) {
        for (let dx = 0; dx < rw; ++dx) {
          setPixel(rx + dx, ry + dy, r, g, b);
        }
      }
    };

    fillRect(220, 50, 360, 240, 16, 20, 28);
    for (let x = 220; x < 580; ++x) { setPixel(x, 50, 220, 180, 50); setPixel(x, 289, 220, 180, 50); }
    for (let y = 50; y < 290; ++y) { setPixel(220, y, 220, 180, 50); setPixel(579, y, 220, 180, 50); }

    const startY = 80;
    for (let i = 0; i < 5; ++i) {
      const itemY = startY + i * 26;
      const isSelected = (this.selected === i);
      const isUnlocked = (i <= this.unlockedTier);
      const isCurrent = (this.currentTier === i);

      if (isSelected) {
        fillRect(230, itemY, 340, 22, 40, 60, 90);
      } else {
        fillRect(230, itemY, 340, 22, 24, 28, 38);
      }

      if (isCurrent) {
        fillRect(234, itemY + 4, 14, 14, 50, 220, 80);
      } else if (!isUnlocked) {
        fillRect(234, itemY + 4, 14, 14, 120, 40, 40);
      }
    }
  }
}
