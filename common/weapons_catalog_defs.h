#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace destinia {

struct WeaponCatalogDef {
  uint16_t baseId;
  uint8_t minLevel;
  uint8_t jobClass;    // 1: Guerreiro, 2: Atirador, 3: Ladino
  const char* name;
  const char* typeDesc;
};

inline const std::vector<WeaponCatalogDef>& allCatalogWeapons() {
  static const std::vector<WeaponCatalogDef> list = {
    // --- GUERREIRO (Job 1) ---
    { 1001, 1,   1, "Espada de Madeira",     "Espada Curta" },
    { 1002, 10,  1, "Espada de Ferro",       "Espada Longa" },
    { 1003, 20,  1, "Espada Larga",          "Montante" },
    { 1004, 35,  1, "Lamina Bastarda",       "Espada Pesada" },
    { 1005, 50,  1, "Gladio de Aco",         "Gladio" },
    { 1006, 65,  1, "Montante Real",         "Montante" },
    { 1007, 80,  1, "Destruidora Draconica", "Lamina Sagrada" },
    { 1008, 100, 1, "Lamina do Destino",     "Arma Lendaria" },

    // --- ATIRADOR (Job 2) ---
    { 2001, 1,   2, "Pistola de Treino",     "Pistola" },
    { 2002, 10,  2, "Revolver de Aco",       "Revolver" },
    { 2003, 20,  2, "Pistolas Duplas",       "Arma Dupla" },
    { 2004, 35,  2, "Carabina de Caca",      "Carabina" },
    { 2005, 50,  2, "Fuzil de Precisao",     "Fuzil Sniper" },
    { 2006, 65,  2, "Canhao Portatil",       "Canhao Pesado" },
    { 2007, 80,  2, "Canhao do Trovao",      "Arma Eletrica" },
    { 2008, 100, 2, "Juizo Final",           "Arma Lendaria" },

    // --- LADINO (Job 3) ---
    { 3001, 1,   3, "Adaga Gasta",           "Adaga Basica" },
    { 3002, 10,  3, "Adaga de Aco",          "Adaga Afiada" },
    { 3003, 20,  3, "Estilete Veloz",        "Estilete" },
    { 3004, 35,  3, "Garras Sombrias",       "Garras Duplas" },
    { 3005, 50,  3, "Laminas Gemeas",        "Adagas Gemeas" },
    { 3006, 65,  3, "Adaga Espectral",       "Lamina Espiritual" },
    { 3007, 80,  3, "Presa da Noite",        "Adaga Toxica" },
    { 3008, 100, 3, "Beijo da Morte",        "Arma Lendaria" }
  };
  return list;
}

inline const WeaponCatalogDef* findCatalogWeapon(uint16_t baseId) {
  for (const auto& w : allCatalogWeapons()) {
    if (w.baseId == baseId) return &w;
  }
  return nullptr;
}

inline std::vector<WeaponCatalogDef> getCatalogWeaponsByClass(uint8_t jobClass) {
  std::vector<WeaponCatalogDef> result;
  for (const auto& w : allCatalogWeapons()) {
    if (jobClass == 0 || w.jobClass == jobClass) {
      result.push_back(w);
    }
  }
  return result;
}

} // namespace destinia
