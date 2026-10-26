#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace destinia {

enum class CatalogItemCategory : uint8_t {
  Weapon = 0,
  Armor = 1
};

struct EquipmentCatalogDef {
  uint16_t baseId;
  uint8_t minLevel;
  uint8_t jobClass;    // 1: Guerreiro, 2: Atirador, 3: Ladino
  CatalogItemCategory category;
  const char* name;
  const char* typeDesc;
};

// Aliases para compatibilidade
using WeaponCatalogDef = EquipmentCatalogDef;

inline const std::vector<EquipmentCatalogDef>& allCatalogEquipment() {
  static const std::vector<EquipmentCatalogDef> list = {
    // =========================================================================
    // --- ARMAS (Categoria 0) ---
    // =========================================================================
    // Guerreiro (Job 1)
    { 1001, 1,   1, CatalogItemCategory::Weapon, "Espada de Madeira",     "Espada Curta" },
    { 1002, 10,  1, CatalogItemCategory::Weapon, "Espada de Ferro",       "Espada Longa" },
    { 1003, 20,  1, CatalogItemCategory::Weapon, "Espada Larga",          "Montante" },
    { 1004, 35,  1, CatalogItemCategory::Weapon, "Lamina Bastarda",       "Espada Pesada" },
    { 1005, 50,  1, CatalogItemCategory::Weapon, "Gladio de Aco",         "Gladio" },
    { 1006, 65,  1, CatalogItemCategory::Weapon, "Montante Real",         "Montante" },
    { 1007, 80,  1, CatalogItemCategory::Weapon, "Destruidora Draconica", "Lamina Sagrada" },
    { 1008, 100, 1, CatalogItemCategory::Weapon, "Lamina do Destino",     "Arma Lendaria" },

    // Atirador (Job 2)
    { 2001, 1,   2, CatalogItemCategory::Weapon, "Pistola de Treino",     "Pistola" },
    { 2002, 10,  2, CatalogItemCategory::Weapon, "Revolver de Aco",       "Revolver" },
    { 2003, 20,  2, CatalogItemCategory::Weapon, "Pistolas Duplas",       "Arma Dupla" },
    { 2004, 35,  2, CatalogItemCategory::Weapon, "Carabina de Caca",      "Carabina" },
    { 2005, 50,  2, CatalogItemCategory::Weapon, "Fuzil de Precisao",     "Fuzil Sniper" },
    { 2006, 65,  2, CatalogItemCategory::Weapon, "Canhao Portatil",       "Canhao Pesado" },
    { 2007, 80,  2, CatalogItemCategory::Weapon, "Canhao do Trovao",      "Arma Eletrica" },
    { 2008, 100, 2, CatalogItemCategory::Weapon, "Juizo Final",           "Arma Lendaria" },

    // Ladino (Job 3)
    { 3001, 1,   3, CatalogItemCategory::Weapon, "Adaga Gasta",           "Adaga Basica" },
    { 3002, 10,  3, CatalogItemCategory::Weapon, "Adaga de Aco",          "Adaga Afiada" },
    { 3003, 20,  3, CatalogItemCategory::Weapon, "Estilete Veloz",        "Estilete" },
    { 3004, 35,  3, CatalogItemCategory::Weapon, "Garras Sombrias",       "Garras Duplas" },
    { 3005, 50,  3, CatalogItemCategory::Weapon, "Laminas Gemeas",        "Adagas Gemeas" },
    { 3006, 65,  3, CatalogItemCategory::Weapon, "Adaga Espectral",       "Lamina Espiritual" },
    { 3007, 80,  3, CatalogItemCategory::Weapon, "Presa da Noite",        "Adaga Toxica" },
    { 3008, 100, 3, CatalogItemCategory::Weapon, "Beijo da Morte",        "Arma Lendaria" },

    // =========================================================================
    // --- ARMADURAS / PEITORAIS (Categoria 1) ---
    // =========================================================================
    // Guerreiro (Job 1)
    { 1101, 1,   1, CatalogItemCategory::Armor,  "Tunica de Recruta",     "Armadura Leve" },
    { 1102, 10,  1, CatalogItemCategory::Armor,  "Cota de Ferro",         "Armadura de Malha" },
    { 1103, 20,  1, CatalogItemCategory::Armor,  "Peitoral de Aco",       "Placas de Aco" },
    { 1104, 35,  1, CatalogItemCategory::Armor,  "Armadura de Infantaria","Placas Reforcadas" },
    { 1105, 50,  1, CatalogItemCategory::Armor,  "Placas do Comandante",  "Armadura Pesada" },
    { 1106, 65,  1, CatalogItemCategory::Armor,  "Armadura Real",         "Armadura Guardiana" },
    { 1107, 80,  1, CatalogItemCategory::Armor,  "Carapaca Draconica",    "Escamas Draconicas" },
    { 1108, 100, 1, CatalogItemCategory::Armor,  "Armadura do Destino",   "Armadura Lendaria" },

    // Atirador (Job 2)
    { 2101, 1,   2, CatalogItemCategory::Armor,  "Traje de Cacador",      "Couro Basico" },
    { 2102, 10,  2, CatalogItemCategory::Armor,  "Gibao de Couro",        "Couro Tratado" },
    { 2103, 20,  2, CatalogItemCategory::Armor,  "Colete do Atirador",    "Couro Reforcado" },
    { 2104, 35,  2, CatalogItemCategory::Armor,  "Jaqueta Balistica",     "Couro Pesado" },
    { 2105, 50,  2, CatalogItemCategory::Armor,  "Colete do Ranger",      "Camuflagem Tatica" },
    { 2106, 65,  2, CatalogItemCategory::Armor,  "Armadura de Caca Real", "Vestimenta Nobre" },
    { 2107, 80,  2, CatalogItemCategory::Armor,  "Manto do Trovao",       "Couro Elemental" },
    { 2108, 100, 2, CatalogItemCategory::Armor,  "Carapaca do Juizo",     "Traje Lendario" },

    // Ladino (Job 3)
    { 3101, 1,   3, CatalogItemCategory::Armor,  "Roupas Velhas",         "Tecido Gasto" },
    { 3102, 10,  3, CatalogItemCategory::Armor,  "Tunica de Couro",       "Couro Leve" },
    { 3103, 20,  3, CatalogItemCategory::Armor,  "Traje Sombrio",         "Tecido Sombrio" },
    { 3104, 35,  3, CatalogItemCategory::Armor,  "Colete do Espiao",      "Couro Furtivo" },
    { 3105, 50,  3, CatalogItemCategory::Armor,  "Manto do Assassino",    "Vestes Noturnas" },
    { 3106, 65,  3, CatalogItemCategory::Armor,  "Tunica Espectral",      "Tecido Espiritual" },
    { 3107, 80,  3, CatalogItemCategory::Armor,  "Vestes do Vazio",       "Sombra Toxica" },
    { 3108, 100, 3, CatalogItemCategory::Armor,  "Manto da Morte",        "Vestimenta Lendaria" }
  };
  return list;
}

inline const std::vector<EquipmentCatalogDef>& allCatalogWeapons() {
  return allCatalogEquipment();
}

inline const EquipmentCatalogDef* findCatalogEquipment(uint16_t baseId) {
  for (const auto& w : allCatalogEquipment()) {
    if (w.baseId == baseId) return &w;
  }
  return nullptr;
}

inline const EquipmentCatalogDef* findCatalogWeapon(uint16_t baseId) {
  return findCatalogEquipment(baseId);
}

inline std::vector<EquipmentCatalogDef> getCatalogItems(uint8_t jobClass, CatalogItemCategory category) {
  std::vector<EquipmentCatalogDef> result;
  for (const auto& w : allCatalogEquipment()) {
    if (w.category == category && (jobClass == 0 || w.jobClass == jobClass)) {
      result.push_back(w);
    }
  }
  return result;
}

inline std::vector<EquipmentCatalogDef> getCatalogWeaponsByClass(uint8_t jobClass) {
  return getCatalogItems(jobClass, CatalogItemCategory::Weapon);
}

inline std::vector<EquipmentCatalogDef> getCatalogArmorsByClass(uint8_t jobClass) {
  return getCatalogItems(jobClass, CatalogItemCategory::Armor);
}

} // namespace destinia
