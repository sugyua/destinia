#pragma once
#include "runtime_support.h"
#include <functional>
#include <regex>
namespace destinia {
inline bool validGameMode(const std::string& mode){return mode=="classico"||mode=="expandido";}
inline fs::path modeSaveDir(const fs::path& classic,const std::string& mode){
 if(!validGameMode(mode))throw std::runtime_error("Modo de jogo invalido; use classico ou expandido.");
 return mode=="classico"?classic:classic.parent_path()/(classic.filename().string()+"_expandido");
}
inline bool modeProgressFile(const fs::path& path){const auto n=path.filename().string();return n=="msave.dat"||std::regex_match(n,std::regex("save[0-9]+\\.dat"));}
inline void verifySaveMode(const fs::path& root,const std::string& mode){
 if(!validGameMode(mode)||isLink(root)||!fs::is_directory(root))throw std::runtime_error("Pasta/modo de save invalido.");
 const auto marker=root/".destinia-mode";
 if(isLink(marker))throw std::runtime_error("Identificacao do modo e um link; pasta preservada.");
 if(fs::exists(marker)){
  if(!fs::is_regular_file(marker)||fs::file_size(marker)>64)throw std::runtime_error("Identificacao do modo invalida.");
  std::ifstream input(marker,std::ios::binary);std::string text((std::istreambuf_iterator<char>(input)),{});
  if(text!="DESTINIA_MODE_1\n"+mode+"\n")throw std::runtime_error("Save pertence a outro modo; selecione o modo correto.");
  return;
 }
 if(mode=="expandido")for(const auto& file:fs::directory_iterator(root))if(modeProgressFile(file.path()))throw std::runtime_error("Save sem identificacao: importe explicitamente do classico.");
 const auto tmp=root/(".destinia-mode-"+uniqueId()+".tmp");
 try{std::ofstream out(tmp,std::ios::binary);out<<"DESTINIA_MODE_1\n"<<mode<<"\n";out.close();if(!out)throw std::runtime_error("Falha ao identificar modo.");fs::rename(tmp,marker);}catch(...){std::error_code ec;fs::remove(tmp,ec);throw;}
}
// Only explicit user action calls this. Destination must be absent; never overwrite.
inline void importClassicSave(const fs::path& source,const fs::path& target){
 const auto from=fs::absolute(source).lexically_normal(),to=fs::absolute(target).lexically_normal();
 if(from==to||isLink(from)||!fs::is_directory(from)||fs::exists(to)||isLink(to)||to==to.root_path())throw std::runtime_error("Importacao exige origem classica e destino novo, sem links.");
 FileLock sourceLock;if(!sourceLock.acquire(from/".destinia.lock"))throw std::runtime_error("Feche o jogo antes de copiar o save.");
 if(fs::exists(from.parent_path()/(from.filename().string()+".transaction")))throw std::runtime_error("Save tem recuperacao pendente; abra/feche no classico antes de importar.");
 if(fs::exists(from/".destinia-mode")){if(isLink(from/".destinia-mode"))throw std::runtime_error("Identificacao de origem invalida.");std::ifstream in(from/".destinia-mode",std::ios::binary);std::string text((std::istreambuf_iterator<char>(in)),{});if(text!="DESTINIA_MODE_1\nclassico\n")throw std::runtime_error("Origem nao e classica.");}
 const auto tmp=to.parent_path()/(to.filename().string()+".import-"+uniqueId());fs::create_directories(to.parent_path());if(!fs::create_directory(tmp))throw std::runtime_error("Falha ao criar copia temporaria.");
 try{
  bool progress=false;
  for(const auto& file:fs::directory_iterator(from)){
   const auto n=file.path().filename();if(!modeProgressFile(file.path())&&n!="codex-v1.txt")continue;
   if(isLink(file.path())||!file.is_regular_file()||file.file_size()>64*1024*1024)throw std::runtime_error("Arquivo de progresso invalido.");
   const auto before=fs::last_write_time(file.path());fs::copy_file(file.path(),tmp/n);
   if(before!=fs::last_write_time(file.path())||!sameFileContents(file.path(),tmp/n))throw std::runtime_error("Progresso mudou durante a copia.");
   progress=progress||modeProgressFile(file.path());
  }
  if(!progress)throw std::runtime_error("Origem nao contem partida para importar.");
  // Mark explicitly copied data; verifySaveMode rejects unmarked legacy expanded data.
  std::ofstream marker(tmp/".destinia-mode",std::ios::binary);marker<<"DESTINIA_MODE_1\nexpandido\n";marker.close();if(!marker)throw std::runtime_error("Falha na identificacao da copia.");
  if(fs::exists(to)||isLink(to))throw std::runtime_error("Destino ja existe; copia cancelada.");fs::rename(tmp,to);
 }catch(...){std::error_code ignored;fs::remove_all(tmp,ignored);throw;}
}
}
