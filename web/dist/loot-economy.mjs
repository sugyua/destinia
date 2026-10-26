// Meta 0.10 - Frente 2: Sistemas de Loot e Economia
// Drops de Boss exclusivos, Sets de Equipamento (2..6 peças), Origens de Drop e Economia de Cash por Missões

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

export const SetDropSourceType = Object.freeze({
  Concentrated: 'Concentrated',
  Distributed: 'Distributed'
});

export const equipmentSetCatalog = [
  {
    id: 'dragon',
    namePt: 'Set do Dragao',
    nameEn: 'Dragon Set',
    patterns: ['dragao', 'dragon'],
    dropType: 'Concentrated',
    primaryBoss: 108,
    bossSources: [108, 109],
    eliteSource: false,
    farmRoutePt: 'Drop Concentrado: As 6 pecas caem exclusivamente de Diablo e Lava Guardian.',
    farmRouteEn: 'Concentrated Drop: All 6 pieces drop exclusively from Diablo and Lava Guardian.',
    bonus2: { piecesRequired: 2, namePt: 'Garras Draconicas', nameEn: 'Dragon Claws', descPt: '+30 Ataque, +20 Forca', descEn: '+30 Attack, +20 Strength', atk: 30, str: 20 },
    bonus3: { piecesRequired: 3, namePt: 'Escamas de Dragao', nameEn: 'Dragon Scales', descPt: '+35 Defesa, +150 HP', descEn: '+35 Defense, +150 HP', def: 35, hp: 150 },
    bonus4: { piecesRequired: 4, namePt: 'Coracao Draconico', nameEn: 'Dragon Heart', descPt: '+300 HP, +15% Dano Critico', descEn: '+300 HP, +15% Crit Damage', hp: 300, critDmg: 15 },
    bonus5: { piecesRequired: 5, namePt: 'Olhar do Dragao', nameEn: 'Dragon Gaze', descPt: '+10% Chance Critica, +35 Forca', descEn: '+10% Crit Chance, +35 Strength', str: 35, critChance: 10 },
    bonus6: { piecesRequired: 6, namePt: 'Furia Draconica Suprema', nameEn: 'Supreme Dragon Fury', descPt: '+100 Ataque, +50 Forca, +25% Dano Critico, +500 HP', descEn: '+100 Attack, +50 Strength, +25% Crit Damage, +500 HP', atk: 100, str: 50, critDmg: 25, hp: 500 }
  },
  {
    id: 'shadow',
    namePt: 'Set Sombrio',
    nameEn: 'Shadow Set',
    patterns: ['sombri', 'shadow', 'sombra'],
    dropType: 'Distributed',
    primaryBoss: -1,
    bossSources: [101, 102, 103, 104],
    eliteSource: true,
    farmRoutePt: 'Drop Distribuido: Pecas espalhadas entre Defcan, Salas, Sileth e Monstros Elites.',
    farmRouteEn: 'Distributed Drop: Pieces spread across Defcan, Salas, Sileth and Elite Monsters.',
    bonus2: { piecesRequired: 2, namePt: 'Passos das Sombras', nameEn: 'Shadow Steps', descPt: '+25 Evasao, +20 Destreza', descEn: '+25 Evasion, +20 Dexterity', evasion: 25, dex: 20 },
    bonus3: { piecesRequired: 3, namePt: 'Capa da Meia-Noite', nameEn: 'Midnight Cloak', descPt: '+25 Defesa, +15 Destreza', descEn: '+25 Defense, +15 Dexterity', def: 25, dex: 15 },
    bonus4: { piecesRequired: 4, namePt: 'Golpe Furtivo', nameEn: 'Stealth Strike', descPt: '+250 HP, +10% Chance Critica', descEn: '+250 HP, +10% Crit Chance', hp: 250, critChance: 10 },
    bonus5: { piecesRequired: 5, namePt: 'Lamina Oculta', nameEn: 'Hidden Blade', descPt: '+20% Dano Critico, +10% Vampirismo', descEn: '+20% Crit Damage, +10% Lifesteal', critDmg: 20, lifeSteal: 10 },
    bonus6: { piecesRequired: 6, namePt: 'Avatar das Sombras', nameEn: 'Shadow Avatar', descPt: '+60 Evasao, +50 Destreza, +15% Critico, +15% Vampirismo', descEn: '+60 Evasion, +50 Dexterity, +15% Crit, +15% Lifesteal', evasion: 60, dex: 50, critChance: 15, lifeSteal: 15 }
  },
  {
    id: 'imperial',
    namePt: 'Set Imperial',
    nameEn: 'Imperial Set',
    patterns: ['imperial', 'kaiser'],
    dropType: 'Distributed',
    primaryBoss: -1,
    bossSources: [106, 107],
    eliteSource: true,
    farmRoutePt: 'Drop Distribuido: Pecas obtidas de Kaiser, Baramos e Masmorras de Elite.',
    farmRouteEn: 'Distributed Drop: Pieces obtained from Kaiser, Baramos and Elite Dungeons.',
    bonus2: { piecesRequired: 2, namePt: 'Vontade Soberana', nameEn: 'Sovereign Will', descPt: '+35 Defesa, +20 Vitalidade', descEn: '+35 Defense, +20 Vitality', def: 35, vit: 20 },
    bonus3: { piecesRequired: 3, namePt: 'Guarda Imperial', nameEn: 'Imperial Guard', descPt: '+30 Defesa, +200 HP', descEn: '+30 Defense, +200 HP', def: 30, hp: 200 },
    bonus4: { piecesRequired: 4, namePt: 'Bastiao Imperial', nameEn: 'Imperial Bastion', descPt: '+400 HP, +15% Reducao de Dano', descEn: '+400 HP, +15% Damage Reduction', def: 40, hp: 400 },
    bonus5: { piecesRequired: 5, namePt: 'Linhagem Real', nameEn: 'Royal Lineage', descPt: '+35 Vitalidade, +20 Regeneracao de HP', descEn: '+35 Vitality, +20 HP Regen', vit: 35, hp: 200 },
    bonus6: { piecesRequired: 6, namePt: 'Soberania Absoluta', nameEn: 'Absolute Sovereignty', descPt: '+100 Defesa, +50 Vitalidade, +600 HP, +25% Reducao de Dano', descEn: '+100 Defense, +50 Vitality, +600 HP, +25% Damage Reduction', def: 100, vit: 50, hp: 600 }
  },
  {
    id: 'arcane',
    namePt: 'Set Arcano',
    nameEn: 'Arcane Set',
    patterns: ['arcan', 'arcane', 'mago', 'mage'],
    dropType: 'Distributed',
    primaryBoss: -1,
    bossSources: [103, 104],
    eliteSource: true,
    farmRoutePt: 'Drop Distribuido: Pecas dropadas por Defcan Desperto, Sileth e Elites Magicos.',
    farmRouteEn: 'Distributed Drop: Pieces dropped by Awakened Defcan, Sileth and Magic Elites.',
    bonus2: { piecesRequired: 2, namePt: 'Chama Arcana', nameEn: 'Arcane Flame', descPt: '+35 Inteligencia, +150 MP', descEn: '+35 Intelligence, +150 MP', intStat: 35, mp: 150 },
    bonus3: { piecesRequired: 3, namePt: 'Escudo Magico', nameEn: 'Magic Shield', descPt: '+25 Defesa, +150 MP', descEn: '+25 Defense, +150 MP', def: 25, mp: 150 },
    bonus4: { piecesRequired: 4, namePt: 'Mente Transcendente', nameEn: 'Transcendent Mind', descPt: '+350 MP, -20% Custo de MP', descEn: '+350 MP, -20% MP Cost', mp: 350, mpCostReduction: 20 },
    bonus5: { piecesRequired: 5, namePt: 'Foco Prismatico', nameEn: 'Prismatic Focus', descPt: '+12% Chance Critica Magica, +40 Inteligencia', descEn: '+12% Magic Crit Chance, +40 Intelligence', intStat: 40, critChance: 12 },
    bonus6: { piecesRequired: 6, namePt: 'Arquimago Supremo', nameEn: 'Supreme Archmage', descPt: '+80 Inteligencia, +500 MP, -35% Custo de MP em Habilidades', descEn: '+80 Intelligence, +500 MP, -35% MP Cost on Skills', intStat: 80, mp: 500, mpCostReduction: 35 }
  },
  {
    id: 'guardian',
    namePt: 'Set do Guardiao',
    nameEn: 'Guardian Set',
    patterns: ['guardiao', 'guardian'],
    dropType: 'Concentrated',
    primaryBoss: 100,
    bossSources: [100],
    eliteSource: false,
    farmRoutePt: 'Drop Concentrado: As 6 pecas caem 100% exclusivamente do Dungeon Guardian.',
    farmRouteEn: 'Concentrated Drop: All 6 pieces drop 100% exclusively from Dungeon Guardian.',
    bonus2: { piecesRequired: 2, namePt: 'Muralha de Pedra', nameEn: 'Stone Wall', descPt: '+35 Defesa, +200 HP', descEn: '+35 Defense, +200 HP', def: 35, hp: 200 },
    bonus3: { piecesRequired: 3, namePt: 'Escudo de Granito', nameEn: 'Granite Shield', descPt: '+30 Defesa, +20 Vitalidade', descEn: '+30 Defense, +20 Vitality', def: 30, vit: 20 },
    bonus4: { piecesRequired: 4, namePt: 'Corpo de Rocha', nameEn: 'Body of Rock', descPt: '+450 HP, +20% Resistencia a Atordoamento', descEn: '+450 HP, +20% Stun Resistance', hp: 450 },
    bonus5: { piecesRequired: 5, namePt: 'Forca da Terra', nameEn: 'Strength of Earth', descPt: '+50 Defesa, +25 Regeneracao de HP', descEn: '+50 Defense, +25 HP Regen', def: 50, hp: 150 },
    bonus6: { piecesRequired: 6, namePt: 'Colosso Inabalavel', nameEn: 'Unshakable Colossus', descPt: '+90 Defesa, +600 HP, +30% Resistencia Geral, +30 Regeneracao de HP', descEn: '+90 Defense, +600 HP, +30% Overall Resistance, +30 HP Regen', def: 90, hp: 600 }
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
      const activeBonuses = [];
      let bonus2Active = false, bonus3Active = false, bonus4Active = false, bonus5Active = false, bonus6Active = false;
      if (count >= 2) { bonus2Active = true; activeBonuses.push(set.bonus2); }
      if (count >= 3) { bonus3Active = true; activeBonuses.push(set.bonus3); }
      if (count >= 4) { bonus4Active = true; activeBonuses.push(set.bonus4); }
      if (count >= 5) { bonus5Active = true; activeBonuses.push(set.bonus5); }
      if (count >= 6) { bonus6Active = true; activeBonuses.push(set.bonus6); }
      summaries.push({
        setId: set.id,
        setNamePt: set.namePt,
        setNameEn: set.nameEn,
        dropType: set.dropType,
        farmRoutePt: set.farmRoutePt,
        farmRouteEn: set.farmRouteEn,
        piecesEquipped: count,
        bonus2Active,
        bonus3Active,
        bonus4Active,
        bonus5Active,
        bonus6Active,
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

export const epicMissionsCatalog = [
  {
    id: 'boss_slayer_10',
    titlePt: 'Matador de Chefes I',
    titleEn: 'Boss Slayer I',
    descPt: 'Derrote 10 chefes de qualquer masmorra.',
    descEn: 'Defeat 10 dungeon bosses.',
    targetType: 'BossKills',
    targetCount: 10,
    cashReward: 25
  },
  {
    id: 'boss_slayer_25',
    titlePt: 'Matador de Chefes II',
    titleEn: 'Boss Slayer II',
    descPt: 'Derrote 25 chefes em sua jornada.',
    descEn: 'Defeat 25 dungeon bosses.',
    targetType: 'BossKills',
    targetCount: 25,
    cashReward: 50
  },
  {
    id: 'boss_slayer_50',
    titlePt: 'Conquistador de Masmorras',
    titleEn: 'Dungeon Conqueror',
    descPt: 'Derrote 50 chefes lendarios.',
    descEn: 'Defeat 50 legendary bosses.',
    targetType: 'BossKills',
    targetCount: 50,
    cashReward: 75
  },
  {
    id: 'boss_slayer_100',
    titlePt: 'Lenda dos Chefes',
    titleEn: 'Boss Legend',
    descPt: 'Derrote 100 chefes supremos do jogo.',
    descEn: 'Defeat 100 supreme bosses of the game.',
    targetType: 'BossKills',
    targetCount: 100,
    cashReward: 100
  },
  {
    id: 'full_set_equipped',
    titlePt: 'Conjunto Perfeito',
    titleEn: 'Flawless Set',
    descPt: 'Equipe as 6 pecas de um conjunto de equipamento lendario.',
    descEn: 'Equip all 6 pieces of a legendary equipment set.',
    targetType: 'FullSetEquipped',
    targetCount: 6,
    cashReward: 50
  }
];

export class LootEconomy {
  constructor() {
    this.characterSlot = 0;
    this.lastTimestamp = 0;
    this.totalOfflineCashAwarded = 0;
    this.totalMissionCashAwarded = 0;
    this.bossDrops = new Map(); // `${slot}:${species}` -> count
    this.bossKills = new Map(); // `${slot}:${species}` -> count
    this.completedMissions = new Set(); // `${slot}:${missionId}`
  }

  select(slot) {
    if (slot < 0 || slot > 9) throw Error('PERSONAGEM INVALIDO');
    this.characterSlot = slot;
  }

  recordBossKill(slot, species) {
    if (slot < 0 || slot > 9) throw Error('PERSONAGEM INVALIDO');
    if (species < 100 || species > 109) return;
    const key = `${slot}:${species}`;
    this.bossKills.set(key, (this.bossKills.get(key) || 0) + 1);
    this.bossDrops.set(key, (this.bossDrops.get(key) || 0) + 1);
  }

  getBossDropCount(slot, species) {
    return this.bossDrops.get(`${slot}:${species}`) || 0;
  }

  getBossKillCount(slot, species) {
    return this.bossKills.get(`${slot}:${species}`) || 0;
  }

  getTotalBossKills(slot) {
    let total = 0;
    for (let sp = 100; sp <= 109; sp++) {
      total += this.bossKills.get(`${slot}:${sp}`) || 0;
    }
    return total;
  }

  checkAndClaimEpicMissions(slot, maxSetPiecesEquipped = 0) {
    let claimedCash = 0;
    const kills = this.getTotalBossKills(slot);
    for (const mission of epicMissionsCatalog) {
      const key = `${slot}:${mission.id}`;
      if (this.completedMissions.has(key)) continue;
      let completed = false;
      if (mission.targetType === 'BossKills') {
        completed = kills >= mission.targetCount;
      } else if (mission.targetType === 'FullSetEquipped') {
        completed = maxSetPiecesEquipped >= mission.targetCount;
      }
      if (completed) {
        this.completedMissions.add(key);
        claimedCash += mission.cashReward;
        this.totalMissionCashAwarded += mission.cashReward;
      }
    }
    return claimedCash;
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
    let out = 'DESTINIA_ECONOMY_2\n';
    out += `timestamp ${this.lastTimestamp}\n`;
    out += `total_offline_cash ${this.totalOfflineCashAwarded}\n`;
    out += `total_mission_cash ${this.totalMissionCashAwarded}\n`;
    for (const [key, count] of this.bossDrops.entries()) {
      const [slot, species] = key.split(':');
      out += `drop ${slot} ${species} ${count}\n`;
    }
    for (const [key, count] of this.bossKills.entries()) {
      const [slot, species] = key.split(':');
      out += `kill ${slot} ${species} ${count}\n`;
    }
    for (const key of this.completedMissions) {
      const [slot, missionId] = key.split(':');
      out += `mission ${slot} ${missionId}\n`;
    }
    return out;
  }

  decode(text) {
    if (!text || typeof text !== 'string' || text.length > 100000 || !text.endsWith('\n')) {
      throw Error('SIDECAR INVALIDO');
    }
    const lines = text.split(/\r?\n/);
    if (lines[0] !== 'DESTINIA_ECONOMY_1' && lines[0] !== 'DESTINIA_ECONOMY_2') {
      throw Error('FORMATO DE ECONOMIA DESCONHECIDO');
    }
    let nextTimestamp = 0;
    let nextOfflineCash = 0;
    let nextMissionCash = 0;
    const nextDrops = new Map();
    const nextKills = new Map();
    const nextMissions = new Set();

    for (let i = 1; i < lines.length; i++) {
      const line = lines[i].trim();
      if (!line) continue;
      const parts = line.split(/\s+/);
      const tag = parts[0];
      if (tag === 'timestamp') {
        nextTimestamp = parseInt(parts[1], 10) || 0;
      } else if (tag === 'total_cash' || tag === 'total_offline_cash') {
        nextOfflineCash = parseInt(parts[1], 10) || 0;
      } else if (tag === 'total_mission_cash') {
        nextMissionCash = parseInt(parts[1], 10) || 0;
      } else if (tag === 'drop') {
        const slot = parseInt(parts[1], 10);
        const species = parseInt(parts[2], 10);
        const count = parseInt(parts[3], 10);
        if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
          nextDrops.set(`${slot}:${species}`, count);
        }
      } else if (tag === 'kill') {
        const slot = parseInt(parts[1], 10);
        const species = parseInt(parts[2], 10);
        const count = parseInt(parts[3], 10);
        if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
          nextKills.set(`${slot}:${species}`, count);
        }
      } else if (tag === 'mission') {
        const slot = parseInt(parts[1], 10);
        const missionId = parts[2];
        if (slot >= 0 && slot <= 9 && missionId) {
          nextMissions.add(`${slot}:${missionId}`);
        }
      }
    }
    this.lastTimestamp = nextTimestamp;
    this.totalOfflineCashAwarded = nextOfflineCash;
    this.totalMissionCashAwarded = nextMissionCash;
    this.bossDrops = nextDrops;
    this.bossKills = nextKills;
    this.completedMissions = nextMissions;
  }
}
