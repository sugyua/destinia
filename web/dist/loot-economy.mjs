// Meta 0.10 - Frente 2: Sistemas de Loot e Economia
// Drops de Boss exclusivos, Sets de Equipamento e Cash Offline

export const MAX_OFFLINE_SECONDS = 86400; // 24 Horas Estritas (60*60*24)
export const DEFAULT_OFFLINE_CASH_INTERVAL = 1800; // 1 Cash a cada 30 min (máx 48 Cash em 24h)

export const bossDropCatalog = [
  {
    species: 100,
    bossName: 'Dungeon Guardian',
    itemId: 'boss_drop_100',
    namePt: 'Coracao de Pedra Ancestral',
    nameEn: 'Ancient Stone Heart',
    descPt: 'Reliquia fossilizada do Guardiao das Profundezas. Concede grande resiliencia fisica.',
    descEn: 'Fossilized relic of the Deep Guardian. Grants immense physical resilience.',
    rarity: 4,
    hpBonus: 150,
    defBonus: 20,
    vitBonus: 15
  },
  {
    species: 101,
    bossName: 'Defcan',
    itemId: 'boss_drop_101',
    namePt: 'Manto das Sombras de Defcan',
    nameEn: "Defcan's Shadow Shroud",
    descPt: 'Manto etereo tecido com as sombras de Defcan. Aumenta a agilidade e a evasao.',
    descEn: "Ethereal shroud woven with Defcan's shadows. Enhances agility and evasion.",
    rarity: 4,
    defBonus: 10,
    dexBonus: 25,
    evasionBonus: 15
  },
  {
    species: 102,
    bossName: 'Salas',
    itemId: 'boss_drop_102',
    namePt: 'Presa Venenosa de Salas',
    nameEn: 'Poison Fang of Salas',
    descPt: 'Presa peconhenta retirada de Salas. Causa dano agudo e amplia golpes criticos.',
    descEn: 'Venomous fang harvested from Salas. Inflicts sharp strikes and boosts critical chance.',
    rarity: 4,
    atkBonus: 30,
    dexBonus: 10,
    critBonus: 5
  },
  {
    species: 103,
    bossName: 'Defcan Desperto',
    itemId: 'boss_drop_103',
    namePt: 'Olho do Vazio de Defcan',
    nameEn: "Defcan's Void Eye",
    descPt: 'Fragmento ocular imbuido com energias dimensionais do Defcan Desperto.',
    descEn: 'Ocular fragment infused with dimensional energy from Awakened Defcan.',
    rarity: 4,
    mpBonus: 150,
    intBonus: 40
  },
  {
    species: 104,
    bossName: 'Sileth',
    itemId: 'boss_drop_104',
    namePt: 'Lamina Espectral de Sileth',
    nameEn: 'Spectral Blade of Sileth',
    descPt: 'Fragmento de aco espectral de Sileth. Amplifica a potencia dos danos criticos.',
    descEn: 'Spectral steel fragment of Sileth. Enhances critical damage potency.',
    rarity: 4,
    atkBonus: 35,
    dexBonus: 15,
    critDmgBonus: 10
  },
  {
    species: 105,
    bossName: 'Berserk Tamur',
    itemId: 'boss_drop_105',
    namePt: 'Nucleo de Furia de Tamur',
    nameEn: "Tamur's Rage Core",
    descPt: 'O coracao pulsante de Tamur em frenesi de sangue. Converte dano em vitalidade.',
    descEn: 'The pulsing heart of berserk Tamur. Converts inflicted damage into vitality.',
    rarity: 4,
    hpBonus: 100,
    atkBonus: 25,
    strBonus: 40,
    lifeStealBonus: 10
  },
  {
    species: 106,
    bossName: 'Kaiser',
    itemId: 'boss_drop_106',
    namePt: 'Coroa Imperial de Kaiser',
    nameEn: "Kaiser's Imperial Crown",
    descPt: 'A diadema dourada do imperador Kaiser. Fortalece todos os atributos vitais.',
    descEn: 'The golden diadem of Emperor Kaiser. Enhances all vital attributes.',
    rarity: 4,
    hpBonus: 200,
    mpBonus: 50,
    atkBonus: 20,
    defBonus: 20,
    strBonus: 25,
    dexBonus: 25,
    vitBonus: 25,
    intBonus: 25
  },
  {
    species: 107,
    bossName: 'Baramos',
    itemId: 'boss_drop_107',
    namePt: 'Chifre Demoniaco de Baramos',
    nameEn: "Baramos's Demon Horn",
    descPt: 'Chifre calcinado de Baramos forjado nas profundezas do inferno.',
    descEn: 'Calcined horn of Baramos forged in the deep underworld.',
    rarity: 4,
    atkBonus: 50,
    defBonus: 30,
    strBonus: 20,
    vitBonus: 20
  },
  {
    species: 108,
    bossName: 'Diablo',
    itemId: 'boss_drop_108',
    namePt: 'Alma Ardente de Diablo',
    nameEn: "Diablo's Burning Soul",
    descPt: 'A chama eterna do demonio primordial Diablo. Poder destrutivo sem igual.',
    descEn: 'The eternal flame of primeval demon Diablo. Unmatched destructive might.',
    rarity: 4,
    hpBonus: 200,
    mpBonus: 100,
    atkBonus: 60,
    strBonus: 30,
    intBonus: 30,
    critBonus: 5,
    critDmgBonus: 10
  },
  {
    species: 109,
    bossName: 'Lava Guardian',
    itemId: 'boss_drop_109',
    namePt: 'Magma Primordial do Guardiao',
    nameEn: "Guardian's Primeval Magma",
    descPt: 'Nucleo de pedra fundida do Guardiao de Lava. Defesa e vitalidade inabalaveis.',
    descEn: 'Molten stone core of the Lava Guardian. Unshakable defense and vitality.',
    rarity: 4,
    hpBonus: 250,
    defBonus: 45,
    vitBonus: 20
  }
];

