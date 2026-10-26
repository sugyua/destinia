import test from 'node:test';
import assert from 'node:assert/strict';
import {
  bossDropCatalog,
  findBossDrop,
  equipmentSetCatalog,
  epicMissionsCatalog,
  SetDropSourceType,
  identifySet,
  evaluateActiveSets,
  calculateOfflineCash,
  LootEconomy,
  MAX_OFFLINE_SECONDS,
  DEFAULT_OFFLINE_CASH_INTERVAL
} from '../dist/loot-economy.mjs';

test('catalogo de drops de boss possui os 10 chefes de 100 a 109', () => {
  assert.equal(bossDropCatalog.length, 10);
  for (let species = 100; species <= 109; species++) {
    const drop = findBossDrop(species);
    assert.ok(drop, `Chefe ${species} deve possuir drop exclusivo`);
    assert.equal(drop.species, species);
    assert.ok(drop.bossName.length > 0);
    assert.ok(drop.namePt.length > 0);
    assert.ok(drop.nameEn.length > 0);
    assert.ok(drop.descPt.length > 0);
    assert.ok(drop.descEn.length > 0);
    assert.equal(drop.rarity, 4); // Lendário
  }
  assert.equal(findBossDrop(99), null);
  assert.equal(findBossDrop(110), null);
});

test('conjuntos de equipamentos possuem origens concentradas e distribuidas', () => {
  assert.equal(equipmentSetCatalog.length, 5);

  // Set Dragão (Concentrado no Boss 108 Diablo)
  const dragon = equipmentSetCatalog[0];
  assert.equal(dragon.id, 'dragon');
  assert.equal(dragon.dropType, SetDropSourceType.Concentrated);
  assert.equal(dragon.primaryBoss, 108);
  assert.equal(dragon.eliteSource, false);
  assert.ok(dragon.farmRoutePt.includes('Concentrado'));

  // Set Sombrio (Distribuído com Elites)
  const shadow = equipmentSetCatalog[1];
  assert.equal(shadow.id, 'shadow');
  assert.equal(shadow.dropType, SetDropSourceType.Distributed);
  assert.equal(shadow.primaryBoss, -1);
  assert.equal(shadow.eliteSource, true);

  // Set Guardião (Concentrado no Boss 100 Dungeon Guardian)
  const guardian = equipmentSetCatalog[4];
  assert.equal(guardian.id, 'guardian');
  assert.equal(guardian.dropType, SetDropSourceType.Concentrated);
  assert.equal(guardian.primaryBoss, 100);
});

