import test from 'node:test';
import assert from 'node:assert/strict';
import { CatalogEquipment, WeaponsCatalogWeb } from '../dist/weapons-catalog.mjs';
import { SharedStashWeb, MAX_STASH_CAPACITY } from '../dist/shared-stash.mjs';
import { BountiesWeb } from '../dist/bounties.mjs';
import { QuestTrackerWeb } from '../dist/quest-tracker.mjs';
import { BossRushUI, BOSS_RUSH_LIST } from '../dist/boss-rush.mjs';

test('Catálogo de Equipamentos 0.11: contem 24 armas e 24 armaduras distribuidas igualmente', () => {
  const weapons = CatalogEquipment.filter(e => e.category === 0);
  const armors = CatalogEquipment.filter(e => e.category === 1);

  assert.equal(weapons.length, 24, 'Deve conter 24 armas canônicas');
  assert.equal(armors.length, 24, 'Deve conter 24 armaduras canônicas');

  for (let job = 1; job <= 3; job++) {
    const jobWeapons = weapons.filter(w => w.job === job);
    const jobArmors = armors.filter(a => a.job === job);
    assert.equal(jobWeapons.length, 8, `Classe ${job} deve ter 8 armas`);
    assert.equal(jobArmors.length, 8, `Classe ${job} deve ter 8 armaduras`);
  }
});

test('Catálogo de Equipamentos 0.11: navegacao por categorias e registro de progresso', () => {
  const catalog = new WeaponsCatalogWeb(null);
  assert.equal(catalog.selectedCategory, 0, 'Inicia na categoria Armas');
  catalog.toggleCategory();
  assert.equal(catalog.selectedCategory, 1, 'Alterna para categoria Armaduras');
  catalog.toggleCategory();
  assert.equal(catalog.selectedCategory, 0, 'Retorna para categoria Armas');

  // Registrar descoberta de arma 1001 rank 0..4
  for (let r = 0; r < 5; r++) {
    catalog.record(1001, r);
    assert.ok(catalog.isRecorded(1001, r));
  }

  const pct = catalog.getCompletionPercentage(1);
  assert.ok(pct > 0 && pct <= 100, `Progresso deve ser proporcional: ${pct}%`);
});

test('Baú Compartilhado 0.11: limite estrito de 48 slots e transferencia segura', () => {
  const stash = new SharedStashWeb(null);
  stash.items = [];

  for (let i = 0; i < MAX_STASH_CAPACITY; i++) {
    const ok = stash.deposit({
      pool: 10 + i,
      count: 1,
      durability: 100,
      itemType: 0,
      rank: i % 5,
      level: 1,
      jobClass: 1,
      name: `Espada ${i}`
    });
    assert.ok(ok, `Item ${i} deve ser depositado com sucesso`);
  }

  assert.equal(stash.isFull(), true, 'Baú deve estar cheio com 48 itens');
  const overflow = stash.deposit({ pool: 999, count: 1 });
  assert.equal(overflow, false, 'Não deve aceitar além da capacidade máxima');

  // Retirada
  const withdrawn = stash.withdraw(0);
  assert.equal(withdrawn.pool, 10);
  assert.equal(stash.isFull(), false, 'Não deve estar mais cheio após retirada');
  assert.equal(stash.items.length, 47);

  // Serialização e deserialização
  const serialized = stash.serialize();
  assert.ok(serialized.includes('shared_stash_version 1'));
  const stash2 = new SharedStashWeb(null);
  stash2.deserialize(serialized);
  assert.equal(stash2.items.length, 47);
  assert.equal(stash2.items[0].pool, 11);
});

test('Contratos Offline 0.11: 3 contratos diarios, contagem de abates e resgate de recompensa', () => {
  const bounties = new BountiesWeb(null);
  const contracts = bounties.getContracts(0);
  assert.equal(contracts.length, 3, 'Deve gerar exatamente 3 contratos diários');

  // Contrato 1: Soldados Imperiais (targetId: 2, count: 15)
  const c1 = contracts[0];
  assert.equal(c1.targetId, 2);
  assert.equal(c1.completed, false);
  assert.equal(c1.claimed, false);

  // Simular abates
  for (let i = 0; i < 14; i++) {
    bounties.onMonsterKilled(0, 2);
    assert.equal(c1.completed, false);
  }
  bounties.onMonsterKilled(0, 2); // 15º abate
  assert.equal(c1.completed, true, 'Deve marcar como completo após 15 abates');

  // Reivindicar recompensa
  const reward = bounties.claim(0, c1.id);
  assert.ok(reward !== null, 'Recompensa deve ser creditada');
  assert.equal(reward.gold, 15000);
  assert.equal(reward.cash, 200);
  assert.equal(c1.claimed, true);

  // Não pode resgatar duas vezes
  const doubleClaim = bounties.claim(0, c1.id);
  assert.equal(doubleClaim, null, 'Não deve permitir resgate duplicado');
});

test('Quest Tracker HUD: alternancia de exibicao e leitura de objetivos', () => {
  const tracker = new QuestTrackerWeb(null);
  assert.equal(tracker.minimized, false);
  tracker.toggle();
  assert.equal(tracker.minimized, true);
  tracker.toggle();
  assert.equal(tracker.minimized, false);

  tracker.setQuest('Mago dos Bosses', 'Derrotar Diablo', 'Lagrant');
  assert.equal(tracker.questTitle, 'Mago dos Bosses');
  assert.equal(tracker.questObjective, 'Derrotar Diablo');
  assert.equal(tracker.destination, 'Lagrant');

  // Toque na hitbox (canto superior direito)
  const handled = tracker.handleTouch(700, 15, 0);
  assert.equal(handled, true);
  assert.equal(tracker.minimized, true);
});

test('Boss Rush Gauntlet: 11 opcoes disponiveis incluindo modo sobrevivencia 999', () => {
  assert.equal(BOSS_RUSH_LIST.length, 11);
  const gauntlet = BOSS_RUSH_LIST.find(b => b.species === 999);
  assert.ok(gauntlet !== undefined, 'Modo Gauntlet deve existir na lista');
  assert.equal(gauntlet.goldCost, 10000);
});
