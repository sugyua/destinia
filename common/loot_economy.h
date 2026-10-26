#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
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
// 2. SETS DE EQUIPAMENTO (3 a 5 Conjuntos com bônus de 2 e 4 peças)
// ============================================================================
struct SetBonus {
  int piecesRequired; // 2 ou 4
  std::string namePt;
  std::string nameEn;
  std::string descPt;
  std::string descEn;
  int atk;
  int def;
  int hp;
  int mp;
  int str;
  int dex;
  int vit;
  int intStat;
  int critChance;
  int critDmg;
  int evasion;
  int lifeSteal;
  int mpCostReduction; // %
};

struct EquipmentSet {
  std::string id;
  std::string namePt;
  std::string nameEn;
  std::vector<std::string> patterns; // Palavras-chave no nome do item
  SetBonus bonus2;
  SetBonus bonus4;
};

inline const std::vector<EquipmentSet>& equipmentSetCatalog() {
  static const std::vector<EquipmentSet> catalog = {
    // 1. Set do Dragão
    {"dragon", "Set do Dragao", "Dragon Set", {"dragao", "dragon"},
     {2, "Furia Draconica Menor", "Lesser Dragon Fury",
      "+25 Ataque, +15 Forca", "+25 Attack, +15 Strength",
      25, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0},
     {4, "Poder Draconico Ancestral", "Ancient Dragon Power",
      "+60 Ataque, +30 Forca, +10% Dano Critico, +200 HP", "+60 Attack, +30 Strength, +10% Crit Damage, +200 HP",
      60, 0, 200, 0, 30, 0, 0, 0, 0, 10, 0, 0, 0}},

    // 2. Set Sombrio
    {"shadow", "Set Sombrio", "Shadow Set", {"sombri", "shadow", "sombra"},
     {2, "Manto Noturno", "Night Shroud",
      "+20 Evasao, +15 Destreza", "+20 Evasion, +15 Dexterity",
      0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 20, 0, 0},
     {4, "Mestre das Sombras", "Shadow Master",
      "+40 Evasao, +30 Destreza, +12% Critico, +10% Vampirismo", "+40 Evasion, +30 Dexterity, +12% Crit, +10% Lifesteal",
      0, 0, 0, 0, 0, 30, 0, 0, 12, 0, 40, 10, 0}},

    // 3. Set Imperial
    {"imperial", "Set Imperial", "Imperial Set", {"imperial", "kaiser"},
     {2, "Vontade de Ferro", "Iron Will",
      "+30 Defesa, +15 Vitalidade", "+30 Defense, +15 Vitality",
      0, 30, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0},
     {4, "Soberania Absoluta", "Absolute Sovereignty",
      "+70 Defesa, +35 Vitalidade, +350 HP, +15% Reducao de Dano", "+70 Defense, +35 Vitality, +350 HP, +15% Damage Reduction",
      0, 70, 350, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0}},

    // 4. Set Arcano
    {"arcane", "Set Arcano", "Arcane Set", {"arcan", "arcane", "mago", "mage"},
     {2, "Fluxo Etereo", "Ethereal Flow",
      "+30 Inteligencia, +120 MP", "+30 Intelligence, +120 MP",
      0, 0, 0, 120, 0, 0, 0, 30, 0, 0, 0, 0, 0},
     {4, "Conhecimento Primordial", "Primordial Knowledge",
      "+65 Inteligencia, +250 MP, -20% Custo MP", "+65 Intelligence, +250 MP, -20% MP Cost",
      0, 0, 0, 250, 0, 0, 0, 65, 0, 0, 0, 0, 20}},

    // 5. Set do Guardião
    {"guardian", "Set do Guardiao", "Guardian Set", {"guardiao", "guardian"},
     {2, "Bastiao Protetor", "Protective Bastion",
      "+25 Defesa, +150 HP", "+25 Defense, +150 HP",
      0, 25, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {4, "Fortaleza Viva", "Living Fortress",
      "+60 Defesa, +300 HP, +20% Resistencia", "+60 Defense, +300 HP, +20% Resistance",
      0, 60, 300, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0}}
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
  bool bonus4Active = false;
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
      s.bonus2Active = true;
      s.activeBonuses.push_back(sets[i].bonus2);
      if (count >= 4) {
        s.bonus4Active = true;
        s.activeBonuses.push_back(sets[i].bonus4);
      }
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
// 4. ESTADO DE ECONOMIA E SIDECAR (Atomic, save*.dat intocados)
// ============================================================================
class LootEconomyState {
public:
  int characterSlot = 0;
  int64_t lastTimestamp = 0;
  int totalOfflineCashAwarded = 0;
  // (slot, species) -> drop count
  std::map<std::pair<int, int>, int> bossDrops;

  void select(int slot) {
    if (slot < 0 || slot > 9) throw std::runtime_error("PERSONAGEM INVALIDO");
    characterSlot = slot;
  }

  void recordBossKill(int slot, int species) {
    if (slot < 0 || slot > 9) throw std::runtime_error("PERSONAGEM INVALIDO");
    if (species < 100 || species > 109) return;
    bossDrops[{slot, species}]++;
  }

  int getBossDropCount(int slot, int species) const {
    auto it = bossDrops.find({slot, species});
    return it != bossDrops.end() ? it->second : 0;
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
    out << "DESTINIA_ECONOMY_1\n";
    out << "timestamp " << lastTimestamp << '\n';
    out << "total_cash " << totalOfflineCashAwarded << '\n';
    for (const auto& entry : bossDrops) {
      out << "drop " << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << '\n';
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
    if (line != "DESTINIA_ECONOMY_1") {
      throw std::runtime_error("FORMATO DE ECONOMIA DESCONHECIDO");
    }
    int64_t nextTimestamp = 0;
    int nextCash = 0;
    decltype(bossDrops) nextDrops;
    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty()) continue;
      std::istringstream row(line);
      std::string tag;
      if (!(row >> tag)) continue;
      if (tag == "timestamp") {
        row >> nextTimestamp;
      } else if (tag == "total_cash") {
        row >> nextCash;
      } else if (tag == "drop") {
        int slot = 0, species = 0, count = 0;
        if (row >> slot >> species >> count) {
          if (slot >= 0 && slot <= 9 && species >= 100 && species <= 109 && count >= 0) {
            nextDrops[{slot, species}] = count;
          }
        }
      }
    }
    lastTimestamp = nextTimestamp;
    totalOfflineCashAwarded = nextCash;
    bossDrops = std::move(nextDrops);
  }
};

} // namespace destinia