test('conjuntos de equipamentos identificam palavras-chave e ativam bonus progressivos de 2 a 6 pecas', () => {
  // Identificação de itens
  assert.equal(identifySet('Elmo do Dragao Vermelho'), 0);
  assert.equal(identifySet('Dragon Breastplate'), 0);
  assert.equal(identifySet('Manto Sombrio do Ladino'), 1);
  assert.equal(identifySet('Capuz Imperial'), 2);
  assert.equal(identifySet('Diadema Arcano'), 3);
  assert.equal(identifySet('Manoplas do Guardiao'), 4);
  assert.equal(identifySet('Espada Curta Vulgar'), -1);

  // Lista vazia
  assert.deepEqual(evaluateActiveSets([]), []);

  // 1 peça
  assert.deepEqual(evaluateActiveSets(['Elmo do Dragao']), []);

  // 2 peças -> bônus de 2 peças ativo
  const twoPieces = evaluateActiveSets(['Elmo do Dragao', 'Botas do Dragao']);
  assert.equal(twoPieces.length, 1);
  assert.equal(twoPieces[0].setId, 'dragon');
  assert.equal(twoPieces[0].piecesEquipped, 2);
  assert.equal(twoPieces[0].bonus2Active, true);
  assert.equal(twoPieces[0].bonus3Active, false);
  assert.equal(twoPieces[0].bonus4Active, false);
  assert.equal(twoPieces[0].bonus5Active, false);
  assert.equal(twoPieces[0].bonus6Active, false);
  assert.equal(twoPieces[0].activeBonuses.length, 1);
  assert.equal(twoPieces[0].activeBonuses[0].piecesRequired, 2);
  assert.equal(twoPieces[0].activeBonuses[0].atk, 30);
  assert.equal(twoPieces[0].activeBonuses[0].str, 20);

  // 3 peças -> bônus de 2 e 3 peças ativos
  const threePieces = evaluateActiveSets(['Elmo do Dragao', 'Botas do Dragao', 'Luvas do Dragao']);
  assert.equal(threePieces.length, 1);
  assert.equal(threePieces[0].piecesEquipped, 3);
  assert.equal(threePieces[0].bonus2Active, true);
  assert.equal(threePieces[0].bonus3Active, true);
  assert.equal(threePieces[0].bonus4Active, false);
  assert.equal(threePieces[0].activeBonuses.length, 2);
  assert.equal(threePieces[0].activeBonuses[1].def, 35);
  assert.equal(threePieces[0].activeBonuses[1].hp, 150);

  // 4 peças -> bônus de 2, 3 e 4 peças ativos
  const fourPieces = evaluateActiveSets([
    'Elmo do Dragao',
    'Armadura do Dragao',
    'Luvas do Dragao',
    'Botas do Dragao'
  ]);
  assert.equal(fourPieces.length, 1);
  assert.equal(fourPieces[0].piecesEquipped, 4);
  assert.equal(fourPieces[0].bonus2Active, true);
  assert.equal(fourPieces[0].bonus3Active, true);
  assert.equal(fourPieces[0].bonus4Active, true);
  assert.equal(fourPieces[0].bonus5Active, false);
  assert.equal(fourPieces[0].activeBonuses.length, 3);
  assert.equal(fourPieces[0].activeBonuses[2].hp, 300);
  assert.equal(fourPieces[0].activeBonuses[2].critDmg, 15);

  // 5 peças -> bônus de 2, 3, 4 e 5 peças ativos
  const fivePieces = evaluateActiveSets([
    'Elmo do Dragao',
    'Armadura do Dragao',
    'Luvas do Dragao',
    'Botas do Dragao',
    'Anel do Dragao'
  ]);
  assert.equal(fivePieces.length, 1);
  assert.equal(fivePieces[0].piecesEquipped, 5);
  assert.equal(fivePieces[0].bonus5Active, true);
  assert.equal(fivePieces[0].bonus6Active, false);
  assert.equal(fivePieces[0].activeBonuses.length, 4);
  assert.equal(fivePieces[0].activeBonuses[3].critChance, 10);
  assert.equal(fivePieces[0].activeBonuses[3].str, 35);

  // 6 peças -> todos os 5 bônus ativos, incluindo bônus 6 supremo
  const sixPieces = evaluateActiveSets([
    'Elmo do Dragao',
    'Armadura do Dragao',
    'Luvas do Dragao',
    'Botas do Dragao',
    'Anel do Dragao',
    'Espada do Dragao'
  ]);
  assert.equal(sixPieces.length, 1);
  assert.equal(sixPieces[0].piecesEquipped, 6);
  assert.equal(sixPieces[0].bonus2Active, true);
  assert.equal(sixPieces[0].bonus3Active, true);
  assert.equal(sixPieces[0].bonus4Active, true);
  assert.equal(sixPieces[0].bonus5Active, true);
  assert.equal(sixPieces[0].bonus6Active, true);
  assert.equal(sixPieces[0].activeBonuses.length, 5);
  assert.equal(sixPieces[0].activeBonuses[4].piecesRequired, 6);
  assert.equal(sixPieces[0].activeBonuses[4].atk, 100);
  assert.equal(sixPieces[0].activeBonuses[4].str, 50);
  assert.equal(sixPieces[0].activeBonuses[4].critDmg, 25);
  assert.equal(sixPieces[0].activeBonuses[4].hp, 500);

  // Conjuntos híbridos (3 Arcano + 3 Guardião)
  const hybrid = evaluateActiveSets([
    'Diadema Arcano',
    'Robe Arcano',
    'Varinha Arcana',
    'Couraça do Guardiao',
    'Grevas do Guardiao',
    'Escudo do Guardiao'
  ]);
  assert.equal(hybrid.length, 2);
  assert.equal(hybrid[0].setId, 'arcane');
  assert.equal(hybrid[0].piecesEquipped, 3);
  assert.equal(hybrid[0].bonus2Active, true);
  assert.equal(hybrid[0].bonus3Active, true);
  assert.equal(hybrid[1].setId, 'guardian');
  assert.equal(hybrid[1].piecesEquipped, 3);
  assert.equal(hybrid[1].bonus2Active, true);
  assert.equal(hybrid[1].bonus3Active, true);
});

