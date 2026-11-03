#pragma once
#include "codex.h"
#include "combat_power.h"
#include "safe_save.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <set>
#include <optional>

namespace destinia {

enum class TitleId : uint32_t {
  Nenhum = 0,
  Iniciante = 1,
  Cacador = 2,
  Veterano = 3,
  MestreDoCombate = 4,
  MatadorDeBosses = 5,
  Complecionista = 6,
  SenhorDoAbismo = 7,
  MestreDaSobrevivencia = 8,
  CacadorDeFungos = 9
};

struct TitleDef {
  TitleId id;
  const char* nome;
  const char* descricao;
  int atkBonusPct = 0;
  int defBonusPct = 0;
  int hpBonusPct = 0;
  int mpBonusPct = 0;
  int resistancePct = 0;
  int evasionPct = 0;
  int critChanceTenths = 0;
};

inline const std::vector<TitleDef>& allTitles() {
  static const std::vector<TitleDef> list = {
    { TitleId::Nenhum, "Nenhum", "Nenhum título equipado.", 0, 0, 0, 0 },
    { TitleId::Iniciante, "Iniciante", "Primeiros passos no mundo de Destinia.", 2, 2, 5, 5 },
    { TitleId::Cacador, "Caçador", "Derrotou mais de 50 inimigos comuns.", 5, 3, 5, 5 },
    { TitleId::Veterano, "Veterano", "Atingiu nível de prestígio em combate.", 10, 8, 10, 10 },
    { TitleId::MestreDoCombate, "Mestre do Combate", "Domínio avançado das artes bélicas.", 15, 15, 15, 15 },
    { TitleId::MatadorDeBosses, "Matador de Chefes", "Eliminou os grandes líderes do continente.", 25, 20, 20, 20 },
    { TitleId::Complecionista, "Complecionista", "100% do Códex concluído. Força absoluta de Destinia.", 100, 100, 100, 100 },
    { TitleId::MestreDaSobrevivencia, "Mestre da Sobrevivencia", "Conclua a onda 20. +120% atributos, +20% resistência, +15% esquiva.", 120, 120, 120, 120, 20, 15 },
    { TitleId::SenhorDoAbismo, "Senhor do Abismo", "Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.", 150, 150, 150, 150 },
    { TitleId::CacadorDeFungos, "Cacador de Fungos", "Derrote 10 Fungos. +10% chance de critico.", 0, 0, 0, 0, 0, 0, 100 }
  };
  return list;
}

inline const TitleDef* findTitle(TitleId id) {
  for (const auto& t : allTitles()) {
    if (t.id == id) return &t;
  }
  return nullptr;
}

// Verifica se o jogador atingiu 100% no Códex (73 monstros conhecidos)
inline bool isCodexComplete(const Codex& codex, int slot) {
  if (slot < 0) return false;
  auto it = codex.entries.find(slot);
  if (it == codex.entries.end()) return false;
  const auto& records = it->second;
  // O jogo possui 73 monstros catalogados (63 comuns + 10 chefes)
  if (codex.names.empty()) return false;
  size_t totalExpected = codex.names.size();
  size_t discovered = 0;
  for (const auto& pair : codex.names) {
    if (records.count(pair.first)) ++discovered;
  }
  return (discovered >= totalExpected && totalExpected > 0);
}

inline bool titleUnlockedByCodex(TitleId id,const Codex& codex,int slot) {
  if(id==TitleId::CacadorDeFungos){
    if(slot<0||slot>=10)return false;
    const auto name=codex.names.find(1);if(name==codex.names.end()||name->second!="Fungus")return false;
    const auto character=codex.entries.find(slot);if(character==codex.entries.end())return false;
    const auto fungus=character->second.find(1);return fungus!=character->second.end()&&fungus->second.kills>=10;
  }
  if(id==TitleId::Complecionista)return isCodexComplete(codex,slot);
  if(id!=TitleId::Cacador||slot<0||slot>=10)return false;
  auto character=codex.entries.find(slot);if(character==codex.entries.end())return false;
  uint64_t commonKills=0;
  for(const auto& entry:character->second){
    if(entry.first>=0&&entry.first<100&&codex.names.count(entry.first))commonKills+=entry.second.kills;
    if(commonKills>50)return true;
  }
  return false;
}

// Aplica o bônus de status do título ativo
inline CombatStats applyTitleBuffs(const CombatStats& baseStats, TitleId title) {
  CombatStats modified = baseStats;
  const TitleDef* def = findTitle(title);
  if (!def) return modified;

  if (def->hpBonusPct > 0) {
    modified[5] += (modified[5] * def->hpBonusPct) / 100; // PV / MaxHP
  }
  if (def->mpBonusPct > 0) {
    modified[6] += (modified[6] * def->mpBonusPct) / 100; // PM / MaxMP
  }
  if (def->atkBonusPct > 0) {
    modified[7] += (modified[7] * def->atkBonusPct) / 100; // ATK
  }
  if (def->defBonusPct > 0) {
    modified[8] += (modified[8] * def->defBonusPct) / 100; // DEF
  }
  return modified;
}

// Sidecar para persistência dos títulos equipados e desbloqueados por personagem
class TitlesStore {
  bool writable=true;
  mutable bool baselineKnown=false;
  mutable fs::path baselinePath;
  mutable std::optional<std::string> baseline;
  static std::optional<std::string> disk(const fs::path& p) {
    if(isLink(p))throw std::runtime_error("Arquivo de titulos vinculado");
    if(!fs::exists(p))return std::nullopt;
    if(!fs::is_regular_file(p)||fs::file_size(p)>120000)throw std::runtime_error("Arquivo de titulos invalido");
    std::ifstream in(p,std::ios::binary);if(!in)throw std::runtime_error("Falha lendo titulos");
    std::string data((std::istreambuf_iterator<char>(in)),{});
    if(in.bad())throw std::runtime_error("Leitura de titulos incompleta");
    return data;
  }
public:
  std::map<int, TitleId> equippedTitles;
  std::map<int, std::vector<TitleId>> unlockedTitles;
  std::set<int> exclusiveTitles;
  bool canSave() const { return writable; }
  bool unchanged(const fs::path& dir) const {
    try {const auto p=dir/"titles-v1.txt";return writable&&baselineKnown&&baselinePath==fs::absolute(p).lexically_normal()&&disk(p)==baseline;}
    catch(...) {return false;}
  }
  void eraseSlot(int slot) {equippedTitles.erase(slot);unlockedTitles.erase(slot);exclusiveTitles.erase(slot);}
  bool isExclusive(int slot) const { return exclusiveTitles.count(slot)!=0; }
  void setExclusive(int slot,bool exclusive) { if(exclusive)exclusiveTitles.insert(slot);else exclusiveTitles.erase(slot); }

