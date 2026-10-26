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
  assert(!active2[0].bonus4Active);
  assert(active2[0].activeBonuses.size() == 1);
  assert(active2[0].activeBonuses[0].piecesRequired == 2);
  assert(active2[0].activeBonuses[0].atk == 25);
  assert(active2[0].activeBonuses[0].str == 15);

  // Evaluation: 4 pieces (activates bonus 2 and bonus 4)
  std::vector<std::string> fourPieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Luvas do Dragao", "Botas do Dragao"
  };
  auto active4 = evaluateActiveSets(fourPieces);
  assert(active4.size() == 1);
  assert(active4[0].setId == "dragon");
  assert(active4[0].piecesEquipped == 4);
  assert(active4[0].bonus2Active);
  assert(active4[0].bonus4Active);
  assert(active4[0].activeBonuses.size() == 2);
  assert(active4[0].activeBonuses[1].piecesRequired == 4);
  assert(active4[0].activeBonuses[1].atk == 60);
  assert(active4[0].activeBonuses[1].hp == 200);

  // Evaluation: Mixed sets (2 Dragon + 2 Shadow)
  std::vector<std::string> mixedPieces = {
    "Elmo do Dragao", "Armadura do Dragao", "Capuz Sombrio", "Passos Sombrios"
  };
  auto activeMixed = evaluateActiveSets(mixedPieces);
  assert(activeMixed.size() == 2);
  assert(activeMixed[0].setId == "dragon");
  assert(activeMixed[0].piecesEquipped == 2);
  assert(activeMixed[1].setId == "shadow");
  assert(activeMixed[1].piecesEquipped == 2);

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

  // Process offline cash
  auto res = state.processOfflineCash(1600003600); // +3600s = +2 Cash
  assert(res.cashToCredit == 2);
  assert(state.totalOfflineCashAwarded == 17);
  assert(state.lastTimestamp == 1600003600);

  // Encode & Decode
  const std::string encoded = state.encode();
  LootEconomyState loaded;
  loaded.decode(encoded);
  assert(loaded.lastTimestamp == 1600003600);
  assert(loaded.totalOfflineCashAwarded == 17);
  assert(loaded.getBossDropCount(2, 100) == 2);
  assert(loaded.getBossDropCount(2, 108) == 1);
  assert(loaded.getBossDropCount(3, 106) == 1);

  // Disk I/O via safe store
  fs::path dir = tempDir;
  saveLootEconomy(dir, loaded);
  assert(fs::exists(lootEconomyFile(dir)));

  LootEconomyState restored;
  loadLootEconomy(dir, restored);
  assert(restored.lastTimestamp == loaded.lastTimestamp);
  assert(restored.totalOfflineCashAwarded == loaded.totalOfflineCashAwarded);
  assert(restored.getBossDropCount(2, 100) == 2);
  assert(restored.getBossDropCount(2, 108) == 1);

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
  testStateSerializationAndSidecar(tempDir);

  std::error_code ec;
  fs::remove_all(tempDir, ec);

  std::cout << "ALL_LOOT_ECONOMY_TESTS_OK\n";
  return 0;
}