test('missoes epicas de cash por abates de boss e set de 6 pecas', () => {
  assert.equal(epicMissionsCatalog.length, 5);
  assert.equal(epicMissionsCatalog[0].id, 'boss_slayer_10');
  assert.equal(epicMissionsCatalog[0].cashReward, 25);
  assert.equal(epicMissionsCatalog[1].id, 'boss_slayer_25');
  assert.equal(epicMissionsCatalog[1].cashReward, 50);
  assert.equal(epicMissionsCatalog[2].id, 'boss_slayer_50');
  assert.equal(epicMissionsCatalog[2].cashReward, 75);
  assert.equal(epicMissionsCatalog[3].id, 'boss_slayer_100');
  assert.equal(epicMissionsCatalog[3].cashReward, 100);
  assert.equal(epicMissionsCatalog[4].id, 'full_set_equipped');
  assert.equal(epicMissionsCatalog[4].cashReward, 50);

  const econ = new LootEconomy();
  econ.select(0);

  // 0 kills
  assert.equal(econ.getTotalBossKills(0), 0);
  assert.equal(econ.checkAndClaimEpicMissions(0, 0), 0);

  // 9 kills -> nenhuma missão
  for (let i = 0; i < 9; i++) {
    econ.recordBossKill(0, 100 + (i % 10));
  }
  assert.equal(econ.getTotalBossKills(0), 9);
  assert.equal(econ.checkAndClaimEpicMissions(0, 0), 0);

  // 10º kill -> resgata boss_10 (25 Cash)
  econ.recordBossKill(0, 100);
  assert.equal(econ.getTotalBossKills(0), 10);
  assert.equal(econ.checkAndClaimEpicMissions(0, 0), 25);
  assert.equal(econ.totalMissionCashAwarded, 25);
  // Idempotente
  assert.equal(econ.checkAndClaimEpicMissions(0, 0), 0);

  // 25 kills -> resgata boss_25 (50 Cash)
  for (let i = 0; i < 15; i++) {
    econ.recordBossKill(0, 101);
  }
  assert.equal(econ.getTotalBossKills(0), 25);
  assert.equal(econ.checkAndClaimEpicMissions(0, 0), 50);
  assert.equal(econ.totalMissionCashAwarded, 75);

  // Missão de Set de 6 peças
  assert.equal(econ.checkAndClaimEpicMissions(0, 5), 0);
  assert.equal(econ.checkAndClaimEpicMissions(0, 6), 50);
  assert.equal(econ.totalMissionCashAwarded, 125);
  assert.equal(econ.checkAndClaimEpicMissions(0, 6), 0);

  // 50 kills -> 75 Cash
  for (let i = 0; i < 25; i++) {
    econ.recordBossKill(0, 102);
  }
  assert.equal(econ.getTotalBossKills(0), 50);
  assert.equal(econ.checkAndClaimEpicMissions(0, 6), 75);
  assert.equal(econ.totalMissionCashAwarded, 200);

  // 100 kills -> 100 Cash
  for (let i = 0; i < 50; i++) {
    econ.recordBossKill(0, 108);
  }
  assert.equal(econ.getTotalBossKills(0), 100);
  assert.equal(econ.checkAndClaimEpicMissions(0, 6), 100);
  assert.equal(econ.totalMissionCashAwarded, 300);
});

test('cash offline calcula progressao proporcional e aplica teto estrito de 24 horas', () => {
  const rate = DEFAULT_OFFLINE_CASH_INTERVAL; // 1800s = 30min
  assert.equal(MAX_OFFLINE_SECONDS, 86400);

  // Casos inválidos ou relógio invertido
  assert.equal(calculateOfflineCash(0, 1000).cashToCredit, 0);
  assert.equal(calculateOfflineCash(1000, 1000).cashToCredit, 0);
  assert.equal(calculateOfflineCash(1000, 500).cashToCredit, 0);

  // Intervalo menor que 1800s
  const r600 = calculateOfflineCash(1000, 1600, rate);
  assert.equal(r600.cashToCredit, 0);
  assert.equal(r600.capped, false);

  // 1 Cash a cada 30min
  const r1800 = calculateOfflineCash(1000, 2800, rate);
  assert.equal(r1800.cashToCredit, 1);
  assert.equal(r1800.capped, false);

  // 2 Cash em 1h
  const r3600 = calculateOfflineCash(1000, 4600, rate);
  assert.equal(r3600.cashToCredit, 2);
  assert.equal(r3600.capped, false);

  // 12 horas (43200s) = 24 Cash
  const r12h = calculateOfflineCash(1000, 1000 + 43200, rate);
  assert.equal(r12h.cashToCredit, 24);
  assert.equal(r12h.capped, false);

  // Exatamente 24 horas (86400s) = 48 Cash
  const r24h = calculateOfflineCash(1000, 1000 + 86400, rate);
  assert.equal(r24h.cashToCredit, 48);
  assert.equal(r24h.cappedSeconds, 86400);
  assert.equal(r24h.capped, false);

  // 48 horas (172800s) -> TETO ESTRITO DE 24H: 48 Cash
  const r48h = calculateOfflineCash(1000, 1000 + 172800, rate);
  assert.equal(r48h.cashToCredit, 48);
  assert.equal(r48h.cappedSeconds, 86400);
  assert.equal(r48h.capped, true);

  // 1 mês offline -> Continua travado em 48 Cash
  const rMonth = calculateOfflineCash(1000, 1000 + 2592000, rate);
  assert.equal(rMonth.cashToCredit, 48);
  assert.equal(rMonth.cappedSeconds, 86400);
  assert.equal(rMonth.capped, true);
});

