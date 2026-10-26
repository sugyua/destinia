// Baú Compartilhado Web (0.11.0)
// Permite transferir equipamentos e consumíveis entre personagens de forma segura.

export const MAX_STASH_CAPACITY = 48; // 2 páginas de 24 slots

export class SharedStashWeb {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.page = 0; // 0 ou 1
    this.selectedIndex = 0;
    this.items = [];
    this.load();
  }

  load() {
    try {
      if (typeof localStorage !== 'undefined') {
        const raw = localStorage.getItem('destinia_shared_stash_v1');
        if (raw) {
          this.deserialize(raw);
        }
      }
    } catch (_) {}
  }

  save() {
    try {
      if (typeof localStorage !== 'undefined') {
        localStorage.setItem('destinia_shared_stash_v1', this.serialize());
      }
    } catch (_) {}
  }

  serialize() {
    const lines = ["shared_stash_version 1", `count ${this.items.length}`];
    for (const it of this.items) {
      lines.push(`item ${it.pool} ${it.count} ${it.durability} ${it.itemType} ${it.rank} ${it.level} ${it.jobClass} ${it.name}`);
    }
    return lines.join('\n');
  }

  deserialize(text) {
    const lines = text.split('\n');
    this.items = [];
    let valid = false;
    for (let line of lines) {
      line = line.trim();
      if (!line || line.startsWith('#')) continue;
      const parts = line.split(/\s+/);
      if (parts[0] === 'shared_stash_version' && parts[1] === '1') {
        valid = true;
      } else if (valid && parts[0] === 'item' && parts.length >= 8) {
        if (this.items.length < MAX_STASH_CAPACITY) {
          const name = parts.slice(8).join(' ') || 'Item do Baú';
          this.items.push({
            pool: parseInt(parts[1], 10) || 0,
            count: parseInt(parts[2], 10) || 1,
            durability: parseInt(parts[3], 10) || 100,
            itemType: parseInt(parts[4], 10) || 0,
            rank: parseInt(parts[5], 10) || 0,
            level: parseInt(parts[6], 10) || 1,
            jobClass: parseInt(parts[7], 10) || 0,
            name
          });
        }
      }
    }
  }

  isFull() {
    return this.items.length >= MAX_STASH_CAPACITY;
  }

  deposit(item) {
    if (this.isFull() || !item) return false;
    this.items.push({
      pool: item.pool || 0,
      count: item.count || 1,
      durability: item.durability || 100,
      itemType: item.itemType || 0,
      rank: item.rank || 0,
      level: item.level || 1,
      jobClass: item.jobClass || 0,
      name: item.name || 'Item do Baú'
    });
    this.save();
    return true;
  }

  withdraw(index) {
    if (index < 0 || index >= this.items.length) return null;
    const item = this.items.splice(index, 1)[0];
    this.save();
    return item;
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

    if (key === 27 || key === 10 || key === 'b' || key === 'B' || key === 'F5') { // ESC / Voltar / F5
      this.open = false;
      return true;
    }
    if (key === 'p' || key === 'P' || key === '\t') { // Alternar página
      this.page = this.page === 0 ? 1 : 0;
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
      // Botão Página 1 / Página 2 (x: 230 a 330, y: 70 a 90)
      if (x >= 230 && x <= 330 && y >= 70 && y <= 90) {
        this.page = 0;
        return true;
      }
      if (x >= 340 && x <= 440 && y >= 70 && y <= 90) {
        this.page = 1;
        return true;
      }

      // Grid de slots: 6 colunas x 4 linhas = 24 slots por página
      // Começa em x: 230, y: 95; cada slot tem 45x35 px com espaçamento de 5px
      for (let row = 0; row < 4; ++row) {
        for (let col = 0; col < 6; ++col) {
          const slotX = 230 + col * 55;
          const slotY = 95 + row * 40;
          if (x >= slotX && x <= slotX + 50 && y >= slotY && y <= slotY + 36) {
            const slotIdx = this.page * 24 + (row * 6 + col);
            this.selectedIndex = slotIdx;
            return true;
          }
        }
      }

      // Botão Retirar (x: 230 a 330, y: 260 a 285)
      if (x >= 230 && x <= 330 && y >= 260 && y <= 285) {
        if (this.selectedIndex < this.items.length) {
          this.withdraw(this.selectedIndex);
        }
        return true;
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
    // Borda Ciano / Dourada
    for (let x = 220; x < 580; ++x) { setPixel(x, 45, 40, 180, 220); setPixel(x, 289, 40, 180, 220); }
    for (let y = 45; y < 290; ++y) { setPixel(220, y, 40, 180, 220); setPixel(579, y, 40, 180, 220); }

    // Botão Fechar [X]
    fillRect(555, 48, 20, 18, 160, 40, 40);

    // Abas de Página (Página 1 / Página 2)
    fillRect(230, 70, 90, 18, this.page === 0 ? 40 : 25, this.page === 0 ? 100 : 35, this.page === 0 ? 140 : 45);
    fillRect(330, 70, 90, 18, this.page === 1 ? 40 : 25, this.page === 1 ? 100 : 35, this.page === 1 ? 140 : 45);

    // Grid de slots (6 colunas x 4 linhas)
    const rarityColors = [
      [200, 200, 200], // 0: Comum
      [40, 200, 60],   // 1: Mágico
      [50, 140, 240],  // 2: Raro
      [240, 210, 40],  // 3: Épico
      [240, 120, 20]   // 4: Lendário
    ];

    for (let row = 0; row < 4; ++row) {
      for (let col = 0; col < 6; ++col) {
        const slotIdx = this.page * 24 + (row * 6 + col);
        const slotX = 230 + col * 56;
        const slotY = 95 + row * 40;
        const isSelected = (slotIdx === this.selectedIndex);

        fillRect(slotX, slotY, 50, 36, isSelected ? 45 : 16, isSelected ? 50 : 20, isSelected ? 65 : 28);

        if (slotIdx < this.items.length) {
          const item = this.items[slotIdx];
          const colR = rarityColors[item.rank] || rarityColors[0];
          // Borda interna indicando raridade
          fillRect(slotX + 2, slotY + 2, 46, 32, colR[0] >> 1, colR[1] >> 1, colR[2] >> 1);
          fillRect(slotX + 4, slotY + 4, 42, 28, 20, 24, 32);
          // Ícone central
          fillRect(slotX + 16, slotY + 10, 18, 16, colR[0], colR[1], colR[2]);
        }
      }
    }

    // Botão Retirar no Rodapé
    fillRect(230, 260, 100, 22, 35, 90, 50);
  }
}
