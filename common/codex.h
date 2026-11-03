#pragma once
#include "runtime_support.h"
#include <map>
#include <set>
#include <limits>
#include <optional>
#include <sstream>
#ifdef _WIN32
#include <io.h>
#endif

namespace destinia {
struct CodexBusyError : std::runtime_error { CodexBusyError():std::runtime_error("Codex ocupado."){} };
struct CodexSaveOps {
  virtual ~CodexSaveOps()=default;
  virtual fs::path temporary(const fs::path& file) const {auto temp=file;temp+="."+uniqueId()+".tmp";return temp;}
};
struct CodexEntry { uint32_t kills=0; };
class Codex {
  std::set<uint64_t> counted;
  bool baselineKnown=false;
  fs::path baselinePath;
  std::optional<std::string> baseline;
  static std::optional<std::string> diskSnapshot(const fs::path& file){
    if(isLink(file))throw std::runtime_error("Codex recusa link.");
    if(!fs::exists(file))return std::nullopt;
    if(!fs::is_regular_file(file)||fs::file_size(file)>65536)throw std::runtime_error("Arquivo de Codex invalido; original preservado.");
    std::ifstream in(file,std::ios::binary);if(!in)throw std::runtime_error("Falha lendo Codex.");
    std::string data((std::istreambuf_iterator<char>(in)),{});if(in.bad()||data.size()>65536)throw std::runtime_error("Leitura de Codex incompleta.");return data;
  }
  static void writeExclusive(const fs::path& path,const std::string& data){
    bool created=false;
    try {
#ifdef _WIN32
      FILE* out=::_wfopen(path.c_str(),L"wbx");
#else
      FILE* out=std::fopen(path.c_str(),"wbx");
#endif
      if(!out)throw std::runtime_error("Falha criando arquivo exclusivo do Codex.");
      created=true;bool okay=std::fwrite(data.data(),1,data.size(),out)==data.size()&&std::fflush(out)==0;
#ifdef _WIN32
      if(okay)okay=::_commit(::_fileno(out))==0;
#else
      if(okay)okay=::fsync(::fileno(out))==0;
#endif
      if(std::fclose(out)!=0)okay=false;
      if(!okay)throw std::runtime_error("Falha gravando arquivo do Codex.");
    }catch(...){if(created){std::error_code ec;fs::remove(path,ec);}throw;}
  }
public:
  int slot=-1; bool dirty=false;
  std::map<int,std::string> names;
  std::map<int,std::map<int,CodexEntry>> entries;
  void select(int value) { if(value<0 || value>9) return; if(slot!=value) {slot=value;counted.clear();} }
  void respawn(uint64_t instance) { counted.erase(instance); }
  void newMap() { counted.clear(); }
  void erase(int value) { entries.erase(value);counted.clear();dirty=true; }
  std::string encode() const {
    std::ostringstream out;out<<"DESTINIA_CODEX_1\n";
    for(const auto& character:entries)for(const auto& entry:character.second)out<<character.first<<' '<<entry.first<<' '<<entry.second.kills<<'\n';
    return out.str();
  }
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
    const auto original=diskSnapshot(file);const auto absolute=fs::absolute(file).lexically_normal();
    if(!original){baselineKnown=true;baselinePath=absolute;baseline=std::nullopt;return;}
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
    if(diskSnapshot(file)!=original)throw std::runtime_error("Codex mudou durante a leitura.");
    entries=std::move(next);dirty=false;baselineKnown=true;baselinePath=absolute;baseline=original;
  }
  bool matchesBaseline(const fs::path& file) const {
    try{return baselineKnown&&baselinePath==fs::absolute(file).lexically_normal()&&diskSnapshot(file)==baseline;}catch(...){return false;}
  }
  void adoptPersisted(const fs::path& file){
    const auto saved=diskSnapshot(file);Codex verified;verified.names=names;verified.load(file);
    if(verified.encode()!=encode()||diskSnapshot(file)!=saved)throw std::runtime_error("Codex mudou depois da exclusao.");
    baselineKnown=true;baselinePath=fs::absolute(file).lexically_normal();baseline=saved;dirty=false;
  }
  void save(const fs::path& file,const CodexSaveOps& operations=CodexSaveOps{}) {
    if(!file.parent_path().empty()) fs::create_directories(file.parent_path());
    if(isLink(file)) throw std::runtime_error("Codex recusa link.");
    FileLock lock;if(!lock.acquire(file.parent_path()/"codex-v1.lock")){if(lock.busy())throw CodexBusyError();throw std::runtime_error("Trava do Codex indisponivel.");}
    const auto before=diskSnapshot(file);const auto absolute=fs::absolute(file).lexically_normal();
    if((baselineKnown&&(baselinePath!=absolute||baseline!=before))||(!baselineKnown&&before))throw std::runtime_error("Codex alterado externamente; progresso preservado em memoria.");
    const fs::path temp=operations.temporary(file);
    const auto absoluteTemp=fs::absolute(temp).lexically_normal(),absoluteFile=fs::absolute(file).lexically_normal();
    if(absoluteTemp.parent_path()!=absoluteFile.parent_path()||absoluteTemp==absoluteFile)throw std::runtime_error("Temporario de Codex invalido.");
    bool created=false;
    try {
      const auto data=encode();writeExclusive(temp,data);created=true;
      if(diskSnapshot(file)!=before)throw std::runtime_error("Codex mudou durante a gravacao.");
#ifdef _WIN32
      if(!MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("Falha ao substituir Codex.");
#else
      fs::rename(temp,file);
#endif
      created=false;baselineKnown=true;baselinePath=absolute;baseline=data;dirty=false;
    }catch(...) {if(created){std::error_code ec;fs::remove(temp,ec);}throw;}
  }
  fs::path saveRecovery(const fs::path& directory) const {
    if(isLink(directory)||!fs::is_directory(directory))throw std::runtime_error("Pasta de recuperacao do Codex invalida.");
    const auto recovery=directory/("codex-recovery-"+uniqueId()+".txt");
    writeExclusive(recovery,encode());return recovery;
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