  TitleId getEquipped(int slot) const {
    auto it = equippedTitles.find(slot);
    return it != equippedTitles.end() ? it->second : TitleId::Nenhum;
  }

  void setEquipped(int slot, TitleId id) {
    equippedTitles[slot] = id;
  }

  bool isUnlocked(int slot, TitleId id) const {
    if (id == TitleId::Nenhum || id == TitleId::Iniciante) return true;
    auto it = unlockedTitles.find(slot);
    if (it == unlockedTitles.end()) return false;
    for (auto t : it->second) {
      if (t == id) return true;
    }
    return false;
  }

  void unlock(int slot, TitleId id) {
    if (isUnlocked(slot, id)) return;
    unlockedTitles[slot].push_back(id);
  }

  std::string encode() const {
    std::ostringstream out;
    out << "DESTINIA_TITLES_V1\n";
    for (const auto& pair : equippedTitles) {
      out << "equipped " << pair.first << " " << static_cast<uint32_t>(pair.second) << "\n";
    }
    for (const auto& pair : unlockedTitles) {
      for (auto id : pair.second) {
        out << "unlocked " << pair.first << " " << static_cast<uint32_t>(id) << "\n";
      }
    }
    for (int slot : exclusiveTitles) out << "exclusive " << slot << " 1\n";
    out << "end\n";
    return out.str();
  }

