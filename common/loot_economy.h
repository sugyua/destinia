#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace destinia {

// ============================================================================
// 1. DROPS EXCLUSIVOS DE BOSS (Species 100..109)
// ============================================================================
struct BossDropInfo {
  int species;           // 100..109
  std::string bossName;  // Nome do Chefe
  std::string itemId;    // ID único do item
  std::string namePt;    // Nome em Português
  std::string nameEn;    // Nome em Inglês
  std::string descPt;    // Descrição / Lore em Português
  std::string descEn;    // Descrição / Lore em Inglês
  int rarity;            // 4 = Lendário (dourado/amarelo)
  int hpBonus;
  int mpBonus;
  int atkBonus;
  int defBonus;
  int strBonus;
  int dexBonus;
  int vitBonus;
  int intBonus;
  int critBonus;         // em %
  int critDmgBonus;      // em %
  int evasionBonus;
  int lifeStealBonus;    // em %
};

inline const std::vector<BossDropInfo>& bossDropCatalog() {
  static const std::vector<BossDropInfo> catalog = {
    {100, "Dungeon Guardian", "boss_drop_100",
     "Coracao de Pedra Ancestral", "Ancient Stone Heart",
     "Reliquia fossilizada do Guardiao das Profundezas. Concede grande resiliencia fisica.",
     "Fossilized relic of the Deep Guardian. Grants immense physical resilience.",
     4, 150, 0, 0, 20, 0, 0, 15, 0, 0, 0, 0, 0},

    {101, "Defcan", "boss_drop_101",
     "Manto das Sombras de Defcan", "Defcan's Shadow Shroud",
     "Manto etereo tecido com as sombras de Defcan. Aumenta a agilidade e a evasao.",
     "Ethereal shroud woven with Defcan's shadows. Enhances agility and evasion.",
     4, 0, 0, 0, 10, 0, 25, 0, 0, 0, 0, 15, 0},

    {102, "Salas", "boss_drop_102",
     "Presa Venenosa de Salas", "Poison Fang of Salas",
     "Presa peconhenta retirada de Salas. Causa dano agudo e amplia golpes criticos.",
     "Venomous fang harvested from Salas. Inflicts sharp strikes and boosts critical chance.",
     4, 0, 0, 30, 0, 0, 10, 0, 0, 5, 0, 0, 0},

    {103, "Defcan Desperto", "boss_drop_103",
     "Olho do Vazio de Defcan", "Defcan's Void Eye",
     "Fragmento ocular imbuido com energias dimensionais do Defcan Desperto.",
     "Ocular fragment infused with dimensional energy from Awakened Defcan.",
     4, 0, 150, 0, 0, 0, 0, 0, 40, 0, 0, 0, 0},

    {104, "Sileth", "boss_drop_104",
     "Lamina Espectral de Sileth", "Spectral Blade of Sileth",
     "Fragmento de aco espectral de Sileth. Amplifica a potencia dos danos criticos.",
     "Spectral steel fragment of Sileth. Enhances critical damage potency.",
     4, 0, 0, 35, 0, 0, 15, 0, 0, 0, 10, 0, 0},

    {105, "Berserk Tamur", "boss_drop_105",
     "Nucleo de Furia de Tamur", "Tamur's Rage Core",
     "O coracao pulsante de Tamur em frenesi de sangue. Converte dano em vitalidade.",
     "The pulsing heart of berserk Tamur. Converts inflicted damage into vitality.",
     4, 100, 0, 25, 0, 40, 0, 0, 0, 0, 0, 0, 10},

    {106, "Kaiser", "boss_drop_106",
     "Coroa Imperial de Kaiser", "Kaiser's Imperial Crown",
     "A diadema dourada do imperador Kaiser. Fortalece todos os atributos vitais.",
     "The golden diadem of Emperor Kaiser. Enhances all vital attributes.",
     4, 200, 50, 20, 20, 25, 25, 25, 25, 0, 0, 0, 0},

    {107, "Baramos", "boss_drop_107",
     "Chifre Demoniaco de Baramos", "Baramos's Demon Horn",
     "Chifre calcinado de Baramos forjado nas profundezas do inferno.",
     "Calcined horn of Baramos forged in the deep underworld.",
     4, 0, 0, 50, 30, 20, 0, 20, 0, 0, 0, 0, 0},

    {108, "Diablo", "boss_drop_108",
     "Alma Ardente de Diablo", "Diablo's Burning Soul",
     "A chama eterna do demonio primordial Diablo. Poder destrutivo sem igual.",
     "The eternal flame of primeval demon Diablo. Unmatched destructive might.",
     4, 200, 100, 60, 0, 30, 0, 0, 30, 5, 10, 0, 0},

    {109, "Lava Guardian", "boss_drop_109",
     "Magma Primordial do Guardiao", "Guardian's Primeval Magma",
     "Nucleo de pedra fundida do Guardiao de Lava. Defesa e vitalidade inabalaveis.",
     "Molten stone core of the Lava Guardian. Unshakable defense and vitality.",
     4, 250, 0, 0, 45, 0, 0, 20, 0, 0, 0, 0, 0}
  };
  return catalog;
}

