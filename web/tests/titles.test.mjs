import test from 'node:test';
import assert from 'node:assert/strict';
import { TitleId, findTitle, isCodexComplete, applyTitleBuffs, TitlesStorage } from '../dist/titles.mjs';

test('sistema de titulos: busca e atributos do Complecionista', () => {
  const c = findTitle(TitleId.Complecionista);
  assert.equal(c.nome, 'Complecionista');
  assert.equal(c.atk, 100);
  assert.equal(c.def, 100);
  assert.equal(c.hp, 100);
  assert.equal(c.mp, 100);
});

test('buff de +100% em todos os status principais para o Complecionista', () => {
  // stats array: [0..4 base, 5=HP, 6=MP, 7=ATK, 8=DEF, ...]
  const baseStats = [0, 0, 0, 0, 0, 5000, 2000, 1500, 800, 100, 100, 50, 50];
  const buffed = applyTitleBuffs(baseStats, TitleId.Complecionista);

  assert.equal(buffed[5], 10000, 'HP deve dobrar (+100%)');
  assert.equal(buffed[6], 4000, 'MP deve dobrar (+100%)');
  assert.equal(buffed[7], 3000, 'ATK deve dobrar (+100%)');
  assert.equal(buffed[8], 1600, 'DEF deve dobrar (+100%)');
});

test('verificacao de conclusao de 100% do Codex', () => {
  const incompleteCodex = new Map();
  for (let i = 1; i <= 72; i++) incompleteCodex.set(i, { kills: 1 });
  assert.equal(isCodexComplete(incompleteCodex, 73), false);

  const completeCodex = new Map();
  for (let i = 1; i <= 73; i++) completeCodex.set(i, { kills: 1 });
  assert.equal(isCodexComplete(completeCodex, 73), true);
});

test('persistencia sidecar de titulos isolada de saves nativos', () => {
  const store = new TitlesStorage();
  store.unlock(0, TitleId.Complecionista);
  store.setEquipped(0, TitleId.Complecionista);

  assert.equal(store.isUnlocked(0, TitleId.Complecionista), true);
  assert.equal(store.getEquipped(0), TitleId.Complecionista);

  const raw = store.serialize();
  const loaded = new TitlesStorage();
  loaded.load(raw);

  assert.equal(loaded.isUnlocked(0, TitleId.Complecionista), true);
  assert.equal(loaded.getEquipped(0), TitleId.Complecionista);
});
