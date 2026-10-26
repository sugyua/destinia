#include "../common/difficulty.h"
#include <cassert>
#include <filesystem>
#include <fstream>
using namespace destinia;
int main(int argc,char**argv){
 assert(argc==2);const fs::path dir=argv[1];fs::create_directories(dir);
 DifficultyProgress p;p.book.mode=progression::Mode::Classic;assert(p.current(0)==0&&p.allowed(0,0)&&!p.allowed(0,1));
 assert(!p.defeated(0,107)&&!p.defeated(0,109));assert(p.defeated(0,108));assert(p.allowed(0,1)&&!p.allowed(0,2));
 assert(!p.defeated(0,108));assert(!p.allowed(1,1));assert(!p.change(0,2));assert(p.change(0,1));assert(!p.allowed(0,2));
 assert(p.defeated(0,108));assert(p.allowed(0,2));assert(p.change(0,0));assert(!p.defeated(0,108));assert(!p.allowed(0,3));
 for(int d=1;d<5;++d){assert(p.change(0,d));p.defeated(0,108);}assert(p.current(0)==4&&!p.defeated(0,108));
 p.book.characters[0].totalXp=12345;p.book.characters[0].ngCycle=2;
 saveDifficulty(dir,p);auto loaded=loadDifficulty(dir,progression::Mode::Classic);assert(loaded.current(0)==4&&loaded.allowed(0,4)&&!loaded.allowed(1,1));assert(loaded.book.characters.at(0).totalXp==12345&&loaded.book.characters.at(0).ngCycle==2);
 const auto oldBook=progression::readText(dir),oldUnlocks=readDifficultyUnlocks(dir);
 struct Fail:AtomicSaveOps{int at;mutable int calls=0;explicit Fail(int n):at(n){}void replace(const fs::path& from,const fs::path& to)const override{if(++calls==at)throw std::runtime_error("disk");AtomicSaveOps::replace(from,to);}};
 for(int at:{1,2}){auto failed=loaded;failed.change(0,0);failed.unlocked[1]=1;try{saveDifficulty(dir,failed,Fail(at));assert(false);}catch(const std::exception&){}assert(progression::readText(dir)==oldBook&&readDifficultyUnlocks(dir)==oldUnlocks);}
 auto stale=loaded;loaded.change(0,0);saveDifficulty(dir,loaded);try{saveDifficulty(dir,stale);assert(false);}catch(const std::exception&){}
 auto recovered=loadDifficulty(dir,progression::Mode::Classic);assert(recovered.current(0)==0&&recovered.unlocked[0]==4&&recovered.unlocked[1]==0);
 recovered.erase(0);saveDifficulty(dir,recovered);assert(loadDifficulty(dir,progression::Mode::Classic).current(0)==0);
 std::ofstream(dir/"difficulty-unlocks-v1.txt")<<"garbage";try{loadDifficulty(dir,progression::Mode::Classic);assert(false);}catch(const std::exception&){}
 assert(difficultyLabel(0)=="Normal"&&difficultyLabel(4)=="Abissal");
}
