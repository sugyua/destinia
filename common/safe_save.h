#pragma once
#include "runtime_support.h"
#include <map>
#include <set>
#include <sstream>
#ifdef _WIN32
#include <io.h>
#endif

namespace destinia {
// OS-backed replacement is injectable to exercise disk errors and abrupt exits
// in tests without a failure switch in the distributed host.
struct AtomicSaveOps {
  virtual ~AtomicSaveOps()=default;
  virtual void replace(const fs::path& source,const fs::path& target) const {
#ifdef _WIN32
    if(!MoveFileExW(source.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
      throw std::runtime_error("Falha ao substituir save com seguranca.");
#else
    if(::rename(source.c_str(),target.c_str())!=0) throw std::runtime_error("Falha ao substituir save com seguranca.");
#endif
  }
};
class SaveTransaction {
  struct Entry {std::string name;bool existed=false,deleted=false;};
  struct Handle {size_t entry;bool writable;};
  fs::path root,journal;
  std::vector<Entry> entries;
  std::vector<fs::path> removals;
  std::map<FILE*,Handle> handles;
  std::set<FILE*> abortedHandles;
  const AtomicSaveOps& ops;
  bool active=false,failed=false,owned=false,committing=false;
  static const AtomicSaveOps& defaults(){static AtomicSaveOps value;return value;}
  static bool validName(const std::string& value){
    return !value.empty() && value.size()<=240 && value[0]!='.' && value.find_first_of("/\\\r\n\t:")==std::string::npos && fs::path(value).filename()==fs::path(value);
  }
  static std::string readText(const fs::path& path,size_t limit=65536){
    if(isLink(path)||!fs::is_regular_file(path)||fs::file_size(path)>limit)throw std::runtime_error("Journal de save invalido; preservado para revisao.");
    std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Falha lendo journal de save.");
    return {std::istreambuf_iterator<char>(in),{}};
  }
  static bool flushFile(FILE* file){
    if(std::fflush(file)!=0)return false;
#ifdef _WIN32
    return ::_commit(::_fileno(file))==0;
#else
    return ::fsync(::fileno(file))==0;
#endif
  }
  static void textFile(const fs::path& file,const std::string& value){
    FILE* f=std::fopen(file.string().c_str(),"wb");if(!f)throw std::runtime_error("Falha criando journal de save.");
    bool okay=std::fwrite(value.data(),1,value.size(),f)==value.size() && flushFile(f);
    if(std::fclose(f)!=0)okay=false;
    if(!okay)throw std::runtime_error("Falha gravando journal de save.");
  }
  static void copyFile(const fs::path& source,const fs::path& target){
    if(isLink(source)||!fs::is_regular_file(source))throw std::runtime_error("Save fonte invalido.");
    fs::copy_file(source,target,fs::copy_options::overwrite_existing);
    FILE*f=std::fopen(target.string().c_str(),"r+b");if(!f)throw std::runtime_error("Falha validando copia do save.");
    bool okay=flushFile(f);if(std::fclose(f)!=0)okay=false;
    if(!okay||!sameFileContents(source,target))throw std::runtime_error("Falha sincronizando copia do save.");
  }
  fs::path candidate(size_t n)const{return journal/"new"/std::to_string(n);}
  void ensureOwned(){
    if(owned)return;
    if(fs::exists(journal))throw std::runtime_error("Journal pendente; recuperar antes de salvar.");
    if(!fs::create_directory(journal))throw std::runtime_error("Falha criando pasta transacional.");
    textFile(journal/"owner","DESTINIA_SAVE_TRANSACTION_1\n");owned=true;
    fs::create_directory(journal/"new");fs::create_directory(journal/"old");
  }
  void validateOwned()const{
    if(isLink(journal)||!fs::is_directory(journal)||readText(journal/"owner",64)!="DESTINIA_SAVE_TRANSACTION_1\n")throw std::runtime_error("Pasta transacional desconhecida; nenhum arquivo alterado.");
    for(const auto& file:fs::recursive_directory_iterator(journal)){
      if(isLink(file.path()))throw std::runtime_error("Link em journal; recuperacao recusada.");
      const auto relative=file.path().lexically_relative(journal);const std::string name=relative.generic_string();
      if(relative.parent_path().empty()){
        if(name=="new"||name=="old"){if(!file.is_directory())throw std::runtime_error("Journal invalido.");}
        else if(name!="owner"&&name!="manifest"&&name!="manifest.tmp"&&name!="committed")throw std::runtime_error("Arquivo desconhecido em journal; preservado.");
      }else{
        const auto parent=relative.parent_path().generic_string(),number=relative.filename().string();
        if((parent!="new"&&parent!="old")||number.empty()||number.size()>3||number.find_first_not_of("0123456789")!=std::string::npos||!file.is_regular_file())throw std::runtime_error("Conteudo inesperado no journal.");
      }
    }
  }
  void cleanup(){if(fs::exists(journal)){validateOwned();fs::remove_all(journal);}owned=false;}
  void abortHandles(){for(auto it=handles.begin();it!=handles.end();){if(it->second.writable){abortedHandles.insert(it->first);std::fclose(it->first);it=handles.erase(it);}else ++it;}}
public:
  explicit SaveTransaction(const AtomicSaveOps& operations=defaults()):ops(operations){}
  SaveTransaction(const SaveTransaction&)=delete;SaveTransaction& operator=(const SaveTransaction&)=delete;
  ~SaveTransaction(){for(auto&h:handles)std::fclose(h.first);if(owned&&!committing){try{cleanup();}catch(...){}}}
  void configure(const fs::path& directory){
    if(active||!handles.empty())throw std::runtime_error("Transacao ainda aberta.");
    root=fs::absolute(directory).lexically_normal();if(root==root.root_path()||isLink(root))throw std::runtime_error("Pasta de save invalida.");
    journal=root.parent_path()/(root.filename().string()+".transaction");
    fs::create_directories(root);
  }
  void recover(){
    if(journal.empty()||!fs::exists(journal))return;
    validateOwned();owned=true;committing=true;
    if(fs::exists(journal/"committed")){
      if(readText(journal/"committed",8)!="OK\n")throw std::runtime_error("Marca transacional invalida.");
      cleanup();committing=false;return;
    }
    if(!fs::exists(journal/"manifest")){cleanup();committing=false;return;} // Never changed active files.
    std::istringstream in(readText(journal/"manifest"));std::string line;
    if(!std::getline(in,line)||line!="DESTINIA_BATCH_1")throw std::runtime_error("Journal incompleto; preservado.");
    std::vector<Entry> restore;std::set<std::string> names;bool ended=false;
    while(std::getline(in,line)){
      if(line=="END"){ended=true;break;}
      const auto tab=line.find('\t');
      if(tab==std::string::npos||!validName(line.substr(0,tab))||(line.substr(tab+1)!="0"&&line.substr(tab+1)!="1")||restore.size()>=128||!names.insert(line.substr(0,tab)).second)throw std::runtime_error("Registro transacional invalido.");
      restore.push_back({line.substr(0,tab),line.substr(tab+1)=="1"});
    }
    if(!ended||restore.empty()||std::getline(in,line))throw std::runtime_error("Journal truncado; recuperacao recusada.");
    // Validate the entire recovery set before changing any active file.
    for(size_t i=0;i<restore.size();++i){
      const auto target=root/restore[i].name;
      if(isLink(target)||(fs::exists(target)&&!fs::is_regular_file(target)))throw std::runtime_error("Destino de recuperacao invalido.");
      if(restore[i].existed && (isLink(journal/"old"/std::to_string(i))||!fs::is_regular_file(journal/"old"/std::to_string(i))))throw std::runtime_error("Copia anterior ausente; journal preservado.");
    }
    committing=true; // Keep recovery data even if this attempt is interrupted.
    for(size_t i=0;i<restore.size();++i){
      if(restore[i].existed){copyFile(journal/"old"/std::to_string(i),candidate(i));ops.replace(candidate(i),root/restore[i].name);}
      else fs::remove(root/restore[i].name);
    }
    cleanup();committing=false;
  }
  void begin(){if(active)throw std::runtime_error("Transacao ARM aninhada.");active=true;failed=false;entries.clear();removals.clear();}
  FILE* open(const fs::path& target,const std::string& mode){
    const auto path=fs::absolute(target).lexically_normal();
    if(root.empty()||path.parent_path()!=root||!validName(path.filename().string())||isLink(path)||(fs::exists(path)&&!fs::is_regular_file(path)))throw std::runtime_error("Caminho de save invalido.");
    const bool writing=mode.find_first_of("wa+")!=std::string::npos;
    if(writing&&!active)throw std::runtime_error("Escrita fora de uma transacao ARM.");
    size_t index=entries.size();for(size_t i=0;i<entries.size();++i)if(entries[i].name==path.filename().string()){index=i;break;}
    if(!writing && index<entries.size() && entries[index].deleted)return nullptr;
    if(!writing){FILE*f=std::fopen((index<entries.size()?candidate(index):path).string().c_str(),mode.c_str());if(f){abortedHandles.erase(f);handles[f]={index,false};}return f;}
    for(const auto& h:handles)if(h.second.writable&&h.second.entry==index)throw std::runtime_error("Save aberto para duas escritas.");
    if(index<entries.size() && entries[index].deleted){
      if(mode.find('r')!=std::string::npos)return nullptr;
      fs::remove(candidate(index));entries[index].deleted=false;
    }
    if(index==entries.size()){
      if(mode.find('r')!=std::string::npos&&!fs::exists(path))return nullptr;
      if(entries.size()>=128)throw std::runtime_error("Arquivos demais no lote de save.");
      ensureOwned();entries.push_back({path.filename().string(),fs::exists(path)});
      if(fs::exists(path)&&mode.find('w')==std::string::npos)copyFile(path,candidate(index));
    }
    FILE*f=std::fopen(candidate(index).string().c_str(),mode.c_str());
    if(!f){failed=true;return nullptr;}abortedHandles.erase(f);handles[f]={index,true};return f;
  }
  int remove(const fs::path& target){
    const auto path=fs::absolute(target).lexically_normal();
    if(root.empty()||!active||path.parent_path()!=root||!validName(path.filename().string())||isLink(path)||(fs::exists(path)&&!fs::is_regular_file(path)))throw std::runtime_error("Exclusao fora do save/transacao.");
    size_t index=entries.size();for(size_t i=0;i<entries.size();++i)if(entries[i].name==path.filename().string()){index=i;break;}
    if(index<entries.size() && entries[index].deleted)return -1;
    if(index==entries.size() && !fs::exists(path))return -1;
    for(const auto& handle:handles)if(handle.second.writable&&handle.second.entry==index)return -1;
    if(index==entries.size()){
      if(entries.size()>=128)throw std::runtime_error("Arquivos demais no lote de save.");
      ensureOwned();entries.push_back({path.filename().string(),true,true});
    }else entries[index].deleted=true;
    return 0;
  }
  const std::vector<fs::path>& committedRemovals()const{return removals;}
  int close(FILE* file){
    if(abortedHandles.erase(file))return EOF; // Host handle table still references an aborted writer.
    auto it=handles.find(file);if(it==handles.end())return std::fclose(file);
    bool okay=std::ferror(file)==0;
    if(it->second.writable&&!flushFile(file))okay=false;
    if(std::fclose(file)!=0)okay=false;handles.erase(it);
    if(!okay)failed=true;return okay?0:EOF;
  }
  void finish(bool success){
    if(!active)return;
    active=false;
    const bool unclosed=std::any_of(handles.begin(),handles.end(),[](const auto&h){return h.second.writable;});
    if(!success||failed||unclosed){abortHandles();if(owned)cleanup();entries.clear();if(success)throw std::runtime_error("Escrita incompleta; save anterior preservado.");return;}
    if(entries.empty())return;
    try{
      std::ostringstream manifest;manifest<<"DESTINIA_BATCH_1\n";
      for(size_t i=0;i<entries.size();++i){
        const auto path=root/entries[i].name;
        if(isLink(path)||(fs::exists(path)&&!fs::is_regular_file(path))||(!entries[i].deleted&&!fs::is_regular_file(candidate(i))))throw std::runtime_error("Save ou temporario invalido.");
        entries[i].existed=fs::exists(path);
        if(entries[i].existed)copyFile(path,journal/"old"/std::to_string(i));
        manifest<<entries[i].name<<'\t'<<(entries[i].existed?'1':'0')<<'\n';
      }
      manifest<<"END\n";textFile(journal/"manifest.tmp",manifest.str());fs::rename(journal/"manifest.tmp",journal/"manifest");
      committing=true;
      for(size_t i=0;i<entries.size();++i){
        if(entries[i].deleted)fs::remove(root/entries[i].name);
        else ops.replace(candidate(i),root/entries[i].name);
      }
      textFile(journal/"committed","OK\n");cleanup();committing=false;
      for(const auto& entry:entries)if(entry.deleted)removals.push_back(root/entry.name);
      entries.clear();
    }catch(...){if(committing)recover();else cleanup();entries.clear();throw;}
  }
};
class SaveCall {
  SaveTransaction& transaction;bool done=false;
public:
  explicit SaveCall(SaveTransaction& value):transaction(value){transaction.begin();}
  ~SaveCall(){if(!done){try{transaction.finish(false);}catch(...){}}}
  void commit(){transaction.finish(true);done=true;}
};
}
