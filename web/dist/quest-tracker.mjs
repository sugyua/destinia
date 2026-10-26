// Rastreador de Missões no HUD (Quest Tracker HUD) Web (0.11.0 / 0.12.0)
// Exibe a missão ativa, o alvo de abate e o destino no estilo visual clássico da Gamevil.

export class QuestTrackerWeb {
  constructor(engine) {
    this.engine = engine;
    this.visible = true;
    this.minimized = false;
    this.questTitle = "Caça: Soldados Imperiais";
    this.questObjective = "Abates: 0 / 15";
    this.destination = "Estrada Imperial";
  }

  setQuest(title, objective, dest = "") {
    this.questTitle = title;
    this.questObjective = objective;
    this.destination = dest;
  }

  toggle() {
    this.minimized = !this.minimized;
  }

  handleKey(key, down) {
    if (!down) return false;
    if (key === 'q' || key === 'Q') {
      this.toggle();
      return true;
    }
    return false;
  }

  handleTouch(x, y, type) {
    if (type !== 0) return false;
    const w = 800;
    const boxW = 210;
    const boxH = this.minimized ? 22 : 52;
    const boxX = w - boxW - 10;
    const boxY = 10;

    if (x >= boxX && x <= boxX + boxW && y >= boxY && y <= boxY + boxH) {
      this.toggle();
      return true;
    }
    return false;
  }

  updateFromBounties() {
    if (!this.engine?.bounties) return;
    const slot = this.engine.bounties.currentSlot();
    const contracts = this.engine.bounties.getContracts(slot);
    if (contracts && contracts.length > 0) {
      const active = contracts.find(c => !c.completed) || contracts[0];
      this.setQuest(
        active.title,
        `Abates: ${active.currentCount} / ${active.targetCount}`,
        active.type === 1 ? "Boss Rush / Mapa" : "Estrada Imperial"
      );
    }
  }

  paint(pixels) {
    if (!this.visible || !pixels) return;
    this.updateFromBounties();

    const w = 800, h = 480;
    const boxW = 210;
    const boxH = this.minimized ? 22 : 52;
    const boxX = w - boxW - 10;
    const boxY = 10;

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

    // Fundo escuro translúcido Gamevil
    fillRect(boxX, boxY, boxW, boxH, 18, 22, 28);

    // Borda superior e inferior
    for (let x = boxX; x < boxX + boxW; ++x) {
      setPixel(x, boxY, 210, 170, 40);             // Borda dourada
      setPixel(x, boxY + boxH - 1, 140, 110, 25);  // Sombra dourada
    }
    // Borda lateral esquerda e direita
    for (let y = boxY; y < boxY + boxH; ++y) {
      setPixel(boxX, y, 210, 170, 40);
      setPixel(boxX + boxW - 1, y, 140, 110, 25);
    }

    // Ícone de Pergaminho (10x10)
    const iconX = boxX + 6, iconY = boxY + 4;
    fillRect(iconX + 1, iconY, 8, 2, 220, 190, 80);
    fillRect(iconX, iconY + 2, 10, 6, 240, 220, 140);
    fillRect(iconX + 1, iconY + 8, 8, 2, 220, 190, 80);
    setPixel(iconX + 3, iconY + 4, 140, 90, 30);
    setPixel(iconX + 5, iconY + 4, 140, 90, 30);
    setPixel(iconX + 7, iconY + 4, 140, 90, 30);
    setPixel(iconX + 3, iconY + 6, 140, 90, 30);
    setPixel(iconX + 5, iconY + 6, 140, 90, 30);

    // Barra de mini status visual indicando progresso
    if (!this.minimized && this.engine?.bounties) {
      const slot = this.engine.bounties.currentSlot();
      const contracts = this.engine.bounties.getContracts(slot);
      if (contracts && contracts.length > 0) {
        const c = contracts.find(x => !x.completed) || contracts[0];
        const pct = Math.min(100, Math.floor((c.currentCount / Math.max(1, c.targetCount)) * 100));
        fillRect(boxX + 20, boxY + 36, 180, 4, 12, 14, 18);
        const fillW = Math.floor((180 * pct) / 100);
        if (fillW > 0) {
          fillRect(boxX + 20, boxY + 36, fillW, 4, 220, 180, 40);
        }
      }
    }
  }
}
