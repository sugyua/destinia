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
  uint8_t jobClass;    // 1: Guerreiro, 2: Atirador, 3: Ladino, 4: Mago
  CatalogItemCategory category;
  const char* name;
  const char* typeDesc;
};

using WeaponCatalogDef = EquipmentCatalogDef;

inline const std::vector<EquipmentCatalogDef>& allCatalogEquipment() {
  static const std::vector<EquipmentCatalogDef> list = {
    // --- Guerreiro (Job 1) ---
    {  1000,   1, 1, CatalogItemCategory::Weapon, "Lamina de Novato", "Espada Grande" },
    {  1001,   5, 1, CatalogItemCategory::Weapon, "Lamina Antiga", "Espada Grande" },
    {  1002,  10, 1, CatalogItemCategory::Weapon, "Espada Longa", "Espada Grande" },
    {  1003,  15, 1, CatalogItemCategory::Weapon, "Sabre", "Espada Grande" },
    {  1004,  20, 1, CatalogItemCategory::Weapon, "Espada de Duas Maos", "Espada Grande" },
    {  1005,  25, 1, CatalogItemCategory::Weapon, "Bastard Sword", "Espada Grande" },
    {  1006,  30, 1, CatalogItemCategory::Weapon, "Quebrador", "Espada Grande" },
    {  1007,  35, 1, CatalogItemCategory::Weapon, "Nodachi", "Espada Grande" },
    {  1008,  40, 1, CatalogItemCategory::Weapon, "Jagged Sword", "Espada Grande" },
    {  1009,  45, 1, CatalogItemCategory::Weapon, "Lamina da Lua", "Espada Grande" },
    {  1010,  50, 1, CatalogItemCategory::Weapon, "Destruidor", "Espada Grande" },
    {  1011,  55, 1, CatalogItemCategory::Weapon, "Espada Grande", "Espada Grande" },
    {  1012,  60, 1, CatalogItemCategory::Weapon, "Matador de Veneno", "Espada Grande" },
    {  1013,  65, 1, CatalogItemCategory::Weapon, "Yaksha Sword", "Espada Grande" },
    {  1014,  70, 1, CatalogItemCategory::Weapon, "Lamina Sombria", "Espada Grande" },
    {  1015,  75, 1, CatalogItemCategory::Weapon, "Matador Sombrio", "Espada Grande" },
    {  1016,  80, 1, CatalogItemCategory::Weapon, "Master Sword", "Espada Grande" },
    {  1017,  85, 1, CatalogItemCategory::Weapon, "Lamina do Ar", "Espada Grande" },
    {  1018,  90, 1, CatalogItemCategory::Weapon, "Divine Sword", "Espada Grande" },
    {  1019,  95, 1, CatalogItemCategory::Weapon, "Cruzado", "Espada Grande" },
    {  1020, 100, 1, CatalogItemCategory::Weapon, "Sabre Antigo", "Espada Grande" },
    // --- Atirador (Job 2) ---
    {  2000,   1, 2, CatalogItemCategory::Weapon, "Noob Gun", "Arma de Fogo" },
    {  2001,   5, 2, CatalogItemCategory::Weapon, "Arma Velha", "Arma de Fogo" },
    {  2002,  10, 2, CatalogItemCategory::Weapon, "Hand Gun", "Arma de Fogo" },
    {  2003,  15, 2, CatalogItemCategory::Weapon, "Small Cannon", "Arma de Fogo" },
    {  2004,  20, 2, CatalogItemCategory::Weapon, "Revolver", "Arma de Fogo" },
    {  2005,  25, 2, CatalogItemCategory::Weapon, "Besta", "Arma de Fogo" },
    {  2006,  30, 2, CatalogItemCategory::Weapon, "Mosquete", "Arma de Fogo" },
    {  2007,  35, 2, CatalogItemCategory::Weapon, "Hand Cannon", "Arma de Fogo" },
    {  2008,  40, 2, CatalogItemCategory::Weapon, "Cavaleiro do Sol", "Arma de Fogo" },
    {  2009,  45, 2, CatalogItemCategory::Weapon, "Olho Morto", "Arma de Fogo" },
    {  2010,  50, 2, CatalogItemCategory::Weapon, "Tazzler Gun", "Arma de Fogo" },
    {  2011,  55, 2, CatalogItemCategory::Weapon, "Disparador", "Arma de Fogo" },
    {  2012,  60, 2, CatalogItemCategory::Weapon, "Wyatt Gun", "Arma de Fogo" },
    {  2013,  65, 2, CatalogItemCategory::Weapon, "Atirador Perfurador", "Arma de Fogo" },
    {  2014,  70, 2, CatalogItemCategory::Weapon, "Tilt Machine", "Arma de Fogo" },
    {  2015,  75, 2, CatalogItemCategory::Weapon, "Dildred", "Arma de Fogo" },
    {  2016,  80, 2, CatalogItemCategory::Weapon, "Mega Disparador", "Arma de Fogo" },
    {  2017,  85, 2, CatalogItemCategory::Weapon, "Shield Gun", "Arma de Fogo" },
    {  2018,  90, 2, CatalogItemCategory::Weapon, "Heaven Cannon", "Arma de Fogo" },
    {  2019,  95, 2, CatalogItemCategory::Weapon, "Atirador Giga", "Arma de Fogo" },
    {  2020, 100, 2, CatalogItemCategory::Weapon, "Ancient Gun", "Arma de Fogo" },
    // --- Ladino (Job 3) ---
    {  3000,   1, 3, CatalogItemCategory::Weapon, "Noob Sword", "Espada" },
    {  3001,   5, 3, CatalogItemCategory::Weapon, "Old Sword", "Espada" },
    {  3002,  10, 3, CatalogItemCategory::Weapon, "Lamina", "Espada" },
    {  3003,  15, 3, CatalogItemCategory::Weapon, "Lamina de Ferro", "Espada" },
    {  3004,  20, 3, CatalogItemCategory::Weapon, "Espada Larga", "Espada" },
    {  3005,  25, 3, CatalogItemCategory::Weapon, "Rapieira", "Espada" },
    {  3006,  30, 3, CatalogItemCategory::Weapon, "Gladio", "Espada" },
    {  3007,  35, 3, CatalogItemCategory::Weapon, "Cimitarra", "Espada" },
    {  3008,  40, 3, CatalogItemCategory::Weapon, "Katana", "Espada" },
    {  3009,  45, 3, CatalogItemCategory::Weapon, "Battle Sword", "Espada" },
    {  3010,  50, 3, CatalogItemCategory::Weapon, "Berserk Sword", "Espada" },
    {  3011,  55, 3, CatalogItemCategory::Weapon, "Lamina de Aco", "Espada" },
    {  3012,  60, 3, CatalogItemCategory::Weapon, "Knight Sword", "Espada" },
    {  3013,  65, 3, CatalogItemCategory::Weapon, "Espada Glacial", "Espada" },
    {  3014,  70, 3, CatalogItemCategory::Weapon, "Espada Vulcanica", "Espada" },
    {  3015,  75, 3, CatalogItemCategory::Weapon, "Lamina Fantasma", "Espada" },
    {  3016,  80, 3, CatalogItemCategory::Weapon, "Espada da Tempestade", "Espada" },
    {  3017,  85, 3, CatalogItemCategory::Weapon, "Espada Tornado", "Espada" },
    {  3018,  90, 3, CatalogItemCategory::Weapon, "Dark Sword", "Espada" },
    {  3019,  95, 3, CatalogItemCategory::Weapon, "Flame Sword", "Espada" },
    {  3020, 100, 3, CatalogItemCategory::Weapon, "Ancient Sword", "Espada" },
    // --- Mago (Job 4) ---
    {  4000,   1, 4, CatalogItemCategory::Weapon, "Chapeu de Novato", "Cajado/Elmo" },
    {  4001,   5, 4, CatalogItemCategory::Weapon, "Old Hood", "Cajado/Elmo" },
    {  4002,  10, 4, CatalogItemCategory::Weapon, "Capacete", "Cajado/Elmo" },
    {  4003,  15, 4, CatalogItemCategory::Weapon, "Iron Helmet", "Cajado/Elmo" },
    {  4004,  20, 4, CatalogItemCategory::Weapon, "Scale Hood", "Cajado/Elmo" },
    {  4005,  25, 4, CatalogItemCategory::Weapon, "Suede Hood", "Cajado/Elmo" },
    {  4006,  30, 4, CatalogItemCategory::Weapon, "Chain Helm", "Cajado/Elmo" },
    {  4007,  35, 4, CatalogItemCategory::Weapon, "Refined Hood", "Cajado/Elmo" },
    {  4008,  40, 4, CatalogItemCategory::Weapon, "Bone Helm", "Cajado/Elmo" },
    {  4009,  45, 4, CatalogItemCategory::Weapon, "Silver Helm", "Cajado/Elmo" },
    {  4010,  50, 4, CatalogItemCategory::Weapon, "Hunting Helm", "Cajado/Elmo" },
    {  4011,  55, 4, CatalogItemCategory::Weapon, "Gold Helm", "Cajado/Elmo" },
    {  4012,  60, 4, CatalogItemCategory::Weapon, "Fighter Helm", "Cajado/Elmo" },
    {  4013,  65, 4, CatalogItemCategory::Weapon, "Great Helm", "Cajado/Elmo" },
    {  4014,  70, 4, CatalogItemCategory::Weapon, "Rune Hood", "Cajado/Elmo" },
    {  4015,  75, 4, CatalogItemCategory::Weapon, "Battle Helm", "Cajado/Elmo" },
    {  4016,  80, 4, CatalogItemCategory::Weapon, "War Helm", "Cajado/Elmo" },
    {  4017,  85, 4, CatalogItemCategory::Weapon, "Savage Helm", "Cajado/Elmo" },
    {  4018,  90, 4, CatalogItemCategory::Weapon, "Divine Helm", "Cajado/Elmo" },
    {  4019,  95, 4, CatalogItemCategory::Weapon, "Holy Helm", "Cajado/Elmo" },
    {  4020, 100, 4, CatalogItemCategory::Weapon, "Ancient Helm", "Cajado/Elmo" },
    // --- Guerreiro (Armaduras) ---
    {  1100,   1, 1, CatalogItemCategory::Armor, "Noob Armor", "Armadura" },
    {  1101,   5, 1, CatalogItemCategory::Armor, "Crude Armor", "Armadura" },
    {  1102,  10, 1, CatalogItemCategory::Armor, "Simple Armor", "Armadura" },
    {  1103,  15, 1, CatalogItemCategory::Armor, "Iron Armor", "Armadura" },
    {  1104,  20, 1, CatalogItemCategory::Armor, "Scale Armor", "Armadura" },
    {  1105,  25, 1, CatalogItemCategory::Armor, "Suede Armor", "Armadura" },
    {  1106,  30, 1, CatalogItemCategory::Armor, "Chain Armor", "Armadura" },
    {  1107,  35, 1, CatalogItemCategory::Armor, "Refined Armor", "Armadura" },
    {  1108,  40, 1, CatalogItemCategory::Armor, "Bone Armor", "Armadura" },
    {  1109,  45, 1, CatalogItemCategory::Armor, "Silvery Armor", "Armadura" },
    {  1110,  50, 1, CatalogItemCategory::Armor, "Hunting Armor", "Armadura" },
    {  1111,  55, 1, CatalogItemCategory::Armor, "Golden Armor", "Armadura" },
    {  1112,  60, 1, CatalogItemCategory::Armor, "Fighter Armor", "Armadura" },
    {  1113,  65, 1, CatalogItemCategory::Armor, "Great Armor", "Armadura" },
    {  1114,  70, 1, CatalogItemCategory::Armor, "Rune Armor", "Armadura" },
    {  1115,  75, 1, CatalogItemCategory::Armor, "Battle Armor", "Armadura" },
    {  1116,  80, 1, CatalogItemCategory::Armor, "War Armor", "Armadura" },
    {  1117,  85, 1, CatalogItemCategory::Armor, "Savage Armor", "Armadura" },
    {  1118,  90, 1, CatalogItemCategory::Armor, "Divine Armor", "Armadura" },
    {  1119,  95, 1, CatalogItemCategory::Armor, "Holy Armor", "Armadura" },
    {  1120, 100, 1, CatalogItemCategory::Armor, "Ancient Armor", "Armadura" },
    // --- Atirador (Armaduras) ---
    {  2100,   1, 2, CatalogItemCategory::Armor, "Noob Armor", "Armadura" },
    {  2101,   5, 2, CatalogItemCategory::Armor, "Crude Armor", "Armadura" },
    {  2102,  10, 2, CatalogItemCategory::Armor, "Simple Armor", "Armadura" },
    {  2103,  15, 2, CatalogItemCategory::Armor, "Iron Armor", "Armadura" },
    {  2104,  20, 2, CatalogItemCategory::Armor, "Scale Armor", "Armadura" },
    {  2105,  25, 2, CatalogItemCategory::Armor, "Suede Armor", "Armadura" },
    {  2106,  30, 2, CatalogItemCategory::Armor, "Chain Armor", "Armadura" },
    {  2107,  35, 2, CatalogItemCategory::Armor, "Refined Armor", "Armadura" },
    {  2108,  40, 2, CatalogItemCategory::Armor, "Bone Armor", "Armadura" },
    {  2109,  45, 2, CatalogItemCategory::Armor, "Silvery Armor", "Armadura" },
    {  2110,  50, 2, CatalogItemCategory::Armor, "Hunting Armor", "Armadura" },
    {  2111,  55, 2, CatalogItemCategory::Armor, "Golden Armor", "Armadura" },
    {  2112,  60, 2, CatalogItemCategory::Armor, "Fighter Armor", "Armadura" },
    {  2113,  65, 2, CatalogItemCategory::Armor, "Great Armor", "Armadura" },
    {  2114,  70, 2, CatalogItemCategory::Armor, "Rune Armor", "Armadura" },
    {  2115,  75, 2, CatalogItemCategory::Armor, "Battle Armor", "Armadura" },
    {  2116,  80, 2, CatalogItemCategory::Armor, "War Armor", "Armadura" },
    {  2117,  85, 2, CatalogItemCategory::Armor, "Savage Armor", "Armadura" },
    {  2118,  90, 2, CatalogItemCategory::Armor, "Divine Armor", "Armadura" },
    {  2119,  95, 2, CatalogItemCategory::Armor, "Holy Armor", "Armadura" },
    {  2120, 100, 2, CatalogItemCategory::Armor, "Ancient Armor", "Armadura" },
    // --- Ladino (Armaduras) ---
    {  3100,   1, 3, CatalogItemCategory::Armor, "Noob Armor", "Armadura" },
    {  3101,   5, 3, CatalogItemCategory::Armor, "Crude Armor", "Armadura" },
    {  3102,  10, 3, CatalogItemCategory::Armor, "Simple Armor", "Armadura" },
    {  3103,  15, 3, CatalogItemCategory::Armor, "Iron Armor", "Armadura" },
    {  3104,  20, 3, CatalogItemCategory::Armor, "Scale Armor", "Armadura" },
    {  3105,  25, 3, CatalogItemCategory::Armor, "Suede Armor", "Armadura" },
    {  3106,  30, 3, CatalogItemCategory::Armor, "Chain Armor", "Armadura" },
    {  3107,  35, 3, CatalogItemCategory::Armor, "Refined Armor", "Armadura" },
    {  3108,  40, 3, CatalogItemCategory::Armor, "Bone Armor", "Armadura" },
    {  3109,  45, 3, CatalogItemCategory::Armor, "Silvery Armor", "Armadura" },
    {  3110,  50, 3, CatalogItemCategory::Armor, "Hunting Armor", "Armadura" },
    {  3111,  55, 3, CatalogItemCategory::Armor, "Golden Armor", "Armadura" },
    {  3112,  60, 3, CatalogItemCategory::Armor, "Fighter Armor", "Armadura" },
    {  3113,  65, 3, CatalogItemCategory::Armor, "Great Armor", "Armadura" },
    {  3114,  70, 3, CatalogItemCategory::Armor, "Rune Armor", "Armadura" },
    {  3115,  75, 3, CatalogItemCategory::Armor, "Battle Armor", "Armadura" },
    {  3116,  80, 3, CatalogItemCategory::Armor, "War Armor", "Armadura" },
    {  3117,  85, 3, CatalogItemCategory::Armor, "Savage Armor", "Armadura" },
    {  3118,  90, 3, CatalogItemCategory::Armor, "Divine Armor", "Armadura" },
    {  3119,  95, 3, CatalogItemCategory::Armor, "Holy Armor", "Armadura" },
    {  3120, 100, 3, CatalogItemCategory::Armor, "Ancient Armor", "Armadura" },
    // --- Mago (Armaduras) ---
    {  4100,   1, 4, CatalogItemCategory::Armor, "Noob Armor", "Armadura" },
    {  4101,   5, 4, CatalogItemCategory::Armor, "Crude Armor", "Armadura" },
    {  4102,  10, 4, CatalogItemCategory::Armor, "Simple Armor", "Armadura" },
    {  4103,  15, 4, CatalogItemCategory::Armor, "Iron Armor", "Armadura" },
    {  4104,  20, 4, CatalogItemCategory::Armor, "Scale Armor", "Armadura" },
    {  4105,  25, 4, CatalogItemCategory::Armor, "Suede Armor", "Armadura" },
    {  4106,  30, 4, CatalogItemCategory::Armor, "Chain Armor", "Armadura" },
    {  4107,  35, 4, CatalogItemCategory::Armor, "Refined Armor", "Armadura" },
    {  4108,  40, 4, CatalogItemCategory::Armor, "Bone Armor", "Armadura" },
    {  4109,  45, 4, CatalogItemCategory::Armor, "Silvery Armor", "Armadura" },
    {  4110,  50, 4, CatalogItemCategory::Armor, "Hunting Armor", "Armadura" },
    {  4111,  55, 4, CatalogItemCategory::Armor, "Golden Armor", "Armadura" },
    {  4112,  60, 4, CatalogItemCategory::Armor, "Fighter Armor", "Armadura" },
    {  4113,  65, 4, CatalogItemCategory::Armor, "Great Armor", "Armadura" },
    {  4114,  70, 4, CatalogItemCategory::Armor, "Rune Armor", "Armadura" },
    {  4115,  75, 4, CatalogItemCategory::Armor, "Battle Armor", "Armadura" },
    {  4116,  80, 4, CatalogItemCategory::Armor, "War Armor", "Armadura" },
    {  4117,  85, 4, CatalogItemCategory::Armor, "Savage Armor", "Armadura" },
    {  4118,  90, 4, CatalogItemCategory::Armor, "Divine Armor", "Armadura" },
    {  4119,  95, 4, CatalogItemCategory::Armor, "Holy Armor", "Armadura" },
    {  4120, 100, 4, CatalogItemCategory::Armor, "Ancient Armor", "Armadura" },
  };
  return list;
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
