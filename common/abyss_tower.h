#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include "safe_save.h"
#include "overlay_font.h"

namespace destinia {

struct AbyssFloor {
  int floor;
  int recommendedLevel;
  int goldCost;
  int rewardGold;
  int area;
  int map;
  bool isBoss;
  int bossSpecies;
  const char* titlePt;
  const char* titleEn;
};

inline const std::vector<AbyssFloor>& getAbyssFloors() {
  static const std::vector<AbyssFloor> floors = []() {
    std::vector<AbyssFloor> list;
    list.reserve(50);
    for (int f = 1; f <= 50; ++f) {
      bool isBoss = (f % 5 == 0);
      int bossIndex = isBoss ? ((f / 5) - 1) % 10 : -1;
      int species = (bossIndex >= 0) ? (100 + bossIndex) : 0;
      
      int area = (f % 2 == 0) ? 2 : 1;
      int map = 5 + (f % 45);

      static const char* bossNamesPt[10] = {
        "Guardiao das Sombras", "Defcan Corrompido", "Salas do Abismo", "Defcan Desperto",
        "Sileth Espiritual", "Tamur Enfurecido", "Imperador Espectral", "Baramos das Trevas",
        "Avatar de Diablo", "Guardiao Primordial"
      };

      const char* name = isBoss ? bossNamesPt[bossIndex] : "Camara dos Renegados";

      list.push_back({
        f,
        f * 2,
        f * 150,
        f * 600,
        area,
        map,
        isBoss,
        species,
        name,
        name
      });
    }
    return list;
  }();
  return floors;
}

class AbyssTowerManager {
public:
  int maxUnlockedFloor = 1;
  int currentFloor = 1;
  int totalClears = 0;

  void load(const fs::path& saveDir) {
    fs::path p = saveDir / "abyss-v1.txt";
    if (!fs::exists(p)) return;
    std::ifstream in(p);
    if (!in) return;
    std::string tag;
    if (!(in >> tag) || tag != "DESTINIA_ABYSS_V1") return;
    std::string key;
    while (in >> key) {
      if (key == "max_unlocked") {
        in >> maxUnlockedFloor;
        maxUnlockedFloor = std::clamp(maxUnlockedFloor, 1, 50);
      } else if (key == "total_clears") {
        in >> totalClears;
      } else if (key == "end") {
        break;
      }
    }
  }

  void save(const fs::path& saveDir) const {
    try {
      fs::create_directories(saveDir);
      fs::path p = saveDir / "abyss-v1.txt";
      fs::path temp = saveDir / "abyss-v1.tmp";
      std::ofstream out(temp);
      if (!out) return;
      out << "DESTINIA_ABYSS_V1\n";
      out << "max_unlocked " << maxUnlockedFloor << "\n";
      out << "total_clears " << totalClears << "\n";
      out << "end\n";
      out.close();
      fs::rename(temp, p);
    } catch (...) {}
  }

  void onFloorCleared(int floor, const fs::path& saveDir) {
    if (floor >= maxUnlockedFloor && maxUnlockedFloor < 50) {
      maxUnlockedFloor = floor + 1;
    }
    totalClears++;
    save(saveDir);
  }
};

} // namespace destinia