test('classe LootEconomy codifica, decodifica DESTINIA_ECONOMY_2 e suporta retrocompatibilidade com v1', () => {
  const econ = new LootEconomy();
  econ.select(1);
  econ.lastTimestamp = 1600000000;
  econ.totalOfflineCashAwarded = 10;
  econ.recordBossKill(1, 100);
  econ.recordBossKill(1, 100);
  econ.recordBossKill(1, 108); // Diablo
  econ.recordBossKill(5, 106); // Kaiser no slot 5

  assert.equal(econ.getBossDropCount(1, 100), 2);
  assert.equal(econ.getBossDropCount(1, 108), 1);
  assert.equal(econ.getBossDropCount(1, 102), 0);
  assert.equal(econ.getBossDropCount(5, 106), 1);
  assert.equal(econ.getBossKillCount(1, 100), 2);
  assert.equal(econ.getTotalBossKills(1), 3);

  // Simular processamento offline
  const result = econ.processOfflineCash(1600003600); // +3600s = 2 Cash
  assert.equal(result.cashToCredit, 2);
  assert.equal(econ.totalOfflineCashAwarded, 12);
  assert.equal(econ.lastTimestamp, 1600003600);

  // Completar mais kills e resgatar missão
  for (let i = 0; i < 7; i++) {
    econ.recordBossKill(1, 103);
  }
  assert.equal(econ.getTotalBossKills(1), 10);
  const claimed = econ.checkAndClaimEpicMissions(1, 6); // 25 (boss_10) + 50 (full_set_6) = 75
  assert.equal(claimed, 75);
  assert.equal(econ.totalMissionCashAwarded, 75);

  // Serialização formato DESTINIA_ECONOMY_2
  const encoded = econ.encode();
  assert.ok(encoded.startsWith('DESTINIA_ECONOMY_2\n'));
  assert.ok(encoded.includes('total_mission_cash 75\n'));
  assert.ok(encoded.includes('mission 1 boss_slayer_10\n'));
  assert.ok(encoded.includes('mission 1 full_set_equipped\n'));

  const restored = new LootEconomy();
  restored.decode(encoded);

  assert.equal(restored.lastTimestamp, 1600003600);
  assert.equal(restored.totalOfflineCashAwarded, 12);
  assert.equal(restored.totalMissionCashAwarded, 75);
  assert.equal(restored.getBossDropCount(1, 100), 2);
  assert.equal(restored.getBossDropCount(1, 108), 1);
  assert.equal(restored.getBossDropCount(5, 106), 1);
  assert.equal(restored.getTotalBossKills(1), 10);
  // Missão já resgatada não deve premiar de novo
  assert.equal(restored.checkAndClaimEpicMissions(1, 6), 0);

  // Retrocompatibilidade: Decodificar payload legado DESTINIA_ECONOMY_1
  const legacyV1 =
    'DESTINIA_ECONOMY_1\n' +
    'timestamp 1500000000\n' +
    'total_cash 45\n' +
    'drop 0 100 3\n' +
    'drop 0 108 2\n';
  const legacyRestored = new LootEconomy();
  legacyRestored.decode(legacyV1);
  assert.equal(legacyRestored.lastTimestamp, 1500000000);
  assert.equal(legacyRestored.totalOfflineCashAwarded, 45);
  assert.equal(legacyRestored.totalMissionCashAwarded, 0);
  assert.equal(legacyRestored.getBossDropCount(0, 100), 3);
  assert.equal(legacyRestored.getBossDropCount(0, 108), 2);

  // Rejeição de formato inválido
  assert.throws(() => restored.decode('INVALID_HEADER\n'), /FORMATO DE ECONOMIA DESCONHECIDO/);
  assert.throws(() => restored.decode(''), /SIDECAR INVALIDO/);
});
