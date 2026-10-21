#include "../common/runtime_support.h"
#include <cassert>
#include <fstream>
#include <iostream>
namespace fs = std::filesystem;
static std::string contents(const fs::path& p) {
  std::ifstream f(p,std::ios::binary); return {std::istreambuf_iterator<char>(f),{}};
}
int main(int argc,char** argv) {
  assert(argc==2); fs::path root=argv[1]; fs::create_directories(root);
  auto save=root/"partida com espacos"; fs::create_directories(save/"personagem");
  std::ofstream(save/"personagem"/"progresso.bin",std::ios::binary)<<"original";
  destinia::FileLock first,second;
  assert(first.acquire(save/".destinia.lock"));
  assert(!second.acquire(save/".destinia.lock"));
  first.release(); assert(second.acquire(save/".destinia.lock")); second.release();
  for(int i=0;i<13;++i) {
    auto backup=destinia::backupSave(save,10);
    assert(contents(backup/"personagem"/"progresso.bin")=="original");
    assert(!fs::exists(backup/".destinia.lock"));
  }
  auto backups=root/"partida com espacos_backups";
  int count=0; for(auto& e:fs::directory_iterator(backups)) if(e.is_directory()) ++count;
  assert(count==10); assert(contents(save/"personagem"/"progresso.bin")=="original");
  fs::create_directories(backups/"pasta do usuario");
  std::ofstream(backups/"pasta do usuario"/"nao apagar.txt")<<"preservar";
  destinia::backupSave(save,10);
  assert(fs::exists(backups/"pasta do usuario"/"nao apagar.txt"));
  std::error_code ec; fs::create_symlink(root/"fora",save/"atalho",ec);
  if(!ec) {
    bool rejected=false; try { destinia::backupSave(save,10); } catch(const std::exception&) { rejected=true; }
    assert(rejected); fs::remove(save/"atalho");
  }
  auto empty=root/"vazia"; fs::create_directories(empty);
  assert(destinia::backupSave(empty,10).empty());
  auto blocked=root/"bloqueada"; fs::create_directories(blocked);
  std::ofstream(blocked/"progresso")<<"intacto";
  std::ofstream(root/"bloqueada_backups")<<"impede criar diretorio";
  bool rejected=false; try { destinia::backupSave(blocked,10); } catch(const std::exception&) { rejected=true; }
  assert(rejected); assert(contents(blocked/"progresso")=="intacto");
  auto reports=root/"relatorios";
  fs::create_directories(reports/"pasta do usuario");
  std::ofstream(reports/"pasta do usuario"/"nao apagar.txt")<<"preservar";
  for(int n=0;n<13;++n) {
    destinia::SessionLog logger;
    assert(logger.start(reports,"host",256,3));
    for(int i=0;i<30;++i) logger.write("evento "+std::to_string(i)+std::string(70,'x'));
    logger.close();
  }
  count=0;
  for(auto& e:fs::directory_iterator(reports)) if(e.path().filename().string().find("jogo_host_")==0) {
    ++count; int logs=0;
    for(auto& f:fs::directory_iterator(e.path())) if(f.path().extension()==".txt") {
      ++logs; assert(f.file_size()<=256);
    }
    assert(logs==3);
  }
  assert(count==3); assert(fs::exists(reports/"pasta do usuario"/"nao apagar.txt"));
  destinia::SessionLog active;
  assert(active.start(reports,"host",256,3)); auto activePath=active.directory();
  for(int n=0;n<4;++n) { destinia::SessionLog other; assert(other.start(reports,"host",256,3)); }
  assert(fs::exists(activePath/"log.txt")); active.close();
  std::cout<<"OK: backup, restauracao por copia, retencao, fontes intactas, links, falhas, trava e logs limitados\n";
}
