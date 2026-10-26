import test from 'node:test';
import assert from 'node:assert/strict';
import { CatalogEquipment, WeaponsCatalogWeb } from '../dist/weapons-catalog.mjs';
import { SharedStashWeb, MAX_STASH_CAPACITY } from '../dist/shared-stash.mjs';
import { BountiesWeb } from '../dist/bounties.mjs';
import { QuestTrackerWeb } from '../dist/quest-tracker.mjs';
import { BossRushUI, BOSS_RUSH_LIST, ABYSS_FLOORS, PETS_LIST } from '../dist/boss-rush.mjs';
import { AutoPotionController } from '../dist/auto-potion.mjs';

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
  assert.ok(c1.targetId === 61 || c1.targetId === 2);
  assert.equal(c1.completed, false);
  assert.equal(c1.claimed, false);

  // Simular abates
  for (let i = 0; i < 14; i++) {
    bounties.onMonsterKilled(0, 61);
    assert.equal(c1.completed, false);
  }
  bounties.onMonsterKilled(0, 61); // 15º abate
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

test('Auto Potion QoL: consumo automatico sob threshold e respeito a cooldown', () => {
  const fakeEngine = {
    exports: new Map([['Game', 0x1000]]),
    mem: new Map(),
    rd16(addr) { return this.mem.get(addr) || 0; },
    wr16(addr, val) { this.mem.set(addr, val); }
  };

  const ap = new AutoPotionController(fakeEngine);
  assert.equal(ap.enabled, true);

  // Toggle
  ap.toggle();
  assert.equal(ap.enabled, false);
  ap.toggle();
  assert.equal(ap.enabled, true);

  // Setup: In gameplay (0x4854 = 0), Max HP = 1000 (0x48c4), Cur HP = 200 (0x48f2) (20% <= 35%)
  fakeEngine.mem.set(0x1000 + 0x4854, 0);
  fakeEngine.mem.set(0x1000 + 0x48c4, 1000);
  fakeEngine.mem.set(0x1000 + 0x48f2, 200);

  ap.update();
  assert.ok(fakeEngine.mem.get(0x1000 + 0x48f2) > 200, 'HP deve ser recuperado');
  assert.equal(ap.cooldownHpTimer, 60, 'Cooldown de HP deve ser ativado');

  // Próximo frame não deve curar de novo
  const healedHp = fakeEngine.mem.get(0x1000 + 0x48f2);
  ap.update();
  assert.equal(fakeEngine.mem.get(0x1000 + 0x48f2), healedHp, 'Não deve curar em cooldown');
});

test('Mago dos Bosses 0.11: 4 Abas (Chefes, Torre Abissal, Mascotes, Santuario)', () => {
  const ui = new BossRushUI(null);
  assert.equal(ui.currentTab, 0);

  // Alternar abas com teclas 5 (esquerda) e 7 (direita)
  ui.openMenu();
  ui.handleKey(7, true); // Aba 1: Torre Abissal
  assert.equal(ui.currentTab, 1);
  assert.equal(ui.getItemsCount(), 5); // maxUnlockedFloor padrão = 5

  ui.handleKey(7, true); // Aba 2: Mascotes
  assert.equal(ui.currentTab, 2);
  assert.equal(ui.getItemsCount(), 4); // 4 mascotes

  ui.handleKey(7, true); // Aba 3: Santuario
  assert.equal(ui.currentTab, 3);
  assert.equal(ui.getItemsCount(), 3); // Reset Atributos, Reset Skills, NG+

  ui.handleKey(7, true); // Aba 4: Titulos (0.12.0)
  assert.equal(ui.currentTab, 4);
  assert.equal(ui.getItemsCount(), 8); // 8 titulos

  ui.handleKey(7, true); // Volta para Aba 0: Chefes
  assert.equal(ui.currentTab, 0);
  assert.equal(ui.getItemsCount(), 11);
});

test('Torre Abissal: 50 andares com chefes a cada 5 andares', () => {
  assert.equal(ABYSS_FLOORS.length, 50);
  for (let f = 1; f <= 50; f++) {
    const fl = ABYSS_FLOORS[f - 1];
    assert.equal(fl.floor, f);
    assert.equal(fl.isBoss, f % 5 === 0);
  }
});

test('Mascotes / Companheiros: 4 mascotes canonicos com buffs', () => {
  assert.equal(PETS_LIST.length, 4);
  assert.equal(PETS_LIST[0].namePt, 'Lobo Glacial');
  assert.equal(PETS_LIST[1].namePt, 'Fenix Ignea');
  assert.equal(PETS_LIST[2].namePt, 'Golem de Pedra');
  assert.equal(PETS_LIST[3].namePt, 'Mini-Diablo');
});

test('Suporte a Controles USB 0.11: Chords com Select e Botoes L3/R3', async () => {
  const { WebGamepad } = await import('../dist/gamepad.mjs');
  const triggered = [];
  const pad = {
    index: 0,
    id: 'USB Gamepad Test',
    connected: true,
    mapping: 'standard',
    axes: [0, 0, 0, 0],
    buttons: Array.from({ length: 17 }, () => ({ pressed: false, value: 0 }))
  };

  const gamepad = new WebGamepad({
    sources: { press: () => {}, release: () => {} },
    read: () => [pad],
    onAction: (act) => triggered.push(act)
  });

  const active = { ready: true, focused: true, paused: false, blocked: false };

  // Primeiro tick neutro para armar o gamepad
  gamepad.tick(active);

  // 1. R3 (Botão 11) -> toggleAutoPotion
  pad.buttons[11].pressed = true;
  gamepad.tick(active);
  assert.deepEqual(triggered, ['toggleAutoPotion']);
  pad.buttons[11].pressed = false;
  gamepad.tick(active);

  // 2. L3 (Botão 10) -> toggleQuestTracker
  pad.buttons[10].pressed = true;
  gamepad.tick(active);
  assert.deepEqual(triggered, ['toggleAutoPotion', 'toggleQuestTracker']);
  pad.buttons[10].pressed = false;
  gamepad.tick(active);

  // 3. SELECT + X (Botão 8 + Botão 2) -> toggleWeaponsCatalog
  pad.buttons[8].pressed = true;
  pad.buttons[2].pressed = true;
  gamepad.tick(active);
  assert.deepEqual(triggered, ['toggleAutoPotion', 'toggleQuestTracker', 'toggleWeaponsCatalog']);
  pad.buttons[2].pressed = false;
  pad.buttons[8].pressed = false;
  gamepad.tick(active);

  // 4. SELECT + Y (Botão 8 + Botão 3) -> toggleSharedStash
  pad.buttons[8].pressed = true;
  pad.buttons[3].pressed = true;
  gamepad.tick(active);
  assert.deepEqual(triggered, ['toggleAutoPotion', 'toggleQuestTracker', 'toggleWeaponsCatalog', 'toggleSharedStash']);
  pad.buttons[3].pressed = false;
  pad.buttons[8].pressed = false;
  gamepad.tick(active);

  // 5. SELECT + B (Botão 8 + Botão 1) -> toggleBounties
  pad.buttons[8].pressed = true;
  pad.buttons[1].pressed = true;
  gamepad.tick(active);
  assert.deepEqual(triggered, ['toggleAutoPotion', 'toggleQuestTracker', 'toggleWeaponsCatalog', 'toggleSharedStash', 'toggleBounties']);
  pad.buttons[1].pressed = false;
  pad.buttons[8].pressed = false;
  gamepad.tick(active);
});
