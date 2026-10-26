#include "../common/loot_economy.h"
#include "../common/loot_economy_store.h"
#include <cassert>
#include <iostream>

using namespace destinia;

void testBossDrops() {
  const auto& drops = bossDropCatalog();
  assert(drops.size() == 10);
  for (int i = 0; i < 10; i++) {
    int expectedSpecies = 100 + i;
    const auto* d = findBossDrop(expectedSpecies);
    assert(d != nullptr);
    assert(d->species == expectedSpecies);
    assert(!d->bossName.empty());
    assert(!d->itemId.empty());
    assert(!d->namePt.empty());
    assert(!d->nameEn.empty());
    assert(!d->descPt.empty());
    assert(!d->descEn.empty());
    assert(d->rarity == 4); // Lendário
  }
  // Boundary tests
  assert(findBossDrop(99) == nullptr);
  assert(findBossDrop(110) == nullptr);
  assert(findBossDrop(-1) == nullptr);
  std::cout << "[PASS] testBossDrops OK\n";
}

void testEquipmentSets() {
  const auto& sets = equipmentSetCatalog();
  assert(sets.size() == 5);

  // Verificação de tipos de drop (Concentrado vs Distribuído)
  // Set 0: Dragon -> Concentrado no Boss 108
  assert(sets[0].dropType == SetDropSourceType::Concentrated);
  assert(sets[0].primaryBoss == 108);
  assert(!sets[0].eliteSource);
  assert(!sets[0].farmRoutePt.empty());

  // Set 1: Shadow -> Distribuído
  assert(sets[1].dropType == SetDropSourceType::Distributed);
  assert(sets[1].primaryBoss == -1);
  assert(sets[1].eliteSource);

  // Set 2: Imperial -> Distribuído
  assert(sets[2].dropType == SetDropSourceType::Distributed);
  assert(sets[2].primaryBoss == -1);

  // Set 3: Arcane -> Distribuído
  assert(sets[3].dropType == SetDropSourceType::Distributed);
  assert(sets[3].primaryBoss == -1);

  // Set 4: Guardian -> Concentrado no Boss 100 (Dungeon Guardian)
  assert(sets[4].dropType == SetDropSourceType::Concentrated);
  assert(sets[4].primaryBoss == 100);
  assert(!sets[4].eliteSource);

  // Set identification
  assert(identifySet("Elmo do Dragao Ancestral") == 0);
  assert(identifySet("Ancient Dragon Helm") == 0);
  assert(identifySet("Capuz Sombrio do Assassino") == 1);
  assert(identifySet("Shadow Robe") == 1);
  assert(identifySet("Armadura Imperial de Kaiser") == 2);
  assert(identifySet("Manto Arcano Elemental") == 3);
  assert(identifySet("Botas do Guardiao da Luz") == 4);
  assert(identifySet("Espada Simples de Madeira") == -1);

  // Evaluation: 0 pieces
  std::vector<std::string> emptyList;
  assert(evaluateActiveSets(emptyList).empty());

  // Evaluation: 1 piece
  std::vector<std::string> onePiece = {"Elmo do Dragao"};
  assert(evaluateActiveSets(onePiece).empty());

  // Evaluation: 2 pieces (activates bonus 2)
  std::vector<std::string> twoPieces = {"Elmo do Dragao", "Armadura do Dragao"};
  auto active2 = evaluateActiveSets(twoPieces);
  assert(active2.size() == 1);
  assert(active2[0].setId == "dragon");
  assert(active2[0].piecesEquipped == 2);
  assert(active2[0].bonus2Active);
  assert(!active2[0].bonus3Active);
  assert(!active2[0].bonus4Active);
  assert(!active2[0].bonus5Active);
  assert(!active2[0].bonus6Active);
  assert(active2[0].activeBonuses.size() == 1);
  assert(active2[0].activeBonuses[0].piecesRequired == 2);
  assert(active2[0].activeBonuses[0].atk == 30);
  assert(active2[0].activeBonuses[0].str == 20);

  // Evaluation: 3 pieces (activates bonus 2 and 3)
  std::vector<std::string> threePieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Luvas do Dragao"
  };
  auto active3 = evaluateActiveSets(threePieces);
  assert(active3.size() == 1);
  assert(active3[0].piecesEquipped == 3);
  assert(active3[0].bonus2Active);
  assert(active3[0].bonus3Active);
  assert(!active3[0].bonus4Active);
  assert(active3[0].activeBonuses.size() == 2);
  assert(active3[0].activeBonuses[1].piecesRequired == 3);
  assert(active3[0].activeBonuses[1].def == 35);
  assert(active3[0].activeBonuses[1].hp == 150);

  // Evaluation: 4 pieces (activates bonus 2, 3 and 4)
  std::vector<std::string> fourPieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Luvas do Dragao", "Botas do Dragao"
  };
  auto active4 = evaluateActiveSets(fourPieces);
  assert(active4.size() == 1);
  assert(active4[0].piecesEquipped == 4);
  assert(active4[0].bonus2Active);
  assert(active4[0].bonus3Active);
  assert(active4[0].bonus4Active);
  assert(!active4[0].bonus5Active);
  assert(active4[0].activeBonuses.size() == 3);
  assert(active4[0].activeBonuses[2].piecesRequired == 4);
  assert(active4[0].activeBonuses[2].hp == 300);
  assert(active4[0].activeBonuses[2].critDmg == 15);

  // Evaluation: 5 pieces (activates bonus 2, 3, 4 and 5)
  std::vector<std::string> fivePieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Luvas do Dragao", "Botas do Dragao", "Anel do Dragao"
  };
  auto active5 = evaluateActiveSets(fivePieces);
  assert(active5.size() == 1);
  assert(active5[0].piecesEquipped == 5);
  assert(active5[0].bonus5Active);
  assert(!active5[0].bonus6Active);
  assert(active5[0].activeBonuses.size() == 4);
  assert(active5[0].activeBonuses[3].piecesRequired == 5);
  assert(active5[0].activeBonuses[3].critChance == 10);
  assert(active5[0].activeBonuses[3].str == 35);

  // Evaluation: 6 pieces (activates all bonuses 2, 3, 4, 5 and 6 supremo)
  std::vector<std::string> sixPieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Luvas do Dragao",
    "Botas do Dragao", "Anel do Dragao", "Espada do Dragao"
  };
  auto active6 = evaluateActiveSets(sixPieces);
  assert(active6.size() == 1);
  assert(active6[0].piecesEquipped == 6);
  assert(active6[0].bonus2Active);
  assert(active6[0].bonus3Active);
  assert(active6[0].bonus4Active);
  assert(active6[0].bonus5Active);
  assert(active6[0].bonus6Active);
  assert(active6[0].activeBonuses.size() == 5);
  assert(active6[0].activeBonuses[4].piecesRequired == 6);
  assert(active6[0].activeBonuses[4].atk == 100);
  assert(active6[0].activeBonuses[4].str == 50);
  assert(active6[0].activeBonuses[4].critDmg == 25);
  assert(active6[0].activeBonuses[4].hp == 500);

  // Evaluation: Mixed sets (3 Shadow + 3 Arcane)
  std::vector<std::string> mixedPieces = {
    "Capuz Sombrio", "Manto Sombrio", "Garras Sombrias",
    "Diadema Arcano", "Robe Arcano", "Botas Arcanas"
  };
  auto activeMixed = evaluateActiveSets(mixedPieces);
  assert(activeMixed.size() == 2);
  assert(activeMixed[0].setId == "shadow");
  assert(activeMixed[0].piecesEquipped == 3);
  assert(activeMixed[0].bonus2Active);
  assert(activeMixed[0].bonus3Active);
  assert(!activeMixed[0].bonus4Active);
  assert(activeMixed[1].setId == "arcane");
  assert(activeMixed[1].piecesEquipped == 3);
  assert(activeMixed[1].bonus2Active);
  assert(activeMixed[1].bonus3Active);

  std::cout << "[PASS] testEquipmentSets OK\n";
}

