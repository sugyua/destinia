// Destinia - Sistema de Títulos e Buffs (Frente 3)

export const TitleId = {
  Nenhum: 0,
  Iniciante: 1,
  Cacador: 2,
  Veterano: 3,
  MestreDoCombate: 4,
  MatadorDeBosses: 5,
  Complecionista: 6,
  SenhorDoAbismo: 7
};

export const ALL_TITLES = [
  { id: TitleId.Nenhum, nome: 'Nenhum', descricao: 'Nenhum título equipado.', atk: 0, def: 0, hp: 0, mp: 0 },
  { id: TitleId.Iniciante, nome: 'Iniciante', descricao: 'Primeiros passos no mundo de Destinia.', atk: 2, def: 2, hp: 5, mp: 5 },
  { id: TitleId.Cacador, nome: 'Caçador', descricao: 'Derrotou dezenas de inimigos comuns.', atk: 5, def: 3, hp: 5, mp: 5 },
  { id: TitleId.Veterano, nome: 'Veterano', descricao: 'Atingiu nível de prestígio em combate.', atk: 10, def: 8, hp: 10, mp: 10 },
  { id: TitleId.MestreDoCombate, nome: 'Mestre do Combate', descricao: 'Domínio avançado das artes bélicas.', atk: 15, def: 15, hp: 15, mp: 15 },
  { id: TitleId.MatadorDeBosses, nome: 'Matador de Chefes', descricao: 'Eliminou os grandes líderes do continente.', atk: 25, def: 20, hp: 20, mp: 20 },
  { id: TitleId.Complecionista, nome: 'Complecionista', descricao: '100% do Códex concluído. Força absoluta de Destinia.', atk: 100, def: 100, hp: 100, mp: 100 },
  { id: TitleId.SenhorDoAbismo, nome: 'Senhor do Abismo', descricao: 'Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.', atk: 150, def: 150, hp: 150, mp: 150 }
];

export function findTitle(id) {
  return ALL_TITLES.find(t => t.id === id) || ALL_TITLES[0];
}

export function isCodexComplete(codexEntries, totalExpectedCount = 73) {
  if (!codexEntries || typeof codexEntries.size !== 'number') return false;
  return codexEntries.size >= totalExpectedCount;
}

export function applyTitleBuffs(stats, titleId) {
  const t = findTitle(titleId);
  const result = [...stats];
  if (t.hp > 0) result[5] = Math.floor(result[5] * (1 + t.hp / 100)); // PV / MaxHP
  if (t.mp > 0) result[6] = Math.floor(result[6] * (1 + t.mp / 100)); // PM / MaxMP
  if (t.atk > 0) result[7] = Math.floor(result[7] * (1 + t.atk / 100)); // ATK
  if (t.def > 0) result[8] = Math.floor(result[8] * (1 + t.def / 100)); // DEF
  return result;
}

export class TitlesStorage {
  constructor(key = 'destinia_titles_v1') {
    this.key = key;
    this.equipped = {};
    this.unlocked = {};
  }

  load(raw) {
    if (!raw) return;
    try {
      const data = JSON.parse(raw);
      this.equipped = data.equipped || {};
      this.unlocked = data.unlocked || {};
    } catch {
      // ignore
    }
  }

  serialize() {
    return JSON.stringify({
      equipped: this.equipped,
      unlocked: this.unlocked
    });
  }

  getEquipped(slot) {
    return this.equipped[slot] || TitleId.Nenhum;
  }

  setEquipped(slot, id) {
    this.equipped[slot] = id;
  }

  unlock(slot, id) {
    if (!this.unlocked[slot]) this.unlocked[slot] = [TitleId.Nenhum, TitleId.Iniciante];
    if (!this.unlocked[slot].includes(id)) {
      this.unlocked[slot].push(id);
    }
  }

  isUnlocked(slot, id) {
    if (id === TitleId.Nenhum || id === TitleId.Iniciante) return true;
    return (this.unlocked[slot] || []).includes(id);
  }
}
