#include "../common/titles.h"
#include <cassert>
#include <iostream>

using namespace destinia;

int main() {
  // Teste 1: atributos do título Complecionista
  const TitleDef* c = findTitle(TitleId::Complecionista);
  assert(c != nullptr);
  assert(c->atkBonusPct == 100);
  assert(c->defBonusPct == 100);
  assert(c->hpBonusPct == 100);
  assert(c->mpBonusPct == 100);

  // Teste 2: aplicação do buff de +100%
  CombatStats baseStats{};
  baseStats[5] = 5000;  // HP
  baseStats[6] = 2000;  // MP
  baseStats[7] = 1500;  // ATK
  baseStats[8] = 800;   // DEF

  CombatStats buffed = applyTitleBuffs(baseStats, TitleId::Complecionista);
  assert(buffed[5] == 10000);
  assert(buffed[6] == 4000);
  assert(buffed[7] == 3000);
  assert(buffed[8] == 1600);

  // Teste 3: verificação do Códex
  Codex codex;
  for (int i = 1; i <= 73; ++i) {
    codex.names[i] = "Monster_" + std::to_string(i);
  }
  codex.select(0);
  assert(!isCodexComplete(codex, 0));

  for (int i = 1; i <= 72; ++i) {
    codex.observe(1000 + i, i, true);
  }
  assert(!isCodexComplete(codex, 0));

  codex.observe(1073, 73, true);
  assert(isCodexComplete(codex, 0));

  // Teste 4: sidecar de títulos
  TitlesStore store;
  store.unlock(0, TitleId::Complecionista);
  store.setEquipped(0, TitleId::Complecionista);
  assert(store.isUnlocked(0, TitleId::Complecionista));
  assert(store.getEquipped(0) == TitleId::Complecionista);

  std::string encoded = store.encode();
  TitlesStore loaded;
  assert(loaded.decode(encoded));
  assert(loaded.isUnlocked(0, TitleId::Complecionista));
  assert(loaded.getEquipped(0) == TitleId::Complecionista);


  // Teste 5: atributos e buffs do titulo Senhor do Abismo (0.12.0)
  const TitleDef* sa = findTitle(TitleId::SenhorDoAbismo);
  assert(sa != nullptr);
  assert(sa->atkBonusPct == 150);
  assert(sa->defBonusPct == 150);
  assert(sa->hpBonusPct == 150);
  assert(sa->mpBonusPct == 150);

  CombatStats abyssalBuffed = applyTitleBuffs(baseStats, TitleId::SenhorDoAbismo);
  assert(abyssalBuffed[5] == 12500); // 5000 + 150% = 12500
  assert(abyssalBuffed[6] == 5000);  // 2000 + 150% = 5000
  assert(abyssalBuffed[7] == 3750);  // 1500 + 150% = 3750
  assert(abyssalBuffed[8] == 2000);  // 800 + 150% = 2000

  std::cout << "TITLES_NATIVE_CASES_PASS\n";
  return 0;
}