void testOfflineCashCalculation() {
  const int64_t rate = 1800; // 30 min por 1 Cash

  // Non-positive or backward timestamps
  assert(calculateOfflineCash(0, 1000, rate).cashToCredit == 0);
  assert(calculateOfflineCash(-10, 1000, rate).cashToCredit == 0);
  assert(calculateOfflineCash(1000, 1000, rate).cashToCredit == 0);
  assert(calculateOfflineCash(1000, 500, rate).cashToCredit == 0); // clock rewound
  assert(calculateOfflineCash(1000, 2000, 0).cashToCredit == 0);

  // Normal offline progression
  auto r600 = calculateOfflineCash(1000, 1600, rate);
  assert(r600.cashToCredit == 0);
  assert(r600.elapsedSeconds == 600);
  assert(!r600.capped);

  auto r1800 = calculateOfflineCash(1000, 2800, rate);
  assert(r1800.cashToCredit == 1);
  assert(r1800.elapsedSeconds == 1800);
  assert(!r1800.capped);

  auto r3600 = calculateOfflineCash(1000, 4600, rate);
  assert(r3600.cashToCredit == 2);
  assert(r3600.elapsedSeconds == 3600);
  assert(!r3600.capped);

  auto r12h = calculateOfflineCash(1000, 1000 + 43200, rate);
  assert(r12h.cashToCredit == 24);
  assert(r12h.elapsedSeconds == 43200);
  assert(!r12h.capped);

  // Exactly 24 Hours (86400s) = 48 Cash
  auto r24h = calculateOfflineCash(1000, 1000 + 86400, rate);
  assert(r24h.cashToCredit == 48);
  assert(r24h.elapsedSeconds == 86400);
  assert(r24h.cappedSeconds == 86400);
  assert(!r24h.capped);

  // Exceeding 24 Hours: STRICT CAP AT 24H (86400s)
  auto r48h = calculateOfflineCash(1000, 1000 + 172800, rate);
  assert(r48h.cashToCredit == 48); // Still strictly 48 Cash!
  assert(r48h.elapsedSeconds == 172800);
  assert(r48h.cappedSeconds == 86400); // Capped at 24h
  assert(r48h.capped);

  // 10 Days offline: Still strictly 48 Cash
  auto r10d = calculateOfflineCash(1000, 1000 + 864000, rate);
  assert(r10d.cashToCredit == 48);
  assert(r10d.elapsedSeconds == 864000);
  assert(r10d.cappedSeconds == 86400);
  assert(r10d.capped);

  std::cout << "[PASS] testOfflineCashCalculation OK\n";
}