export function findBossDrop(species) {
  return bossDropCatalog.find(b => b.species === species) || null;
}

export const equipmentSetCatalog = [
  {
    id: 'dragon',
    namePt: 'Set do Dragao',
    nameEn: 'Dragon Set',
    patterns: ['dragao', 'dragon'],
    bonus2: {
      piecesRequired: 2,
      namePt: 'Furia Draconica Menor',
      nameEn: 'Lesser Dragon Fury',
      descPt: '+25 Ataque, +15 Forca',
      descEn: '+25 Attack, +15 Strength',
      atk: 25,
      str: 15
    },
    bonus4: {
      piecesRequired: 4,
      namePt: 'Poder Draconico Ancestral',
      nameEn: 'Ancient Dragon Power',
      descPt: '+60 Ataque, +30 Forca, +10% Dano Critico, +200 HP',
      descEn: '+60 Attack, +30 Strength, +10% Crit Damage, +200 HP',
      atk: 60,
      str: 30,
      critDmg: 10,
      hp: 200
    }
  },
  {
    id: 'shadow',
    namePt: 'Set Sombrio',
    nameEn: 'Shadow Set',
    patterns: ['sombri', 'shadow', 'sombra'],
    bonus2: {
      piecesRequired: 2,
      namePt: 'Manto Noturno',
      nameEn: 'Night Shroud',
      descPt: '+20 Evasao, +15 Destreza',
      descEn: '+20 Evasion, +15 Dexterity',
      evasion: 20,
      dex: 15
    },
    bonus4: {
      piecesRequired: 4,
      namePt: 'Mestre das Sombras',
      nameEn: 'Shadow Master',
      descPt: '+40 Evasao, +30 Destreza, +12% Critico, +10% Vampirismo',
      descEn: '+40 Evasion, +30 Dexterity, +12% Crit, +10% Lifesteal',
      evasion: 40,
      dex: 30,
      critChance: 12,
      lifeSteal: 10
    }
  },
  {
    id: 'imperial',
    namePt: 'Set Imperial',
    nameEn: 'Imperial Set',
    patterns: ['imperial', 'kaiser'],
    bonus2: {
      piecesRequired: 2,
      namePt: 'Vontade de Ferro',
      nameEn: 'Iron Will',
      descPt: '+30 Defesa, +15 Vitalidade',
      descEn: '+30 Defense, +15 Vitality',
      def: 30,
      vit: 15
    },
    bonus4: {
      piecesRequired: 4,
      namePt: 'Soberania Absoluta',
      nameEn: 'Absolute Sovereignty',
      descPt: '+70 Defesa, +35 Vitalidade, +350 HP, +15% Reducao de Dano',
      descEn: '+70 Defense, +35 Vitality, +350 HP, +15% Damage Reduction',
      def: 70,
      vit: 35,
      hp: 350
    }
  },
  {
    id: 'arcane',
    namePt: 'Set Arcano',
    nameEn: 'Arcane Set',
    patterns: ['arcan', 'arcane', 'mago', 'mage'],
    bonus2: {
      piecesRequired: 2,
      namePt: 'Fluxo Etereo',
      nameEn: 'Ethereal Flow',
      descPt: '+30 Inteligencia, +120 MP',
      descEn: '+30 Intelligence, +120 MP',
      intStat: 30,
      mp: 120
    },
    bonus4: {
      piecesRequired: 4,
      namePt: 'Conhecimento Primordial',
      nameEn: 'Primordial Knowledge',
      descPt: '+65 Inteligencia, +250 MP, -20% Custo MP',
      descEn: '+65 Intelligence, +250 MP, -20% MP Cost',
      intStat: 65,
      mp: 250,
      mpCostReduction: 20
    }
  },
  {
    id: 'guardian',
    namePt: 'Set do Guardiao',
    nameEn: 'Guardian Set',
    patterns: ['guardiao', 'guardian'],
    bonus2: {
      piecesRequired: 2,
      namePt: 'Bastiao Protetor',
      nameEn: 'Protective Bastion',
      descPt: '+25 Defesa, +150 HP',
      descEn: '+25 Defense, +150 HP',
      def: 25,
      hp: 150
    },
    bonus4: {
      piecesRequired: 4,
      namePt: 'Fortaleza Viva',
      nameEn: 'Living Fortress',
      descPt: '+60 Defesa, +300 HP, +20% Resistencia',
      descEn: '+60 Defense, +300 HP, +20% Resistance',
      def: 60,
      hp: 300
    }
  }
];

