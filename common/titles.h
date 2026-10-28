#pragma once
#include "codex.h"
#include "combat_power.h"
#include "safe_save.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

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
  MestreDaSobrevivencia = 8
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
    { TitleId::SenhorDoAbismo, "Senhor do Abismo", "Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.", 150, 150, 150, 150 }
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
public:
  std::map<int, TitleId> equippedTitles;
  std::map<int, std::vector<TitleId>> unlockedTitles;

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
    out << "end\n";
    return out.str();
  }

  bool decode(const std::string& text) {
    std::istringstream in(text);
    std::string token;
    if (!(in >> token) || token != "DESTINIA_TITLES_V1") return false;
    equippedTitles.clear();
    unlockedTitles.clear();
    while (in >> token) {
      if (token == "end") break;
      if (token == "equipped") {
        int slot; uint32_t id;
        if (in >> slot >> id) equippedTitles[slot] = static_cast<TitleId>(id);
      } else if (token == "unlocked") {
        int slot; uint32_t id;
        if (in >> slot >> id) unlockedTitles[slot].push_back(static_cast<TitleId>(id));
      }
    }
    return true;
  }

  bool load(const fs::path& dir) {
    fs::path p = dir / "titles-v1.txt";
    if (!fs::exists(p)) return false;
    std::ifstream in(p);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return decode(content);
  }

  bool save(const fs::path& dir) const {
    fs::path p = dir / "titles-v1.txt";
    fs::path tmp = dir / "titles-v1.txt.tmp";
    if(isLink(dir)||isLink(p)||isLink(tmp))return false;
    std::ofstream out(tmp,std::ios::binary);
    if (!out.is_open()) return false;
    out << encode();
    out.close();
    std::error_code ec;
    if(!out)return false;
    try{AtomicSaveOps{}.replace(tmp,p);return true;}catch(...){return false;}
  }
};

} // namespace destinia