void testEpicMissionsAndKills() {
  const auto& catalog = epicMissionsCatalog();
  assert(catalog.size() == 5);
  assert(catalog[0].id == "boss_slayer_10");
  assert(catalog[0].cashReward == 25);
  assert(catalog[1].id == "boss_slayer_25");
  assert(catalog[1].cashReward == 50);
  assert(catalog[2].id == "boss_slayer_50");
  assert(catalog[2].cashReward == 75);
  assert(catalog[3].id == "boss_slayer_100");
  assert(catalog[3].cashReward == 100);
  assert(catalog[4].id == "full_set_equipped");
  assert(catalog[4].cashReward == 50);

  LootEconomyState state;
  state.select(1);

  // Inicialmente 0 kills
  assert(state.getTotalBossKills(1) == 0);
  assert(state.checkAndClaimEpicMissions(1, 0) == 0);

  // Registrar 9 kills -> nenhuma missão
  for (int i = 0; i < 9; i++) {
    state.recordBossKill(1, 100 + (i % 10));
  }
  assert(state.getTotalBossKills(1) == 9);
  assert(state.checkAndClaimEpicMissions(1, 0) == 0);

  // 10º kill -> completa boss_10 (25 Cash)
  state.recordBossKill(1, 100);
  assert(state.getTotalBossKills(1) == 10);
  int claimed10 = state.checkAndClaimEpicMissions(1, 0);
  assert(claimed10 == 25);
  assert(state.totalMissionCashAwarded == 25);

  // Chamada subsequente imediata não pode conceder recompensa repetida
  assert(state.checkAndClaimEpicMissions(1, 0) == 0);

  // Adicionar mais 15 kills para totalizar 25 -> completa boss_25 (50 Cash)
  for (int i = 0; i < 15; i++) {
    state.recordBossKill(1, 101);
  }
  assert(state.getTotalBossKills(1) == 25);
  int claimed25 = state.checkAndClaimEpicMissions(1, 0);
  assert(claimed25 == 50);
  assert(state.totalMissionCashAwarded == 75);

  // Testar missão de Full Set 6 peças
  // 5 peças -> não ativa
  assert(state.checkAndClaimEpicMissions(1, 5) == 0);
  // 6 peças -> ativa full_set_6 (50 Cash)
  int claimedSet = state.checkAndClaimEpicMissions(1, 6);
  assert(claimedSet == 50);
  assert(state.totalMissionCashAwarded == 125);
  // Não repete
  assert(state.checkAndClaimEpicMissions(1, 6) == 0);

  // Avançar para 50 kills -> completa boss_50 (75 Cash)
  for (int i = 0; i < 25; i++) {
    state.recordBossKill(1, 102);
  }
  assert(state.getTotalBossKills(1) == 50);
  assert(state.checkAndClaimEpicMissions(1, 6) == 75);
  assert(state.totalMissionCashAwarded == 200);

  // Avançar para 100 kills -> completa boss_100 (100 Cash)
  for (int i = 0; i < 50; i++) {
    state.recordBossKill(1, 108); // Diablo
  }
  assert(state.getTotalBossKills(1) == 100);
  assert(state.checkAndClaimEpicMissions(1, 6) == 100);
  assert(state.totalMissionCashAwarded == 300);

  // Todas as 5 missões resgatadas para o slot 1
  assert(state.checkAndClaimEpicMissions(1, 6) == 0);

  // Slot 2 independente: 0 kills e 0 missões
  assert(state.getTotalBossKills(2) == 0);
  assert(state.checkAndClaimEpicMissions(2, 0) == 0);

  std::cout << "[PASS] testEpicMissionsAndKills OK\n";
}

