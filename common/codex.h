#pragma once
#include "runtime_support.h"
#include <map>
#include <set>
#include <limits>
#include <sstream>

namespace destinia {
struct CodexEntry { uint32_t kills=0; };
class Codex {
  std::set<uint64_t> counted;
public:
  int slot=-1; bool dirty=false;
  std::map<int,std::string> names;
  std::map<int,std::map<int,CodexEntry>> entries;
  void select(int value) { if(value<0 || value>9) return; if(slot!=value) {slot=value;counted.clear();} }
  void respawn(uint64_t instance) { counted.erase(instance); }
  void newMap() { counted.clear(); }
  void erase(int value) { entries.erase(value);counted.clear();dirty=true; }
  void observe(uint64_t instance,int species,bool killed) {
    if(slot<0 || !names.count(species)) return;
    auto& records=entries[slot];auto found=records.find(species);
    if(found==records.end()) {found=records.emplace(species,CodexEntry{}).first;dirty=true;}
    if(killed && counted.insert(instance).second) {
      if(found->second.kills<std::numeric_limits<uint32_t>::max()) ++found->second.kills;
      dirty=true;
    }
  }
  void load(const fs::path& file) {
    if(!fs::exists(file)) return;
    if(isLink(file) || fs::file_size(file)>65536) throw std::runtime_error("Arquivo de Codex invalido; original preservado.");
    std::ifstream in(file);std::string line;
    if(!std::getline(in,line) || line!="DESTINIA_CODEX_1") throw std::runtime_error("Formato de Codex desconhecido.");
    decltype(entries) next;
    while(std::getline(in,line)) {
      if(line.empty()) continue;
      std::istringstream row(line);int character,species;long long kills;std::string extra;
      if(!(row>>character>>species>>kills) || row>>extra || character<0 || character>9 || !names.count(species) || kills<0 || kills>UINT32_MAX || next[character].count(species))
        throw std::runtime_error("Registro de Codex invalido; original preservado.");
      next[character][species].kills=uint32_t(kills);
    }
    if(!in.eof()) throw std::runtime_error("Erro ao ler Codex.");
    entries=std::move(next);dirty=false;
  }
  void save(const fs::path& file) {
    if(!file.parent_path().empty()) fs::create_directories(file.parent_path());
    if(fs::exists(file) && isLink(file)) throw std::runtime_error("Codex recusa link.");
    const fs::path temp=file.string()+"."+uniqueId()+".tmp";
    try {
      std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<"DESTINIA_CODEX_1\n";
      for(const auto& character:entries) for(const auto& entry:character.second) out<<character.first<<' '<<entry.first<<' '<<entry.second.kills<<'\n';
      out.flush();out.close();if(!out) throw std::runtime_error("Falha ao salvar Codex.");
#ifdef _WIN32
      if(!MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("Falha ao substituir Codex.");
#else
      fs::rename(temp,file);
#endif
      dirty=false;
    }catch(...) {std::error_code ec;fs::remove(temp,ec);throw;}
  }
  void readNames(const std::vector<uint8_t>& data) {
    if(data.size()!=3840) throw std::runtime_error("Tabela de monstros incompatível com o Codex 0.7.");
    names.clear();
    for(size_t i=0;i<data.size()/32;++i) {
      std::string text;for(size_t j=12;j<32 && data[i*32+j];++j) {auto c=data[i*32+j];if(c<32 || c>126) {text.clear();break;}text+=char(c);}
      if(!text.empty()) names[int(i)]=text;
    }
  }
};
} // namespace destinia
