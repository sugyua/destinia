// Catálogo de Armas e Complecionista Web (0.11.0)
export const CatalogWeapons = [
  // Guerreiro (1)
  { id: 1001, level: 1,   job: 1, name: "Espada de Madeira",     type: "Espada Curta" },
  { id: 1002, level: 10,  job: 1, name: "Espada de Ferro",       type: "Espada Longa" },
  { id: 1003, level: 20,  job: 1, name: "Espada Larga",          type: "Montante" },
  { id: 1004, level: 35,  job: 1, name: "Lâmina Bastarda",       type: "Espada Pesada" },
  { id: 1005, level: 50,  job: 1, name: "Gládio de Aço",         type: "Gládio" },
  { id: 1006, level: 65,  job: 1, name: "Montante Real",         type: "Montante" },
  { id: 1007, level: 80,  job: 1, name: "Destruidora Dracônica", type: "Lâmina Sagrada" },
  { id: 1008, level: 100, job: 1, name: "Lâmina do Destino",     type: "Arma Lendária" },

  // Atirador (2)
  { id: 2001, level: 1,   job: 2, name: "Pistola de Treino",     type: "Pistola" },
  { id: 2002, level: 10,  job: 2, name: "Revólver de Aço",       type: "Revólver" },
  { id: 2003, level: 20,  job: 2, name: "Pistolas Duplas",       type: "Arma Dupla" },
  { id: 2004, level: 35,  job: 2, name: "Carabina de Caça",      type: "Carabina" },
  { id: 2005, level: 50,  job: 2, name: "Fuzil de Precisão",     type: "Fuzil Sniper" },
  { id: 2006, level: 65,  job: 2, name: "Canhão Portátil",       type: "Canhão Pesado" },
  { id: 2007, level: 80,  job: 2, name: "Canhão do Trovão",      type: "Arma Elétrica" },
  { id: 2008, level: 100, job: 2, name: "Juízo Final",           type: "Arma Lendária" },

  // Ladino (3)
  { id: 3001, level: 1,   job: 3, name: "Adaga Gasta",           type: "Adaga Básica" },
  { id: 3002, level: 10,  job: 3, name: "Adaga de Aço",          type: "Adaga Afiada" },
  { id: 3003, level: 20,  job: 3, name: "Estilete Veloz",        type: "Estilete" },
  { id: 3004, level: 35,  job: 3, name: "Garras Sombrias",       type: "Garras Duplas" },
  { id: 3005, level: 50,  job: 3, name: "Lâminas Gêmeas",        type: "Adagas Gêmeas" },
  { id: 3006, level: 65,  job: 3, name: "Adaga Espectral",       type: "Lâmina Espiritual" },
  { id: 3007, level: 80,  job: 3, name: "Presa da Noite",        type: "Adaga Tóxica" },
  { id: 3008, level: 100, job: 3, name: "Beijo da Morte",        type: "Arma Lendária" }
];