inline const BossDropInfo* findBossDrop(int species) {
  for (const auto& item : bossDropCatalog()) {
    if (item.species == species) return &item;
  }
  return nullptr;
}

// ============================================================================
// 2. SETS DE EQUIPAMENTO EXPANDIDOS (2, 3, 4, 5 e 6 peças equipadas)
//    Origens: Sets Concentrados vs Sets Distribuídos
// ============================================================================
enum class SetDropSourceType { Concentrated, Distributed };

struct SetBonus {
  int piecesRequired; // 2, 3, 4, 5 ou 6
  std::string namePt;
  std::string nameEn;
  std::string descPt;
  std::string descEn;
  int atk = 0;
  int def = 0;
  int hp = 0;
  int mp = 0;
  int str = 0;
  int dex = 0;
  int vit = 0;
  int intStat = 0;
  int critChance = 0;
  int critDmg = 0;
  int evasion = 0;
  int lifeSteal = 0;
  int mpCostReduction = 0; // %
};

struct EquipmentSet {
  std::string id;
  std::string namePt;
  std::string nameEn;
  std::vector<std::string> patterns; // Palavras-chave no nome do item
  SetDropSourceType dropType;
  int primaryBoss; // species se Concentrado, ou -1 se Distribuído
  std::vector<int> bossSources; // Lista de chefes de onde cai
  bool eliteSource; // Cai de monstros elites?
  std::string farmRoutePt;
  std::string farmRouteEn;
  SetBonus bonus2;
  SetBonus bonus3;
  SetBonus bonus4;
  SetBonus bonus5;
  SetBonus bonus6;
};

