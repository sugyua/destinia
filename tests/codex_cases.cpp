#include "../common/codex.h"
#include "../common/codex_ui.h"
#include <cassert>
#include <filesystem>
#include <fstream>
struct CollisionOps : destinia::CodexSaveOps {
 std::filesystem::path collision;
 explicit CollisionOps(std::filesystem::path p):collision(std::move(p)){}
 std::filesystem::path temporary(const std::filesystem::path&) const override{return collision;}
};
static std::string fileText(const std::filesystem::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int main(int argc,char** argv) {
  assert(argc==2); std::filesystem::path root=argv[1];std::filesystem::create_directories(root);
  destinia::Codex book;book.names[1]="Fungus";book.names[4]="Scorpion";
  // No Códex button may cover gameplay outside the in-game menu.
  std::vector<uint8_t> screen(400*240*2,0x35);auto original=screen;
  destinia::codexPaint(screen.data(),400,240,book,false,0,true);
  assert(screen==original);
  // An actual portrait must occupy its catalog cell, including opaque black.
  destinia::Codex pictured;pictured.names[1]="Fungus";pictured.select(0);pictured.observe(1,1,true);
  destinia::CodexPortraits art;art[1]={2,1,{0x10000,0x1f81f}};
  destinia::codexPaint(screen.data(),400,240,pictured,true,0,true,true,true,&art);
  const auto cell=destinia::codexCell(0,400,240);const int at=((cell.y+10)*400+cell.x+3)*2;
  assert(screen[at]==0 && screen[at+1]==0);assert(pictured.entries[0][1].kills==1);
  auto missing=screen;destinia::codexPaint(missing.data(),400,240,pictured,true,0,true,true,true);
  assert(missing!=screen);
  // Unknown catalog positions remain visible, with no name or art disclosure.
  destinia::Codex catalog;for(int id=1;id<=73;++id)catalog.names[id]="SECRET";catalog.select(0);
  auto hidden=std::vector<uint8_t>(400*240*2),changed=hidden;
  destinia::codexPaint(hidden.data(),400,240,catalog,true,0,true,true,true,&art);
  catalog.names[1]="OTHER SECRET";destinia::codexPaint(changed.data(),400,240,catalog,true,0,true,true,true);
  assert(hidden==changed);assert(catalog.entries.empty());
  for(auto size:std::vector<std::pair<int,int>>{{400,240},{800,480}})for(size_t i=0;i<32;++i){auto c=destinia::codexCell(i,size.first,size.second);assert(destinia::codexHitTest(c.x+16,c.y+16,73,0,size.first,size.second)==int(i));}
  auto ninth=destinia::codexCell(8,400,240),tenth=destinia::codexCell(9,400,240);assert(destinia::codexHitTest(ninth.x+1,ninth.y+1,73,2,400,240)==72);assert(destinia::codexHitTest(tenth.x+1,tenth.y+1,73,2,400,240)==-1);
  auto detail=destinia::codexDetails(400,240),next=destinia::codexNext(400,240);assert(next.y>=detail.y+detail.h);
  catalog.observe(1,1,true);hidden.assign(hidden.size(),0);changed=hidden;destinia::codexPaint(hidden.data(),400,240,catalog,true,0,true,true,true,&art,0);
  catalog.observe(2,1,true);destinia::codexPaint(changed.data(),400,240,catalog,true,0,true,true,true,&art,0);
  int different=0;for(int y=0;y<240;++y)for(int x=0;x<400;++x){int at=(y*400+x)*2;if(hidden[at]!=changed[at] || hidden[at+1]!=changed[at+1]){++different;assert(detail.contains(x,y));}}assert(different>0);
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
  destinia::FileLock held;assert(held.acquire(root/"codex-v1.lock"));book.select(0);book.observe(900,1,true);
  bool denied=false;try{book.save(root/"codex-v1.txt");}catch(const destinia::CodexBusyError&){denied=true;}
  assert(denied&&book.dirty);destinia::Codex still;still.names=book.names;still.load(root/"codex-v1.txt");assert(still.entries[0][1].kills==3);
  held.release();book.save(root/"codex-v1.txt");assert(!book.dirty);still.load(root/"codex-v1.txt");assert(still.entries[0][1].kills==4);
  const auto collision=root/"personal.tmp";std::ofstream(collision)<<"PERSONAL";const auto before=fileText(root/"codex-v1.txt");
  book.observe(901,1,true);CollisionOps ops(collision);bool refused=false;
  try{book.save(root/"codex-v1.txt",ops);}catch(const std::exception&){refused=true;}
  assert(refused&&book.dirty&&fileText(collision)=="PERSONAL"&&fileText(root/"codex-v1.txt")==before);
  book.save(root/"codex-v1.txt");assert(!book.dirty);
  // A cooperating writer may have advanced a different slot since this
  // instance loaded; a stale snapshot must not overwrite that progress.
  destinia::Codex concurrent;concurrent.names=book.names;concurrent.load(root/"codex-v1.txt");concurrent.select(1);concurrent.observe(777,4,true);concurrent.save(root/"codex-v1.txt");
  const auto newer=fileText(root/"codex-v1.txt");book.observe(903,1,true);bool stale=false;
  try{book.save(root/"codex-v1.txt");}catch(const std::exception&){stale=true;}
  assert(stale&&book.dirty&&fileText(root/"codex-v1.txt")==newer);
  const auto recovery=book.saveRecovery(root);assert(fileText(recovery)==book.encode()&&fileText(root/"codex-v1.txt")==newer&&book.dirty);
  // An invalid lock path is an access error, not a transient lock held by a
  // cooperating writer. Keep the directory and retain unsaved progress.
  std::filesystem::remove(root/"codex-v1.lock");std::filesystem::create_directory(root/"codex-v1.lock");
  book.observe(902,1,true);bool accessError=false,misclassifiedBusy=false;
  try{book.save(root/"codex-v1.txt");}catch(const destinia::CodexBusyError&){misclassifiedBusy=true;}catch(const std::exception&){accessError=true;}
  assert(accessError&&!misclassifiedBusy&&book.dirty&&std::filesystem::is_directory(root/"codex-v1.lock"));
}
