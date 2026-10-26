// Sistema de Recompensas e Contratos Offline Web (0.11.0)
// Permite caçar monstros e chefes para obter ouro e moedas de cash offline.

export class BountiesWeb {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.contractsBySlot = new Map();
    this.load();
  }

  load() {
    try {
      if (typeof localStorage !== 'undefined') {
        const raw = localStorage.getItem('destinia_bounties_v1');
        if (raw) {
          this.deserialize(raw);
        }
      }
    } catch (_) {}
  }

  save() {
    try {
      if (typeof localStorage !== 'undefined') {
        localStorage.setItem('destinia_bounties_v1', this.serialize());
      }
    } catch (_) {}
  }

  serialize() {
    const lines = ["bounties_version 1"];
    for (const [slot, list] of this.contractsBySlot.entries()) {
      for (const c of list) {
        lines.push(`bounty ${slot} ${c.id} ${c.type} ${c.targetId} ${c.targetCount} ${c.currentCount} ${c.rewardGold} ${c.rewardCash} ${c.completed ? 1 : 0} ${c.claimed ? 1 : 0}`);
      }
    }
    return lines.join('\n');
  }

  deserialize(text) {
    const lines = text.split('\n');
    this.contractsBySlot.clear();
    let valid = false;
    for (let line of lines) {
      line = line.trim();
      if (!line || line.startsWith('#')) continue;
      const parts = line.split(/\s+/);
      if (parts[0] === 'bounties_version' && parts[1] === '1') {
        valid = true;
      } else if (valid && parts[0] === 'bounty' && parts.length >= 10) {
        const slot = parseInt(parts[1], 10);
        if (!this.contractsBySlot.has(slot)) {
          this.contractsBySlot.set(slot, []);
        }
        const id = parseInt(parts[2], 10);
        const type = parseInt(parts[3], 10);
        const targetId = parseInt(parts[4], 10);
        const targetCount = parseInt(parts[5], 10);
        const currentCount = parseInt(parts[6], 10);
        const rewardGold = parseInt(parts[7], 10);
        const rewardCash = parseInt(parts[8], 10);
        const completed = parts[9] === '1';
        const claimed = parts[10] === '1';

        let title = "Contrato de Caça";
        let description = `Elimine o alvo ${targetId}.`;
        if (id === 1) {
          title = "Caça: Soldados Imperiais";
          description = `Elimine ${targetCount} Soldados Imperiais.`;
        } else if (id === 2) {
          title = "Bounty: Diablo, Senhor do Caos";
          description = "Derrote o Boss Diablo no Boss Rush ou mapa.";
        } else if (id === 3) {
          title = "Bounty: Guardião Ancestral";
          description = `Derrote o Guardião Ancestral (Boss ${targetId}).`;
        }

        this.contractsBySlot.get(slot).push({
          id, type, targetId, targetCount, currentCount, rewardGold, rewardCash, completed, claimed, title, description
        });
      }
    }
  }

  getContracts(slot = 0) {
    if (!this.contractsBySlot.has(slot) || this.contractsBySlot.get(slot).length === 0) {
      this.generateDailyContracts(slot);
    }
    return this.contractsBySlot.get(slot);
  }

  generateDailyContracts(slot = 0) {
    const list = [
      {
        id: 1,
        type: 0,
        targetId: 61, // Imperial Soldier
        targetCount: 15,
        currentCount: 0,
        rewardGold: 15000,
        rewardCash: 200,
        completed: false,
        claimed: false,
        title: "Caça: Soldados Imperiais",
        description: "Elimine 15 Soldados Imperiais nas estradas."
      },
      {
        id: 2,
        type: 1,
        targetId: 108, // Diablo
        targetCount: 1,
        currentCount: 0,
        rewardGold: 35000,
        rewardCash: 500,
        completed: false,
        claimed: false,
        title: "Bounty: Diablo, Senhor do Caos",
        description: "Derrote o Boss Diablo no Boss Rush ou mapa."
      },
      {
        id: 3,
        type: 1,
        targetId: 105, // Guardião Ancestral
        targetCount: 1,
        currentCount: 0,
        rewardGold: 25000,
        rewardCash: 350,
        completed: false,
        claimed: false,
        title: "Bounty: Guardião Ancestral",
        description: "Derrote o Guardião Ancestral (Boss 105)."
      }
    ];
    this.contractsBySlot.set(slot, list);
    this.save();
    return list;
  }

  onMonsterKilled(slot = 0, species = 0) {
    const list = this.getContracts(slot);
    let changed = false;
    for (const c of list) {
      const matches = (!c.completed) && (
        c.targetId === species ||
        (c.id === 1 && (species === 61 || species === 62 || species === 63 || species === 2))
      );
      if (matches) {
        c.currentCount++;
        if (c.currentCount >= c.targetCount) {
          c.currentCount = c.targetCount;
          c.completed = true;
        }
        changed = true;
      }
    }
    if (changed) {
      this.save();
    }
  }

  claim(slot = 0, contractId = 1) {
    const list = this.getContracts(slot);
    const c = list.find(x => x.id === contractId);
    if (!c || !c.completed || c.claimed) return null;
    c.claimed = true;
    this.save();
    return { gold: c.rewardGold, cash: c.rewardCash };
  }

  currentSlot() {
    if (!this.engine?.exports) return 0;
    const g = this.engine.exports.get("Game");
    if (!g) return 0;
    const s = this.engine.get16(g + 0x4886);
    return s < 10 ? s : 0;
  }

  toggle() {
    this.open = !this.open;
    if (this.open) {
      this.load();
    }
    return this.open;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    if (key === 27 || key === 10 || key === 'c' || key === 'C' || key === 'F8') { // ESC / Voltar / F8
      this.open = false;
      return true;
    }
    return true;
  }

  handleTouch(x, y, type) {
    if (!this.open) return false;
    if (type === 0) {
      // Botão fechar [X] (canto superior direito)
      if (x >= 555 && x <= 575 && y >= 50 && y <= 70) {
        this.open = false;
        return true;
      }

      // Botões Resgatar / Reivindicar para os 3 contratos
      const slot = this.currentSlot();
      const list = this.getContracts(slot);
      for (let i = 0; i < list.length && i < 3; ++i) {
        const c = list[i];
        const cardY = 85 + i * 62;
        // Botão Resgatar (x: 485 a 565, y: cardY + 24 a cardY + 44)
        if (x >= 485 && x <= 565 && y >= cardY + 24 && y <= cardY + 44) {
          if (c.completed && !c.claimed) {
            const reward = this.claim(slot, c.id);
            if (reward && this.engine?.exports) {
              const g = this.engine.exports.get("Game");
              if (g) {
                const curGold = this.engine.get32(g + 0x4338);
                this.engine.set32(g + 0x4338, curGold + reward.gold);
              }
            }
          }
          return true;
        }
      }
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

    // Painel principal (360x245 centrado em x=220, y=45)
    fillRect(220, 45, 360, 245, 20, 24, 32);
    // Borda Dourada avermelhada
    for (let x = 220; x < 580; ++x) { setPixel(x, 45, 240, 140, 40); setPixel(x, 289, 240, 140, 40); }
    for (let y = 45; y < 290; ++y) { setPixel(220, y, 240, 140, 40); setPixel(579, y, 240, 140, 40); }

    // Botão Fechar [X]
    fillRect(555, 48, 20, 18, 160, 40, 40);

    const slot = this.currentSlot();
    const list = this.getContracts(slot);

    // 3 Cards de Contrato
    for (let i = 0; i < list.length && i < 3; ++i) {
      const c = list[i];
      const cardY = 75 + i * 65;

      fillRect(230, cardY, 340, 58, 16, 20, 28);

      // Barra de progresso do contrato
      const pct = Math.min(100, Math.floor((c.currentCount / Math.max(1, c.targetCount)) * 100));
      fillRect(240, cardY + 36, 220, 6, 12, 14, 18);
      const fillW = Math.floor((220 * pct) / 100);
      if (fillW > 0) {
        fillRect(240, cardY + 36, fillW, 6, c.completed ? 50 : 220, c.completed ? 200 : 160, c.completed ? 80 : 40);
      }

      // Botão Resgatar / Concluído
      const btnX = 475;
      const btnY = cardY + 28;
      if (c.claimed) {
        fillRect(btnX, btnY, 85, 20, 40, 45, 50); // Já resgatado (cinza)
      } else if (c.completed) {
        fillRect(btnX, btnY, 85, 20, 40, 180, 60); // Concluído / Resgatar (verde brilhante)
      } else {
        fillRect(btnX, btnY, 85, 20, 50, 55, 65); // Em andamento
      }
    }
  }
}
