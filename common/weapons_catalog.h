#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

namespace destinia {

enum class WeaponRarity : uint8_t {
  Common = 0,    // Branca
  Magic = 1,     // Verde
  Rare = 2,      // Azul
  Epic = 3,      // Amarela
  Legendary = 4  // Laranja / Lendaria
};

struct WeaponEntry {
  uint16_t baseId;    // ID base da arma (ex: 1001)
  uint8_t level;      // Nivel de requerimento da arma
  uint8_t rarity;     // 0..4 (WeaponRarity)
  uint8_t jobClass;   // 0: Qualquer / 1: Guerreiro / 2: Atirador / 3: Ladrao

  bool operator<(const WeaponEntry& o) const {
    if (baseId != o.baseId) return baseId < o.baseId;
    if (rarity != o.rarity) return rarity < o.rarity;
    return level < o.level;
  }
  bool operator==(const WeaponEntry& o) const {
    return baseId == o.baseId && rarity == o.rarity && level == o.level;
  }
};

class WeaponsCatalog {
public:
  // slot -> set de WeaponEntry registradas
  std::map<unsigned, std::set<WeaponEntry>> catalog;

  // Registra uma arma obtida sem explodir combinatoria de afixos
  bool record(unsigned slot, uint16_t baseId, uint8_t level, uint8_t rarity, uint8_t jobClass) {
    if (slot >= 10 || rarity > 4) return false;
    WeaponEntry entry{baseId, level, rarity, jobClass};
    return catalog[slot].insert(entry).second;
  }

  bool isRecorded(unsigned slot, uint16_t baseId, uint8_t rarity) const {
    auto it = catalog.find(slot);
    if (it == catalog.end()) return false;
    for (const auto& w : it->second) {
      if (w.baseId == baseId && w.rarity == rarity) return true;
    }
    return false;
  }

  size_t count(unsigned slot) const {
    auto it = catalog.find(slot);
    return it != catalog.end() ? it->second.size() : 0;
  }

  // Percentual de conclusao para a classe informada (jobClass: 1=Guerreiro, 2=Atirador, 3=Ladrao)
  float completionPercentage(unsigned slot, uint8_t jobClass, size_t totalExpectedInClass) const {
    if (totalExpectedInClass == 0) return 100.0f;
    auto it = catalog.find(slot);
    if (it == catalog.end()) return 0.0f;
    size_t countClass = 0;
    for (const auto& w : it->second) {
      if (w.jobClass == jobClass || w.jobClass == 0) {
        countClass++;
      }
    }
    return std::min(100.0f, (float(countClass) / float(totalExpectedInClass)) * 100.0f);
  }

  void eraseSlot(unsigned slot) {
    catalog.erase(slot);
  }

  // Serializacao em texto para sidecar weapons-catalog-v1.txt
  std::string serialize() const {
    std::ostringstream ss;
    ss << "version 1\n";
    for (const auto& [slot, weapons] : catalog) {
      for (const auto& w : weapons) {
        ss << "slot " << slot << " " << w.baseId << " " << unsigned(w.level) << " " << unsigned(w.rarity) << " " << unsigned(w.jobClass) << "\n";
      }
    }
    return ss.str();
  }

  // Desserializacao a partir do sidecar
  static WeaponsCatalog deserialize(const std::string& content) {
    WeaponsCatalog result;
    std::istringstream ss(content);
    std::string line;
    bool hasHeader = false;
    while (std::getline(ss, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream ls(line);
      std::string tag;
      ls >> tag;
      if (tag == "version") {
        int v = 0;
        ls >> v;
        if (v != 1) return result;
        hasHeader = true;
      } else if (tag == "slot" && hasHeader) {
        unsigned slot = 0;
        uint16_t baseId = 0;
        unsigned level = 0, rarity = 0, jobClass = 0;
        if (ls >> slot >> baseId >> level >> rarity >> jobClass) {
          result.record(slot, baseId, uint8_t(level), uint8_t(rarity), uint8_t(jobClass));
        }
      }
    }
    return result;
  }
};

} // namespace destinia