export class WeaponsCatalogWeb {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.selectedJob = 1; // 1: Guerreiro, 2: Atirador, 3: Ladino
    this.page = 0;
    this.discovered = new Set(); // formato: "id:rarity"
  }

  record(id, rarity) {
    this.discovered.add(`${id}:${rarity}`);
  }

  isRecorded(id, rarity) {
    return this.discovered.has(`${id}:${rarity}`);
  }

  getWeaponsForJob(job) {
    return CatalogWeapons.filter(w => w.job === job);
  }

  getCompletionPercentage(job) {
    const list = this.getWeaponsForJob(job);
    if (!list.length) return 100;
    let found = 0;
    for (const w of list) {
      for (let r = 0; r < 5; ++r) {
        if (this.isRecorded(w.id, r)) found++;
      }
    }
    return Math.min(100, (found / (list.length * 5)) * 100);
  }

  scanInventory() {
    if (!this.engine?.exports) return;
    const g = this.engine.exports.get("Game");
    if (!g) return;

    for (let b = 0; b < 4; ++b) {
      const count = this.engine.get16(g + 0x433c + b * 0x92);
      if (count > 24) continue;
      for (let i = 0; i < count; ++i) {
        const record = g + 0x433e + b * 0x92 + 6 * i;
        if (!this.engine.get16(record + 4) || this.engine.get16(record) !== 0) continue;
        const pool = this.engine.get16(record + 2);
        if (pool >= 200) continue;

        const header = this.engine.get16(g + 0x2ed6 + 20 * pool);
        const itemType = (header >> 5) & 7;
        if (itemType !== 0) continue; // 0 = Arma

        const rank = (header >> 8) & 7; // Raridade 0..4
        if (rank > 4) continue;

        const minLv = header & 0x1f;
        for (const w of CatalogWeapons) {
          if (w.level <= minLv) {
            this.record(w.id, rank);
          }
        }
      }
    }
  }

  toggle() {
    this.open = !this.open;
    if (this.open) {
      this.scanInventory();
    }
    return this.open;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    if (key === 27 || key === 10 || key === 'w' || key === 'W') { // ESC / Voltar
      this.open = false;
      return true;
    }
    if (key === 3 || key === 5) { // Anterior
      if (this.page > 0) this.page--;
      return true;
    }
    if (key === 9 || key === 7) { // Próximo
      const list = this.getWeaponsForJob(this.selectedJob);
      const totalPages = Math.ceil(list.length / 4);
      if (this.page + 1 < totalPages) this.page++;
      return true;
    }
    if (key === 1 || key === '1') { this.selectedJob = 1; this.page = 0; return true; }
    if (key === 2 || key === '2') { this.selectedJob = 2; this.page = 0; return true; }
    if (key === 3 || key === '3') { this.selectedJob = 3; this.page = 0; return true; }
    if (key === 9 || key === '\t') { // TAB
      this.selectedJob = (this.selectedJob % 3) + 1;
      this.page = 0;
      return true;
    }
    return true;
  }

  handleTouch(x, y, type) {
    if (!this.open) return false;
    if (type === 0) {
      // Abas de classe (x: 230 a 570, y: 75 a 95)
      for (let i = 0; i < 3; ++i) {
        const tx = 230 + i * 115;
        if (x >= tx && x <= tx + 110 && y >= 75 && y <= 95) {
          this.selectedJob = i + 1;
          this.page = 0;
          return true;
        }
      }
      // Botões anterior/próximo
      if (x >= 230 && x <= 310 && y >= 250 && y <= 275) {
        if (this.page > 0) this.page--;
        return true;
      }
      if (x >= 490 && x <= 570 && y >= 250 && y <= 275) {
        const list = this.getWeaponsForJob(this.selectedJob);
        const totalPages = Math.ceil(list.length / 4);
        if (this.page + 1 < totalPages) this.page++;
        return true;
      }
      // Botão fechar [X] (canto superior direito)
      if (x >= 555 && x <= 575 && y >= 50 && y <= 70) {
        this.open = false;
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

    // Painel principal (360x240 centrado)
    fillRect(220, 45, 360, 245, 20, 24, 32);
    // Borda Dourada
    for (let x = 220; x < 580; ++x) { setPixel(x, 45, 220, 180, 50); setPixel(x, 289, 220, 180, 50); }
    for (let y = 45; y < 290; ++y) { setPixel(220, y, 220, 180, 50); setPixel(579, y, 220, 180, 50); }

    // Botão Fechar [X]
    fillRect(555, 48, 20, 18, 160, 40, 40);

    // Barra de Progresso Superior
    const pct = this.getCompletionPercentage(this.selectedJob);
    fillRect(230, 68, 340, 4, 15, 18, 24);
    const fillW = Math.floor((340 * pct) / 100);
    if (fillW > 0) {
      fillRect(230, 68, fillW, 4, 220, 180, 50);
    }

    // Abas de Classes
    for (let i = 0; i < 3; ++i) {
      const tx = 230 + i * 115;
      const active = (this.selectedJob === (i + 1));
      fillRect(tx, 76, 110, 18, active ? 45 : 25, active ? 70 : 30, active ? 105 : 40);
    }

    // Cards das Armas na Página (4 por página)
    const list = this.getWeaponsForJob(this.selectedJob);
    const startIdx = this.page * 4;
    const pageItems = list.slice(startIdx, startIdx + 4);

    const rarityColors = [
      [200, 200, 200], // 0: Comum (Branca)
      [40, 200, 60],   // 1: Mágica (Verde)
      [50, 140, 240],  // 2: Rara (Azul)
      [240, 210, 40],  // 3: Épica (Amarela)
      [240, 120, 20]   // 4: Lendária (Laranja)
    ];

    let cardY = 98;
    for (const w of pageItems) {
      fillRect(230, cardY, 340, 32, 16, 20, 28);

      // 5 badges de raridade à direita
      for (let r = 0; r < 5; ++r) {
        const bx = 450 + r * 23;
        const by = cardY + 8;
        const collected = this.isRecorded(w.id, r);
        if (collected) {
          const col = rarityColors[r];
          fillRect(bx, by, 18, 16, col[0], col[1], col[2]);
        } else {
          fillRect(bx, by, 18, 16, 40, 44, 52); // Silhueta apagada
        }
      }

      cardY += 36;
    }

    // Rodapé de Navegação
    fillRect(230, 252, 70, 20, 25, 30, 40);
    fillRect(500, 252, 70, 20, 25, 30, 40);
  }
}