export function identifySet(itemName) {
  if (!itemName || typeof itemName !== 'string') return -1;
  const lower = itemName.toLowerCase();
  for (let i = 0; i < equipmentSetCatalog.length; i++) {
    for (const pattern of equipmentSetCatalog[i].patterns) {
      if (lower.includes(pattern)) return i;
    }
  }
  return -1;
}

export function evaluateActiveSets(equippedNames) {
  if (!Array.isArray(equippedNames)) return [];
  const counts = new Array(equipmentSetCatalog.length).fill(0);
  for (const name of equippedNames) {
    const idx = identifySet(name);
    if (idx >= 0) counts[idx]++;
  }
  const summaries = [];
  for (let i = 0; i < equipmentSetCatalog.length; i++) {
    const count = counts[i];
    if (count >= 2) {
      const set = equipmentSetCatalog[i];
      const activeBonuses = [set.bonus2];
      let bonus4Active = false;
      if (count >= 4) {
        bonus4Active = true;
        activeBonuses.push(set.bonus4);
      }
      summaries.push({
        setId: set.id,
        setNamePt: set.namePt,
        setNameEn: set.nameEn,
        piecesEquipped: count,
        bonus2Active: true,
        bonus4Active,
        activeBonuses
      });
    }
  }
  return summaries;
}

