#include "../common/codex.h"
#include <cassert>
#include <filesystem>
#include <fstream>
int main(int argc,char** argv) {
  assert(argc==2); std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
  destinia::Codex book;book.names[1]="Fungus";book.names[4]="Scorpion";
  book.select(0);book.observe(100,1,false);assert(book.entries[0][1].kills==0);
  book.observe(100,1,true);book.observe(100,1,true);assert(book.entries[0][1].kills==1);
  book.observe(200,1,true);assert(book.entries[0][1].kills==2);
  book.respawn(100);book.observe(100,1,true);assert(book.entries[0][1].kills==3);
  book.observe(300,999,true);assert(book.entries[0].size()==1);
  book.select(1);book.observe(100,4,true);assert(book.entries[1][4].kills==1);
  book.save(root/"codex-v1.txt");destinia::Codex loaded;loaded.names=book.names;loaded.load(root/"codex-v1.txt");
  assert(loaded.entries[0][1].kills==3);assert(loaded.entries[1][4].kills==1);
  loaded.erase(0);assert(loaded.entries.count(0)==0);assert(loaded.entries[1][4].kills==1);
  std::ofstream(root/"bad.txt")<<"DESTINIA_CODEX_1\n0 1 -2\n";
  try { loaded.load(root/"bad.txt");assert(false); }catch(const std::exception&){}
  assert(loaded.entries[1][4].kills==1);
  book.save(root/"codex-v1.txt");assert(std::filesystem::is_regular_file(root/"codex-v1.txt"));
}
