#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <ctime>
#include <map>
#include <set>
#include <stdexcept>
#include <cstdio>
#include "safe_save.h"

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
    c1.targetId = 61; // Imperial Soldier (species 61 no mon_data_en.gdt)
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
      bool matches = (!c.completed) && (
        (c.targetId == species) ||
        (c.id == 1 && (species == 61 || species == 62 || species == 63 || species == 2))
      );
      if (matches) {
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
          if (c.targetId == 2) c.targetId = 61;
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
  namespace fs=std::filesystem;
  const fs::path p=bountiesFilePath(dir);
  if (isLink(dir) || isLink(p))
    throw std::runtime_error("Bounty: link inseguro; arquivo preservado");
  if (!fs::exists(p)) return BountyManager();
  if (!fs::is_regular_file(p) || fs::file_size(p)>1048576)
    throw std::runtime_error("Bounty: arquivo invalido; preservado");
  std::ifstream f(p,std::ios::binary);
  if (!f) throw std::runtime_error("Bounty: falha de leitura");
  const std::string content{std::istreambuf_iterator<char>(f),{}};
  if (f.bad()) throw std::runtime_error("Bounty: leitura incompleta");
  std::istringstream ss(content);
  std::string line,extra;
  if (!std::getline(ss,line) || line!="bounties_version 1")
    throw std::runtime_error("Bounty: versao/estrutura invalida; preservada");
  std::set<std::pair<unsigned,unsigned>> seen;
  while (std::getline(ss,line)) {
    if (line.empty() || line[0]=='#') continue;
    std::istringstream row(line);
    std::string name;
    unsigned slot=0,id=0,type=0,target=0,goal=0,progress=0;
    uint64_t gold=0,cash=0,completed=0,claimed=0;
    if (!(row>>name>>slot>>id>>type>>target>>goal>>progress
              >>gold>>cash>>completed>>claimed) || row>>extra ||
        name!="bounty" || slot>=10 || id<1 || id>3 || type>2 ||
        target>65535 || goal==0 || goal>65535 || progress>goal ||
        gold>UINT32_MAX || cash>65535 || completed>1 || claimed>1 ||
        (claimed && !completed) || (completed && progress!=goal) ||
        !seen.insert({slot,id}).second)
      throw std::runtime_error("Bounty: registro invalido; preservado");
  }
  if (!ss.eof()) throw std::runtime_error("Bounty: arquivo truncado");
  return BountyManager::deserialize(content);
}

inline bool saveBounties(const std::filesystem::path& dir,
                        const BountyManager& mgr,
                        const AtomicSaveOps& ops = AtomicSaveOps{}) {
  namespace fs=std::filesystem;
  fs::path tmp;
  try {
    if(isLink(dir)||(fs::exists(dir)&&!fs::is_directory(dir)))return false;
    fs::create_directories(dir);
    const auto target=bountiesFilePath(dir);
    if(isLink(target)||(fs::exists(target)&&
       (!fs::is_regular_file(target)||fs::file_size(target)>1048576)))return false;
    const std::string data=mgr.serialize();
    if(data.size()>1048576)return false;
    tmp=fs::path(target.string()+"."+uniqueId()+".tmp");
    FILE* f=std::fopen(tmp.string().c_str(),"wb");
    if(!f)return false;
    bool okay=std::fwrite(data.data(),1,data.size(),f)==data.size()&&
              std::fflush(f)==0;
#ifdef _WIN32
    if(okay)okay=::_commit(::_fileno(f))==0;
#else
    if(okay)okay=::fsync(::fileno(f))==0;
#endif
    if(std::fclose(f)!=0)okay=false;
    if(!okay)throw std::runtime_error("Falha gravando bounty");
    if(isLink(target)||(fs::exists(target)&&!fs::is_regular_file(target)))
      throw std::runtime_error("Destino bounty invalido");
    ops.replace(tmp,target);
    return true;
  }catch(...){
    if(!tmp.empty()) {std::error_code ec;fs::remove(tmp,ec);}
    return false;
  }
}
} // namespace destinia