  bool decode(const std::string& text) {
    if (text.size() > 120000) return false;
    std::istringstream in(text);
    std::string token;
    if (!(in >> token) || token != "DESTINIA_TITLES_V1") return false;
    std::map<int,TitleId> equipped;
    std::map<int,std::vector<TitleId>> unlocked;
    std::set<std::pair<int,uint32_t>> seenUnlocked;
    std::set<int> exclusive;
    bool ended=false;
    while (in >> token) {
      if (token == "end") { ended=true; break; }
      int slot=-1; uint32_t id=0;
      if (!(in >> slot >> id) || slot<0 || slot>=10) return false;
      if(token=="exclusive") {if(id!=1||!exclusive.insert(slot).second)return false;continue;}
      if(!findTitle(static_cast<TitleId>(id))) return false;
      if (token == "equipped") {
        if (!equipped.emplace(slot,static_cast<TitleId>(id)).second) return false;
      } else if (token == "unlocked") {
        if (!seenUnlocked.emplace(slot,id).second) return false;
        unlocked[slot].push_back(static_cast<TitleId>(id));
      } else return false;
    }
    if (!ended || (in >> token)) return false;
    for(int slot:exclusive) {
      auto it=equipped.find(slot);
      if(it==equipped.end()||it->second==TitleId::Nenhum)return false;
      if(it->second!=TitleId::Iniciante&&!seenUnlocked.count({slot,static_cast<uint32_t>(it->second)}))return false;
    }
    equippedTitles.swap(equipped);
    unlockedTitles.swap(unlocked);
    exclusiveTitles.swap(exclusive);
    return true;
  }

  bool load(const fs::path& dir) {
    try {
      const fs::path p=dir/"titles-v1.txt";
      if(isLink(dir)||isLink(p)){writable=false;return false;}
      auto content=disk(p);
      baselineKnown=true;baselinePath=fs::absolute(p).lexically_normal();baseline=content;
      if(!content){writable=true;return false;}
      if(!decode(*content)){writable=false;return false;}
      writable=true;return true;
    } catch(...) {writable=false;return false;}
  }

  bool save(const fs::path& dir) const {
    if(!writable)return false;
    fs::path temp;bool owned=false;
    try {
      const auto target=dir/"titles-v1.txt";
      if(isLink(dir)||isLink(target)||(fs::exists(target)&&!fs::is_regular_file(target)))return false;
      // Coordinate with character deletion and other cooperating title writers.
      // The lock is held through the final atomic replacement.
      FileLock titleLock;if(!titleLock.acquire(dir/"titles-v1.lock"))return false;
      const auto absolute=fs::absolute(target).lexically_normal();
      const auto before=disk(target);
      if((baselineKnown&&(baselinePath!=absolute||baseline!=before))||(!baselineKnown&&before))return false;
      const auto data=encode();TitlesStore check;if(!check.decode(data))return false;
      temp=target;temp+=".";temp+=uniqueId();temp+=".tmp";
#ifdef _WIN32
      HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
      if(file==INVALID_HANDLE_VALUE)return false;
      owned=true;DWORD written=0;
      const bool complete=data.size()<=MAXDWORD&&WriteFile(file,data.data(),DWORD(data.size()),&written,nullptr)&&written==data.size()&&FlushFileBuffers(file);
      const bool closed=CloseHandle(file)!=0;
      if(!complete||!closed)throw std::runtime_error("Falha gravando titulos temporarios");
#else
      const int file=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
      if(file<0)return false;
      owned=true;size_t offset=0;bool complete=true;
      while(offset<data.size()){const auto n=::write(file,data.data()+offset,data.size()-offset);if(n<=0){complete=false;break;}offset+=size_t(n);}
      if(::fsync(file)!=0)complete=false;
      if(::close(file)!=0)complete=false;
      if(!complete)throw std::runtime_error("Falha gravando titulos temporarios");
#endif
      if(disk(target)!=before)throw std::runtime_error("Titulos alterados externamente");
      AtomicSaveOps{}.replace(temp,target);owned=false;
      baselineKnown=true;baselinePath=absolute;baseline=data;return true;
    }catch(...){if(owned){std::error_code ec;fs::remove(temp,ec);}return false;}
  }
};

// Do not expose an unlock in memory until it has been persisted successfully.
inline bool persistTitleUnlock(TitlesStore& store,int slot,TitleId id,const fs::path& dir) {
  if(slot<0||slot>=10||!findTitle(id))return false;
  if(store.isUnlocked(slot,id))return true;
  auto next=store;next.unlock(slot,id);
  if(!next.save(dir))return false;
  store=std::move(next);return true;
}

} // namespace destinia