inline const std::vector<EquipmentSet>& equipmentSetCatalog() {
  static const std::vector<EquipmentSet> catalog = {
    // 1. Set do Dragão (Concentrado: Boss 108 Diablo / 109 Lava Guardian)
    {"dragon", "Set do Dragao", "Dragon Set", {"dragao", "dragon"},
     SetDropSourceType::Concentrated, 108, {108, 109}, false,
     "Drop Concentrado: As 6 pecas caem exclusivamente de Diablo e Lava Guardian.",
     "Concentrated Drop: All 6 pieces drop exclusively from Diablo and Lava Guardian.",
     {2, "Garras Draconicas", "Dragon Claws", "+30 Ataque, +20 Forca", "+30 Attack, +20 Strength", 30, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0},
     {3, "Escamas de Dragao", "Dragon Scales", "+35 Defesa, +150 HP", "+35 Defense, +150 HP", 0, 35, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {4, "Coracao Draconico", "Dragon Heart", "+300 HP, +15% Dano Critico", "+300 HP, +15% Crit Damage", 0, 0, 300, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0},
     {5, "Olhar do Dragao", "Dragon Gaze", "+10% Chance Critica, +35 Forca", "+10% Crit Chance, +35 Strength", 0, 0, 0, 0, 35, 0, 0, 0, 10, 0, 0, 0, 0},
     {6, "Furia Draconica Suprema", "Supreme Dragon Fury", "+100 Ataque, +50 Forca, +25% Dano Critico, +500 HP", "+100 Attack, +50 Strength, +25% Crit Damage, +500 HP", 100, 0, 500, 0, 50, 0, 0, 0, 0, 25, 0, 0, 0}},

    // 2. Set Sombrio (Distribuído: Defcan 101, Defcan Desperto 103, Salas 102, Sileth 104 e Elites)
    {"shadow", "Set Sombrio", "Shadow Set", {"sombri", "shadow", "sombra"},
     SetDropSourceType::Distributed, -1, {101, 102, 103, 104}, true,
     "Drop Distribuido: Pecas espalhadas entre Defcan, Salas, Sileth e Monstros Elites.",
     "Distributed Drop: Pieces spread across Defcan, Salas, Sileth and Elite Monsters.",
     {2, "Passos das Sombras", "Shadow Steps", "+25 Evasao, +20 Destreza", "+25 Evasion, +20 Dexterity", 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 25, 0, 0},
     {3, "Capa da Meia-Noite", "Midnight Cloak", "+25 Defesa, +15 Destreza", "+25 Defense, +15 Dexterity", 0, 25, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0},
     {4, "Golpe Furtivo", "Stealth Strike", "+250 HP, +10% Chance Critica", "+250 HP, +10% Crit Chance", 0, 0, 250, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0},
     {5, "Lamina Oculta", "Hidden Blade", "+20% Dano Critico, +10% Vampirismo", "+20% Crit Damage, +10% Lifesteal", 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 10, 0},
     {6, "Avatar das Sombras", "Shadow Avatar", "+60 Evasao, +50 Destreza, +15% Critico, +15% Vampirismo", "+60 Evasion, +50 Dexterity, +15% Crit, +15% Lifesteal", 0, 0, 0, 0, 0, 50, 0, 0, 15, 0, 60, 15, 0}},

    // 3. Set Imperial (Distribuído: Kaiser 106, Baramos 107 e Elites)
    {"imperial", "Set Imperial", "Imperial Set", {"imperial", "kaiser"},
     SetDropSourceType::Distributed, -1, {106, 107}, true,
     "Drop Distribuido: Pecas obtidas de Kaiser, Baramos e Masmorras de Elite.",
     "Distributed Drop: Pieces obtained from Kaiser, Baramos and Elite Dungeons.",
     {2, "Vontade Soberana", "Sovereign Will", "+35 Defesa, +20 Vitalidade", "+35 Defense, +20 Vitality", 0, 35, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0},
     {3, "Guarda Imperial", "Imperial Guard", "+30 Defesa, +200 HP", "+30 Defense, +200 HP", 0, 30, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {4, "Bastiao Imperial", "Imperial Bastion", "+400 HP, +15% Reducao de Dano", "+400 HP, +15% Damage Reduction", 0, 40, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {5, "Linhagem Real", "Royal Lineage", "+35 Vitalidade, +20 Regeneracao de HP", "+35 Vitality, +20 HP Regen", 0, 0, 200, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0},
     {6, "Soberania Absoluta", "Absolute Sovereignty", "+100 Defesa, +50 Vitalidade, +600 HP, +25% Reducao de Dano", "+100 Defense, +50 Vitality, +600 HP, +25% Damage Reduction", 0, 100, 600, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0}},

    // 4. Set Arcano (Distribuído: Defcan Desperto 103, Sileth 104 e Monstros Elites Mágicos)
    {"arcane", "Set Arcano", "Arcane Set", {"arcan", "arcane", "mago", "mage"},
     SetDropSourceType::Distributed, -1, {103, 104}, true,
     "Drop Distribuido: Pecas dropadas por Defcan Desperto, Sileth e Elites Magicos.",
     "Distributed Drop: Pieces dropped by Awakened Defcan, Sileth and Magic Elites.",
     {2, "Chama Arcana", "Arcane Flame", "+35 Inteligencia, +150 MP", "+35 Intelligence, +150 MP", 0, 0, 0, 150, 0, 0, 0, 35, 0, 0, 0, 0, 0},
     {3, "Escudo Magico", "Magic Shield", "+25 Defesa, +150 MP", "+25 Defense, +150 MP", 0, 25, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {4, "Mente Transcendente", "Transcendent Mind", "+350 MP, -20% Custo de MP", "+350 MP, -20% MP Cost", 0, 0, 0, 350, 0, 0, 0, 0, 0, 0, 0, 0, 20},
     {5, "Foco Prismatico", "Prismatic Focus", "+12% Chance Critica Magica, +40 Inteligencia", "+12% Magic Crit Chance, +40 Intelligence", 0, 0, 0, 0, 0, 0, 0, 40, 12, 0, 0, 0, 0},
     {6, "Arquimago Supremo", "Supreme Archmage", "+80 Inteligencia, +500 MP, -35% Custo de MP em Habilidades", "+80 Intelligence, +500 MP, -35% MP Cost on Skills", 0, 0, 0, 500, 0, 0, 0, 80, 0, 0, 0, 0, 35}},

    // 5. Set do Guardião (Concentrado: Boss 100 Dungeon Guardian)
    {"guardian", "Set do Guardiao", "Guardian Set", {"guardiao", "guardian"},
     SetDropSourceType::Concentrated, 100, {100}, false,
     "Drop Concentrado: As 6 pecas caem 100% exclusivamente do Dungeon Guardian.",
     "Concentrated Drop: All 6 pieces drop 100% exclusively from Dungeon Guardian.",
     {2, "Muralha de Pedra", "Stone Wall", "+35 Defesa, +200 HP", "+35 Defense, +200 HP", 0, 35, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {3, "Escudo de Granito", "Granite Shield", "+30 Defesa, +20 Vitalidade", "+30 Defense, +20 Vitality", 0, 30, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0},
     {4, "Corpo de Rocha", "Body of Rock", "+450 HP, +20% Resistencia a Atordoamento", "+450 HP, +20% Stun Resistance", 0, 0, 450, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {5, "Forca da Terra", "Strength of Earth", "+50 Defesa, +25 Regeneracao de HP", "+50 Defense, +25 HP Regen", 0, 50, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {6, "Colosso Inabalavel", "Unshakable Colossus", "+90 Defesa, +600 HP, +30% Resistencia Geral, +30 Regeneracao de HP", "+90 Defense, +600 HP, +30% Overall Resistance, +30 HP Regen", 0, 90, 600, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0}}
  };
  return catalog;
}

inline std::string toLowerAscii(const std::string& str) {
  std::string lower;
  lower.reserve(str.size());
  for (char ch : str) {
    if (ch >= 'A' && ch <= 'Z') lower.push_back(char(ch + ('a' - 'A')));
    else lower.push_back(ch);
  }
  return lower;
}

inline int identifySet(const std::string& itemName) {
  const std::string lower = toLowerAscii(itemName);
  const auto& sets = equipmentSetCatalog();
  for (size_t i = 0; i < sets.size(); i++) {
    for (const auto& pattern : sets[i].patterns) {
      if (lower.find(pattern) != std::string::npos) return int(i);
    }
  }
  return -1;
}

struct ActiveSetSummary {
  std::string setId;
  std::string setNamePt;
  std::string setNameEn;
  int piecesEquipped = 0;
  bool bonus2Active = false;
  bool bonus3Active = false;
  bool bonus4Active = false;
  bool bonus5Active = false;
  bool bonus6Active = false;
  std::vector<SetBonus> activeBonuses;
};

inline std::vector<ActiveSetSummary> evaluateActiveSets(const std::vector<std::string>& equippedNames) {
  const auto& sets = equipmentSetCatalog();
  std::vector<int> counts(sets.size(), 0);
  for (const auto& name : equippedNames) {
    int idx = identifySet(name);
    if (idx >= 0 && idx < int(sets.size())) counts[size_t(idx)]++;
  }
  std::vector<ActiveSetSummary> summaries;
  for (size_t i = 0; i < sets.size(); i++) {
    int count = counts[i];
    if (count >= 2) {
      ActiveSetSummary s;
      s.setId = sets[i].id;
      s.setNamePt = sets[i].namePt;
      s.setNameEn = sets[i].nameEn;
      s.piecesEquipped = count;
      if (count >= 2) { s.bonus2Active = true; s.activeBonuses.push_back(sets[i].bonus2); }
      if (count >= 3) { s.bonus3Active = true; s.activeBonuses.push_back(sets[i].bonus3); }
      if (count >= 4) { s.bonus4Active = true; s.activeBonuses.push_back(sets[i].bonus4); }
      if (count >= 5) { s.bonus5Active = true; s.activeBonuses.push_back(sets[i].bonus5); }
      if (count >= 6) { s.bonus6Active = true; s.activeBonuses.push_back(sets[i].bonus6); }
      summaries.push_back(s);
    }
  }
  return summaries;
}

// ============================================================================
// 3. CASH OFFLINE (Ganho por tempo offline via timestamp, teto estrito de 24h)
// ============================================================================
constexpr int64_t MAX_OFFLINE_SECONDS = 86400; // 24 Horas Estritas (60*60*24)
constexpr int64_t DEFAULT_OFFLINE_CASH_INTERVAL = 1800; // 1 Cash a cada 30 min (máx 48 Cash em 24h)

struct OfflineCashResult {
  int cashToCredit = 0;
  int64_t elapsedSeconds = 0;
  int64_t cappedSeconds = 0;
  bool capped = false;
};

inline OfflineCashResult calculateOfflineCash(
    int64_t lastTimestamp,
    int64_t currentTimestamp,
    int64_t intervalSeconds = DEFAULT_OFFLINE_CASH_INTERVAL) {
  OfflineCashResult res{};
  if (lastTimestamp <= 0 || currentTimestamp <= lastTimestamp || intervalSeconds <= 0) {
    return res;
  }
  res.elapsedSeconds = currentTimestamp - lastTimestamp;
  res.capped = res.elapsedSeconds > MAX_OFFLINE_SECONDS;
  res.cappedSeconds = std::min(res.elapsedSeconds, MAX_OFFLINE_SECONDS);
  res.cashToCredit = int(res.cappedSeconds / intervalSeconds);
  return res;
}

// ============================================================================
// 4. MISSÕES ÉPICAS DE CASH (Calibradas com a loja nativa de 20 Cash por item)
// ============================================================================
enum class MissionTargetType { BossKills, FullSetEquipped };

struct EpicMission {
  std::string id;
  std::string titlePt;
  std::string titleEn;
  std::string descPt;
  std::string descEn;
  MissionTargetType targetType;
  int targetCount;
  int cashReward; // Calibrado com os preços da loja nativa (.gdt = ~20 Cash por item)
};

inline const std::vector<EpicMission>& epicMissionsCatalog() {
  static const std::vector<EpicMission> catalog = {
    {"boss_slayer_10", "Matador de Chefes I", "Boss Slayer I",
     "Derrote 10 chefes de qualquer masmorra.", "Defeat 10 dungeon bosses.",
     MissionTargetType::BossKills, 10, 25},

    {"boss_slayer_25", "Matador de Chefes II", "Boss Slayer II",
     "Derrote 25 chefes em sua jornada.", "Defeat 25 dungeon bosses.",
     MissionTargetType::BossKills, 25, 50},

    {"boss_slayer_50", "Conquistador de Masmorras", "Dungeon Conqueror",
     "Derrote 50 chefes lendarios.", "Defeat 50 legendary bosses.",
     MissionTargetType::BossKills, 50, 75},

    {"boss_slayer_100", "Lenda dos Chefes", "Boss Legend",
     "Derrote 100 chefes supremos do jogo.", "Defeat 100 supreme bosses of the game.",
     MissionTargetType::BossKills, 100, 100},

    {"full_set_equipped", "Conjunto Perfeito", "Flawless Set",
     "Equipe as 6 pecas de um conjunto de equipamento lendario.",
     "Equip all 6 pieces of a legendary equipment set.",
     MissionTargetType::FullSetEquipped, 6, 50}
  };
  return catalog;
}

// ============================================================================
// 5. ESTADO DE ECONOMIA E SIDECAR (Atomic, save*.dat intocados)
// ============================================================================
class LootEconomyState {
public:
  int characterSlot = 0;
  int64_t lastTimestamp = 0;
  int totalOfflineCashAwarded = 0;
  int totalMissionCashAwarded = 0;
  // (slot, species) -> drop count
  std::map<std::pair<int, int>, int> bossDrops;
  // (slot, species) -> kill count
  std::map<std::pair<int, int>, int> bossKills;
  // (slot, missionId)
  std::set<std::pair<int, std::string>> completedMissions;

  void select(int slot) {
    if (slot < 0 || slot > 9) throw std::runtime_error("PERSONAGEM INVALIDO");
    characterSlot = slot;
  }

  void recordBossKill(int slot, int species) {
    if (slot < 0 || slot > 9) throw std::runtime_error("PERSONAGEM INVALIDO");
    if (species < 100 || species > 109) return;
    bossKills[{slot, species}]++;
    bossDrops[{slot, species}]++;
  }

  int getBossDropCount(int slot, int species) const {
    auto it = bossDrops.find({slot, species});
    return it != bossDrops.end() ? it->second : 0;
  }

  int getBossKillCount(int slot, int species) const {
    auto it = bossKills.find({slot, species});
    return it != bossKills.end() ? it->second : 0;
  }

  int getTotalBossKills(int slot) const {
    int total = 0;
    for (int sp = 100; sp <= 109; sp++) {
      auto it = bossKills.find({slot, sp});
      if (it != bossKills.end()) total += it->second;
    }
    return total;
  }

  int checkAndClaimEpicMissions(int slot, int maxSetPiecesEquipped = 0) {
    int claimedCash = 0;
    int kills = getTotalBossKills(slot);
    for (const auto& mission : epicMissionsCatalog()) {
      if (completedMissions.count({slot, mission.id})) continue;
      bool completed = false;
      if (mission.targetType == MissionTargetType::BossKills) {
        completed = kills >= mission.targetCount;
      } else if (mission.targetType == MissionTargetType::FullSetEquipped) {
        completed = maxSetPiecesEquipped >= mission.targetCount;
      }
      if (completed) {
        completedMissions.insert({slot, mission.id});
        claimedCash += mission.cashReward;
        totalMissionCashAwarded += mission.cashReward;
      }
    }
    return claimedCash;
  }

  OfflineCashResult processOfflineCash(int64_t nowTimestamp, int64_t intervalSeconds = DEFAULT_OFFLINE_CASH_INTERVAL) {
    auto res = calculateOfflineCash(lastTimestamp, nowTimestamp, intervalSeconds);
    if (res.cashToCredit > 0) {
      totalOfflineCashAwarded += res.cashToCredit;
    }
    lastTimestamp = nowTimestamp;
    return res;
  }

  std::string encode() const {
    std::ostringstream out;
    out << "DESTINIA_ECONOMY_2\n";
    out << "timestamp " << lastTimestamp << '\n';
    out << "total_offline_cash " << totalOfflineCashAwarded << '\n';
    out << "total_mission_cash " << totalMissionCashAwarded << '\n';
    for (const auto& entry : bossDrops) {
      out << "drop " << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << '\n';
    }
    for (const auto& entry : bossKills) {
      out << "kill " << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << '\n';
    }
    for (const auto& entry : completedMissions) {
      out << "mission " << entry.first << ' ' << entry.second << '\n';
    }
    return out.str();
  }

  void decode(const std::string& text) {
    if (text.empty() || text.size() > 100000 || text.back() != '\n') {
      throw std::runtime_error("SIDECAR INVALIDO");
    }
    std::istringstream in(text);
    std::string line;
    std::getline(in, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line != "DESTINIA_ECONOMY_1" && line != "DESTINIA_ECONOMY_2") {
      throw std::runtime_error("FORMATO DE ECONOMIA DESCONHECIDO");
    }
    int64_t nextTimestamp = 0;
    int nextOfflineCash = 0;
    int nextMissionCash = 0;
    decltype(bossDrops) nextDrops;
    decltype(bossKills) nextKills;
    decltype(completedMissions) nextMissions;

    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty()) continue;
      std::istringstream row(line);
      std::string tag;
      if (!(row >> tag)) continue;
      if (tag == "timestamp") {
        row >> nextTimestamp;
      } else if (tag == "total_cash" || tag == "total_offline_cash") {
        row >> nextOfflineCash;
      } else if (tag == "total_mission_cash") {
        row >> nextMissionCash;
      } else if (tag == "drop") {
        int slot = 0, species = 0, count = 0;
        if (row >> slot >> species >> count) {
          if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
            nextDrops[{slot, species}] = count;
          }
        }
      } else if (tag == "kill") {
        int slot = 0, species = 0, count = 0;
        if (row >> slot >> species >> count) {
          if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
            nextKills[{slot, species}] = count;
          }
        }
      } else if (tag == "mission") {
        int slot = 0;
        std::string missionId;
        if (row >> slot >> missionId) {
          if (slot >= 0 && slot <= 9 && !missionId.empty()) {
            nextMissions.insert({slot, missionId});
          }
        }
      }
    }
    lastTimestamp = nextTimestamp;
    totalOfflineCashAwarded = nextOfflineCash;
    totalMissionCashAwarded = nextMissionCash;
    bossDrops = std::move(nextDrops);
    bossKills = std::move(nextKills);
    completedMissions = std::move(nextMissions);
  }
};

} // namespace destinia
