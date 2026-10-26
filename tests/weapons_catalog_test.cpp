#include "common/weapons_catalog.h"
#include <cassert>
#include <iostream>

int main() {
  using namespace destinia;

  WeaponsCatalog cat;

  // Registrar Pistola Nv 1 Comum (rarity 0) e Rara (rarity 2) para Atirador (jobClass 2)
  assert(cat.record(0, 1001, 1, 0, 2) == true);
  assert(cat.record(0, 1001, 1, 2, 2) == true);

  // Duplicata com mesmos dados deve ser ignorada (idempotente)
  assert(cat.record(0, 1001, 1, 0, 2) == false);

  assert(cat.isRecorded(0, 1001, 0) == true);
  assert(cat.isRecorded(0, 1001, 1) == false);
  assert(cat.isRecorded(0, 1001, 2) == true);
  assert(cat.count(0) == 2);

  // Serializar e desserializar
  std::string saved = cat.serialize();
  WeaponsCatalog loaded = WeaponsCatalog::deserialize(saved);

  assert(loaded.count(0) == 2);
  assert(loaded.isRecorded(0, 1001, 0) == true);
  assert(loaded.isRecorded(0, 1001, 2) == true);

  // Completude (se o total esperado for 4 armas, 2 registradas = 50%)
  float pct = loaded.completionPercentage(0, 2, 4);
  assert(pct == 50.0f);

  std::cout << "TESTES WEAPONS CATALOG APROVADOS COM SUCESSO!\n";
  return 0;
}