void testStateSerializationAndSidecar(const std::string& tempDir) {
  LootEconomyState state;
  state.select(2);
  state.lastTimestamp = 1600000000;
  state.totalOfflineCashAwarded = 15;
  state.recordBossKill(2, 100);
  state.recordBossKill(2, 100);
  state.recordBossKill(2, 108); // Diablo
  state.recordBossKill(3, 106); // Kaiser on slot 3

  assert(state.getBossDropCount(2, 100) == 2);
  assert(state.getBossDropCount(2, 108) == 1);
  assert(state.getBossDropCount(2, 105) == 0);
  assert(state.getBossDropCount(3, 106) == 1);
  assert(state.getBossKillCount(2, 100) == 2);
  assert(state.getBossKillCount(2, 108) == 1);

  // Process offline cash
  auto res = state.processOfflineCash(1600003600); // +3600s = +2 Cash
  assert(res.cashToCredit == 2);
  assert(state.totalOfflineCashAwarded == 17);
  assert(state.lastTimestamp == 1600003600);

  // Adicionar missões épicas
  for (int i = 0; i < 8; i++) {
    state.recordBossKill(2, 102);
  }
  // Total kills no slot 2 = 2 + 1 + 8 = 11 -> ativa boss_10
  assert(state.getTotalBossKills(2) == 11);
  int claimed = state.checkAndClaimEpicMissions(2, 6); // ativa boss_10 (25) + full_set_6 (50) = 75
  assert(claimed == 75);
  assert(state.totalMissionCashAwarded == 75);

  // Encode & Decode (DESTINIA_ECONOMY_2)
  const std::string encoded = state.encode();
  assert(encoded.rfind("DESTINIA_ECONOMY_2", 0) == 0);
  LootEconomyState loaded;
  loaded.decode(encoded);
  assert(loaded.lastTimestamp == 1600003600);
  assert(loaded.totalOfflineCashAwarded == 17);
  assert(loaded.totalMissionCashAwarded == 75);
  assert(loaded.getBossDropCount(2, 100) == 2);
  assert(loaded.getBossDropCount(2, 108) == 1);
  assert(loaded.getBossDropCount(3, 106) == 1);
  assert(loaded.getTotalBossKills(2) == 11);
  // Missões já completadas permanecem completadas (não dão cash de novo)
  assert(loaded.checkAndClaimEpicMissions(2, 6) == 0);

  // Retrocompatibilidade: Decodificar payload antigo DESTINIA_ECONOMY_1
  std::string legacyV1 =
    "DESTINIA_ECONOMY_1\n"
    "timestamp 1500000000\n"
    "total_cash 40\n"
    "drop 0 100 5\n"
    "drop 0 108 3\n";
  LootEconomyState legacyLoaded;
  legacyLoaded.decode(legacyV1);
  assert(legacyLoaded.lastTimestamp == 1500000000);
  assert(legacyLoaded.totalOfflineCashAwarded == 40);
  assert(legacyLoaded.totalMissionCashAwarded == 0);
  assert(legacyLoaded.getBossDropCount(0, 100) == 5);
  assert(legacyLoaded.getBossDropCount(0, 108) == 3);

  // Disk I/O via safe store
  fs::path dir = tempDir;
  saveLootEconomy(dir, loaded);
  assert(fs::exists(lootEconomyFile(dir)));

  LootEconomyState restored;
  loadLootEconomy(dir, restored);
  assert(restored.lastTimestamp == loaded.lastTimestamp);
  assert(restored.totalOfflineCashAwarded == loaded.totalOfflineCashAwarded);
  assert(restored.totalMissionCashAwarded == loaded.totalMissionCashAwarded);
  assert(restored.getBossDropCount(2, 100) == 2);
  assert(restored.getBossDropCount(2, 108) == 1);
  assert(restored.getTotalBossKills(2) == 11);

  // Corruption handling
  bool caught = false;
  try {
    restored.decode("DESTINIA_INVALID_FORMAT\n");
  } catch (const std::exception&) {
    caught = true;
  }
  assert(caught);

  std::cout << "[PASS] testStateSerializationAndSidecar OK\n";
}

int main(int argc, char** argv) {
  std::string tempDir = "tmp_economy_test";
  if (argc > 1) tempDir = argv[1];
  fs::create_directories(tempDir);

  testBossDrops();
  testEquipmentSets();
  testOfflineCashCalculation();
  testEpicMissionsAndKills();
  testStateSerializationAndSidecar(tempDir);

  std::error_code ec;
  fs::remove_all(tempDir, ec);

  std::cout << "ALL_LOOT_ECONOMY_TESTS_OK\n";
  return 0;
}
