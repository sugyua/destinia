#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <ctime>

namespace destinia {

enum class BountyType : uint8_t {
  HuntMonsters = 0,
  HuntBoss = 1,
  AccumulateGold = 2
};

struct BountyContract {
  uint16_t id = 0;
  BountyType type = BountyType::HuntMonsters;
  uint16_t targetId = 0;      // Espécie do monstro ou chefe
  uint16_t targetCount = 10;
  uint16_t currentCount = 0;
  uint32_t rewardGold = 10000;
  uint16_t rewardCash = 250;
  bool completed = false;
  bool claimed = false;
  std::string title;
  std::string description;
};

class BountyManager {
public:
  // slot -> 3 contratos ativos
  std::map<unsigned, std::vector<BountyContract>> contractsBySlot;

  std::vector<BountyContract>& getContracts(unsigned slot) {
    if (contractsBySlot[slot].empty()) {
      generateDailyContracts(slot);
    }
    return contractsBySlot[slot];
  }

  void generateDailyContracts(unsigned slot) {
    auto& list = contractsBySlot[slot];
    list.clear();

    // Contrato 1: Caça Comum (ex: 15 Soldados Imperiais)
    BountyContract c1;
    c1.id = 1;
    c1.type = BountyType::HuntMonsters;
    c1.targetId = 2; // Imperial Soldier
    c1.targetCount = 15;
    c1.currentCount = 0;
    c1.rewardGold = 15000;
    c1.rewardCash = 200;
    c1.title = "Caca: Soldados Imperiais";
    c1.description = "Elimine 15 Soldados Imperiais nas estradas.";
    list.push_back(c1);

    // Contrato 2: Caça a Chefe (ex: Diablo species 108 ou outro boss 100..109)
    BountyContract c2;
    c2.id = 2;
    c2.type = BountyType::HuntBoss;
    c2.targetId = 108; // Diablo
    c2.targetCount = 1;
    c2.currentCount = 0;
    c2.rewardGold = 35000;
    c2.rewardCash = 500;
    c2.title = "Bounty: Diablo, Senhor do Caos";
    c2.description = "Derrote o Boss Diablo no Boss Rush ou mapa.";
    list.push_back(c2);

    // Contrato 3: Desafio de Caça Épica (ex: 20 Espectros ou Minotauros)
    BountyContract c3;
    c3.id = 3;
    c3.type = BountyType::HuntMonsters;
    c3.targetId = 105; // Boss ou Monstro Especial
    c3.targetCount = 1;
    c3.currentCount = 0;
    c3.rewardGold = 25000;
    c3.rewardCash = 350;
    c3.title = "Bounty: O Guardiao Ancestral";
    c3.description = "Derrote o Guardiao Ancestral (Boss 105).";
    list.push_back(c3);
  }

  void onMonsterKilled(unsigned slot, uint16_t species) {
    if (slot >= 10) return;
    auto& list = getContracts(slot);
    for (auto& c : list) {
      if (!c.completed && c.targetId == species) {
        c.currentCount++;
        if (c.currentCount >= c.targetCount) {
          c.currentCount = c.targetCount;
          c.completed = true;
        }
      }
    }
  }

  std::string serialize() const {
    std::ostringstream ss;
    ss << "bounties_version 1\n";
    for (const auto& [slot, list] : contractsBySlot) {
      for (const auto& c : list) {
        ss << "bounty " << slot << " " << c.id << " " << unsigned(c.type) << " "
           << c.targetId << " " << c.targetCount << " " << c.currentCount << " "
           << c.rewardGold << " " << c.rewardCash << " " << (c.completed ? 1 : 0)
           << " " << (c.claimed ? 1 : 0) << "\n";
      }
    }
    return ss.str();
  }

  static BountyManager deserialize(const std::string& text) {
    BountyManager mgr;
    std::istringstream ss(text);
    std::string line;
    bool valid = false;

    while (std::getline(ss, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream ls(line);
      std::string tag;
      ls >> tag;

      if (tag == "bounties_version") {
        int v = 0;
        ls >> v;
        if (v == 1) valid = true;
      } else if (valid && tag == "bounty") {
        unsigned slot = 0, id = 0, type = 0, comp = 0, claimed = 0;
        BountyContract c;
        ls >> slot >> id >> type >> c.targetId >> c.targetCount >> c.currentCount
           >> c.rewardGold >> c.rewardCash >> comp >> claimed;
        c.id = id;
        c.type = BountyType(type);
        c.completed = (comp != 0);
        c.claimed = (claimed != 0);

        if (c.id == 1) {
          c.title = "Caca: Soldados Imperiais";
          c.description = "Elimine 15 Soldados Imperiais nas estradas.";
        } else if (c.id == 2) {
          c.title = "Bounty: Diablo, Senhor do Caos";
          c.description = "Derrote o Boss Diablo no Boss Rush ou mapa.";
        } else {
          c.title = "Bounty: O Guardiao Ancestral";
          c.description = "Derrote o Guardiao Ancestral (Boss 105).";
        }

        mgr.contractsBySlot[slot].push_back(c);
      }
    }
    return mgr;
  }
};

inline std::filesystem::path bountiesFilePath(const std::filesystem::path& dir) {
  return dir / "bounties-v1.txt";
}

inline BountyManager loadBounties(const std::filesystem::path& dir) {
  std::filesystem::path p = bountiesFilePath(dir);
  if (!std::filesystem::exists(p)) return BountyManager();
  std::ifstream f(p, std::ios::binary);
  if (!f.is_open()) return BountyManager();
  std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return BountyManager::deserialize(content);
}

inline bool saveBounties(const std::filesystem::path& dir, const BountyManager& mgr) {
  try {
    std::filesystem::create_directories(dir);
    std::filesystem::path target = bountiesFilePath(dir);
    std::filesystem::path tmp = target;
    tmp += ".tmp";

    {
      std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
      if (!f.is_open()) return false;
      std::string data = mgr.serialize();
      f.write(data.data(), data.size());
      f.flush();
      if (!f.good()) return false;
    }

    std::filesystem::rename(tmp, target);
    return true;
  } catch (...) {
    return false;
  }
}

} // namespace destinia