export function calculateOfflineCash(lastTimestamp, currentTimestamp, intervalSeconds = DEFAULT_OFFLINE_CASH_INTERVAL) {
  if (!lastTimestamp || lastTimestamp <= 0 || !currentTimestamp || currentTimestamp <= lastTimestamp || intervalSeconds <= 0) {
    return { cashToCredit: 0, elapsedSeconds: 0, cappedSeconds: 0, capped: false };
  }
  const elapsedSeconds = currentTimestamp - lastTimestamp;
  const capped = elapsedSeconds > MAX_OFFLINE_SECONDS;
  const cappedSeconds = Math.min(elapsedSeconds, MAX_OFFLINE_SECONDS);
  const cashToCredit = Math.floor(cappedSeconds / intervalSeconds);
  return { cashToCredit, elapsedSeconds, cappedSeconds, capped };
}

export class LootEconomy {
  constructor() {
    this.characterSlot = 0;
    this.lastTimestamp = 0;
    this.totalOfflineCashAwarded = 0;
    this.bossDrops = new Map(); // `${slot}:${species}` -> count
  }

  select(slot) {
    if (slot < 0 || slot > 9) throw Error('PERSONAGEM INVALIDO');
    this.characterSlot = slot;
  }

  recordBossKill(slot, species) {
    if (slot < 0 || slot > 9) throw Error('PERSONAGEM INVALIDO');
    if (species < 100 || species > 109) return;
    const key = `${slot}:${species}`;
    this.bossDrops.set(key, (this.bossDrops.get(key) || 0) + 1);
  }

  getBossDropCount(slot, species) {
    return this.bossDrops.get(`${slot}:${species}`) || 0;
  }

  processOfflineCash(nowTimestamp, intervalSeconds = DEFAULT_OFFLINE_CASH_INTERVAL) {
    const res = calculateOfflineCash(this.lastTimestamp, nowTimestamp, intervalSeconds);
    if (res.cashToCredit > 0) {
      this.totalOfflineCashAwarded += res.cashToCredit;
    }
    this.lastTimestamp = nowTimestamp;
    return res;
  }

  encode() {
    let out = 'DESTINIA_ECONOMY_1\n';
    out += `timestamp ${this.lastTimestamp}\n`;
    out += `total_cash ${this.totalOfflineCashAwarded}\n`;
    for (const [key, count] of this.bossDrops.entries()) {
      const [slot, species] = key.split(':');
      out += `drop ${slot} ${species} ${count}\n`;
    }
    return out;
  }

  decode(text) {
    if (!text || typeof text !== 'string' || text.length > 100000 || !text.endsWith('\n')) {
      throw Error('SIDECAR INVALIDO');
    }
    const lines = text.split(/\r?\n/);
    if (lines[0] !== 'DESTINIA_ECONOMY_1') {
      throw Error('FORMATO DE ECONOMIA DESCONHECIDO');
    }
    let nextTimestamp = 0;
    let nextCash = 0;
    const nextDrops = new Map();
    for (let i = 1; i < lines.length; i++) {
      const line = lines[i].trim();
      if (!line) continue;
      const parts = line.split(/\s+/);
      const tag = parts[0];
      if (tag === 'timestamp') {
        nextTimestamp = parseInt(parts[1], 10) || 0;
      } else if (tag === 'total_cash') {
        nextCash = parseInt(parts[1], 10) || 0;
      } else if (tag === 'drop') {
        const slot = parseInt(parts[1], 10);
        const species = parseInt(parts[2], 10);
        const count = parseInt(parts[3], 10);
        if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
          nextDrops.set(`${slot}:${species}`, count);
        }
      }
    }
    this.lastTimestamp = nextTimestamp;
    this.totalOfflineCashAwarded = nextCash;
    this.bossDrops = nextDrops;
  }
}
