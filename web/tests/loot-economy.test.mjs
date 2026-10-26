import test from 'node:test';
import assert from 'node:assert/strict';
import {
  bossDropCatalog,
  findBossDrop,
  equipmentSetCatalog,
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

test('conjuntos de equipamentos identificam palavras-chave e ativam bonus de 2 e 4 pecas', () => {
  assert.equal(equipmentSetCatalog.length, 5);

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
  assert.equal(twoPieces[0].bonus4Active, false);
  assert.equal(twoPieces[0].activeBonuses.length, 1);
  assert.equal(twoPieces[0].activeBonuses[0].piecesRequired, 2);
  assert.equal(twoPieces[0].activeBonuses[0].atk, 25);
  assert.equal(twoPieces[0].activeBonuses[0].str, 15);

  // 4 peças -> bônus de 2 e 4 peças ativos
  const fourPieces = evaluateActiveSets([
    'Elmo do Dragao',
    'Armadura do Dragao',
    'Luvas do Dragao',
    'Botas do Dragao'
  ]);
  assert.equal(fourPieces.length, 1);
  assert.equal(fourPieces[0].setId, 'dragon');
  assert.equal(fourPieces[0].piecesEquipped, 4);
  assert.equal(fourPieces[0].bonus2Active, true);
  assert.equal(fourPieces[0].bonus4Active, true);
  assert.equal(fourPieces[0].activeBonuses.length, 2);
  assert.equal(fourPieces[0].activeBonuses[1].piecesRequired, 4);
  assert.equal(fourPieces[0].activeBonuses[1].atk, 60);
  assert.equal(fourPieces[0].activeBonuses[1].hp, 200);

  // Conjuntos híbridos (2 Arcano + 2 Guardião)
  const hybrid = evaluateActiveSets([
    'Diadema Arcano',
    'Robe Arcano',
    'Couraça do Guardiao',
    'Grevas do Guardiao'
  ]);
  assert.equal(hybrid.length, 2);
  assert.equal(hybrid[0].setId, 'arcane');
  assert.equal(hybrid[0].bonus2Active, true);
  assert.equal(hybrid[1].setId, 'guardian');
  assert.equal(hybrid[1].bonus2Active, true);
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

test('classe LootEconomy codifica, decodifica e preserva drops e cash acumulado', () => {
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

  // Simular processamento offline
  const result = econ.processOfflineCash(1600003600); // +3600s = 2 Cash
  assert.equal(result.cashToCredit, 2);
  assert.equal(econ.totalOfflineCashAwarded, 12);
  assert.equal(econ.lastTimestamp, 1600003600);

  // Serialização
  const encoded = econ.encode();
  const restored = new LootEconomy();
  restored.decode(encoded);

  assert.equal(restored.lastTimestamp, 1600003600);
  assert.equal(restored.totalOfflineCashAwarded, 12);
  assert.equal(restored.getBossDropCount(1, 100), 2);
  assert.equal(restored.getBossDropCount(1, 108), 1);
  assert.equal(restored.getBossDropCount(5, 106), 1);

  // Rejeição de formato inválido
  assert.throws(() => restored.decode('INVALID_HEADER\n'), /FORMATO DE ECONOMIA DESCONHECIDO/);
  assert.throws(() => restored.decode(''), /SIDECAR INVALIDO/);
});
