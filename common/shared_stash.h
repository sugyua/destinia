#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

namespace destinia {

struct StashItem {
  uint16_t pool = 0;
  uint16_t count = 1;
  uint16_t durability = 100;
  uint8_t itemType = 0; // 0: Arma, 1: Armadura, 2: Elmo, 3: Luva, 4: Bota, 5: Acessorio, 6: Consumivel
  uint8_t rank = 0;     // 0: Comum, 1: Magico, 2: Raro, 3: Epico, 4: Lendario
  uint8_t level = 1;
  uint8_t jobClass = 0; // 0: Todos, 1: Guerreiro, 2: Atirador, 3: Ladino
  uint16_t attr[6] = {0, 0, 0, 0, 0, 0};
  std::string name = "Item do Bau";

  bool empty() const { return pool == 0 && count == 0; }
};

class SharedStash {
public:
  static constexpr size_t MAX_CAPACITY = 48; // 2 páginas de 24 itens
  std::vector<StashItem> items;

  SharedStash() {
    items.reserve(MAX_CAPACITY);
  }

  bool isFull() const {
    return items.size() >= MAX_CAPACITY;
  }

  size_t count() const {
    return items.size();
  }

  bool deposit(const StashItem& item) {
    if (isFull() || item.empty()) return false;
    items.push_back(item);
    return true;
  }

  bool withdraw(size_t index, StashItem& out) {
    if (index >= items.size()) return false;
    out = items[index];
    items.erase(items.begin() + index);
    return true;
  }

  std::string serialize() const {
    std::ostringstream ss;
    ss << "shared_stash_version 1\n";
    ss << "capacity " << MAX_CAPACITY << "\n";
    ss << "count " << items.size() << "\n";
    for (size_t i = 0; i < items.size(); ++i) {
      const auto& it = items[i];
      ss << "item " << it.pool << " " << it.count << " " << it.durability << " "
         << unsigned(it.itemType) << " " << unsigned(it.rank) << " "
         << unsigned(it.level) << " " << unsigned(it.jobClass);
      for (int a = 0; a < 6; ++a) {
        ss << " " << it.attr[a];
      }
      ss << " " << (it.name.empty() ? "Item" : it.name) << "\n";
    }
    return ss.str();
  }

  static SharedStash deserialize(const std::string& text) {
    SharedStash stash;
    std::istringstream ss(text);
    std::string line;
    bool valid = false;

    while (std::getline(ss, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream ls(line);
      std::string tag;
      ls >> tag;

      if (tag == "shared_stash_version") {
        int v = 0;
        ls >> v;
        if (v == 1) valid = true;
      } else if (valid && tag == "item") {
        StashItem it;
        unsigned type = 0, rank = 0, lv = 1, job = 0;
        ls >> it.pool >> it.count >> it.durability >> type >> rank >> lv >> job;
        it.itemType = uint8_t(type);
        it.rank = uint8_t(rank);
        it.level = uint8_t(lv);
        it.jobClass = uint8_t(job);
        for (int a = 0; a < 6; ++a) {
          ls >> it.attr[a];
        }
        std::string name;
        if (ls >> name) {
          it.name = name;
        }
        if (stash.items.size() < MAX_CAPACITY) {
          stash.items.push_back(it);
        }
      }
    }
    return stash;
  }
};

inline std::filesystem::path stashFilePath(const std::filesystem::path& dir) {
  return dir / "shared-stash-v1.txt";
}

inline SharedStash loadSharedStash(const std::filesystem::path& dir) {
  std::filesystem::path p = stashFilePath(dir);
  if (!std::filesystem::exists(p)) return SharedStash();
  std::ifstream f(p, std::ios::binary);
  if (!f.is_open()) return SharedStash();
  std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return SharedStash::deserialize(content);
}

inline bool saveSharedStash(const std::filesystem::path& dir, const SharedStash& stash) {
  try {
    std::filesystem::create_directories(dir);
    std::filesystem::path target = stashFilePath(dir);
    std::filesystem::path tmp = target;
    tmp += ".tmp";

    {
      std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
      if (!f.is_open()) return false;
      std::string data = stash.serialize();
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
