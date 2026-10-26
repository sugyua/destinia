#include "../common/progression.h"
#include "../common/progression_store.h"
#include <cassert>
#include <iostream>
using namespace destinia::progression;
template<class F> void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
int main(int argc,char** argv){
 if(argc==2&&std::string(argv[1])=="--parity"){
  for(uint32_t n=1;n<=999;++n)std::cout<<n<<' '<<unsigned(rankForLevel(n))<<' '<<xpForNextLevel(n)<<' '<<xpToReachLevel(n)<<' '<<statAtLevel(n,{100,3,1,10})<<'\n';
  for(unsigned d=0;d<5;++d)for(uint32_t cycle:{0u,1u,2u,1000u}){auto m=multipliers(Difficulty(d),cycle);std::cout<<d<<' '<<cycle<<' '<<m.hp<<' '<<m.attack<<' '<<m.xp<<' '<<m.gold<<'\n';}
  for(auto value:{uint64_t(0),uint64_t(1),uint64_t(UINT32_MAX),uint64_t(UINT64_MAX)})for(uint32_t bps:{0u,1u,9999u,10000u})std::cout<<value<<' '<<bps<<' '<<scale(value,bps)<<'\n';
  Ledger sample;sample.characters.emplace(3,Character{xpToReachLevel(999),2,Difficulty::Abissal});std::cout<<encode(sample);return 0;
 }
 assert(xpForNextLevel(1)==29 && xpForNextLevel(99)==1168581);
 assert(xpForNextLevel(999)==0);
 const unsigned starts[]={1,21,41,61,81,100,200,300,400,500,600,700,800,900};
 for(unsigned i=0;i<14;++i){assert(unsigned(rankForLevel(starts[i]))==i);assert(unsigned(rankForLevel(i==13?999:starts[i+1]-1))==i);}
 rejects([]{rankForLevel(0);});rejects([]{rankForLevel(1000);});
 uint64_t total=0;
 for(uint32_t level=1;level<=999;++level){
  assert(xpToReachLevel(level)==total);auto p=query(total);assert(p.level==level&&p.currentXp==0);
  if(level<999){auto edge=query(total+xpForNextLevel(level)-1);assert(edge.level==level&&edge.currentXp==xpForNextLevel(level)-1);total+=xpForNextLevel(level);}
 }
 assert(total>UINT32_MAX);auto gain=awardXp(0,UINT64_MAX);assert(gain.progress.level==999&&gain.accepted==total&&gain.discarded==UINT64_MAX-total);
 rejects([]{query(UINT64_MAX);});rejects([]{checkedAdd(UINT64_MAX,1);});
 assert(scale(UINT64_MAX,10000)==UINT64_MAX);assert(scale(UINT64_MAX,1)==UINT64_MAX/10000);
 rejects([]{scale(UINT64_MAX,10001);});
 assert(statAtLevel(999,{100,3,1,10})==102694);
 rejects([]{statAtLevel(999,{UINT32_MAX,1,0,1});});
 Rules rules;assert(scale(100,multipliers(Difficulty::Normal,0,rules).hp)==100);
 const uint32_t hp[]={10000,15000,25000,40000,60000},reward[]={10000,13000,18000,25000,40000};
 for(unsigned i=0;i<5;++i){auto m=multipliers(Difficulty(i),0,rules);assert(m.hp==hp[i]&&m.attack==hp[i]&&m.xp==reward[i]&&m.gold==reward[i]);}
 rules.ngGrowth={2500,2500,1000,1000};auto ng=multipliers(Difficulty::Dificil,2,rules);assert(ng.hp==22500&&ng.xp==15600);
 rejects([&]{multipliers(Difficulty(5),0,rules);});rejects([&]{multipliers(Difficulty::Abissal,UINT32_MAX,rules);});
 assert(checkedArmU16(65535)==65535);rejects([]{checkedArmU16(65536);});
 assert(checkedArmStat(32767)==32767);rejects([]{checkedArmStat(32768);});
 assert(checkedArmXp(UINT32_MAX)==UINT32_MAX);rejects([]{checkedArmXp(uint64_t(UINT32_MAX)+1);});
 assert(nativeLevelCompatible(99)&&!nativeLevelCompatible(100));
 rejects([]{checkedArmLevel(999);});
 assert(importNative(2,1)==xpToReachLevel(2)+1);rejects([]{importNative(100,0);});rejects([]{importNative(99,1);});
 Ledger book;book.mode=Mode::Expanded;book.rules=rules;book.characters.emplace(3,Character{total,2,Difficulty::Abissal});
 const auto encoded=encode(book);assert(encode(decode(encoded,Mode::Expanded))==encoded);
 rejects([&]{decode(encoded,Mode::Classic);});rejects([&]{decode(encoded+"garbage\n",Mode::Expanded);});rejects([&]{decode(encoded.substr(0,encoded.size()-4),Mode::Expanded);});
 for(const std::string bad:{"-1","+1","00","18446744073709551616","99999999999999999999999999999"}){
  auto malformed=encoded;const auto pos=malformed.find("character 3 ")+12;malformed.replace(pos,malformed.find(' ',pos)-pos,bad);rejects([&]{decode(malformed,Mode::Expanded);});
 }
 rejects([&]{decode(std::string(4097,' '),Mode::Expanded);});
 auto duplicate=encoded;auto row=encoded.substr(encoded.find("character 3 "));row=row.substr(0,row.find('\n')+1);duplicate.replace(duplicate.find("characters 1"),12,"characters 2");duplicate.insert(duplicate.find("end\n"),row);rejects([&]{decode(duplicate,Mode::Expanded);});
 auto name=destinia::fs::u8path(u8"destinia-progressão-test-");name+=destinia::uniqueId();
 auto dir=destinia::fs::temp_directory_path()/name;destinia::fs::create_directory(dir);
 try{
  const std::string sentinel("ORIGINAL\0DATA",13);
  {std::ofstream original(dir/"save0.dat",std::ios::binary);original.write(sentinel.data(),sentinel.size());}
  const auto originalSize=destinia::fs::file_size(dir/"save0.dat");
  assert(!load(dir,Mode::Expanded));save(dir,book);assert(encode(*load(dir,Mode::Expanded))==encoded);
  struct Fail:destinia::AtomicSaveOps{mutable bool attempted=false;void replace(const destinia::fs::path&,const destinia::fs::path&)const override{attempted=true;throw std::runtime_error("disk");}} fail;
  auto newer=book;newer.characters.at(3).ngCycle=4;rejects([&]{save(dir,newer,fail,encoded);});assert(fail.attempted);assert(encode(*load(dir,Mode::Expanded))==encoded);
  rejects([&]{save(dir,newer);});
  {destinia::FileLock other;assert(other.acquire(dir/"progression-v1.lock"));rejects([&]{save(dir,newer,destinia::AtomicSaveOps{},encoded);});}
  save(dir,newer,destinia::AtomicSaveOps{},encoded);assert(encode(*load(dir,Mode::Expanded))==encode(newer));
  rejects([&]{save(dir,book,destinia::AtomicSaveOps{},encoded);});
  assert(destinia::fs::file_size(dir/"save0.dat")==originalSize);std::ifstream original(dir/"save0.dat",std::ios::binary);std::string data((std::istreambuf_iterator<char>(original)),{});assert(data==sentinel);original.close();
  {std::ofstream corrupt(file(dir),std::ios::binary|std::ios::trunc);corrupt<<"corrupt";}
  rejects([&]{save(dir,book);});rejects([&]{load(dir,Mode::Expanded);});
  assert(*readText(dir)=="corrupt");
  for(const auto& item:destinia::fs::directory_iterator(dir))assert(item.path().extension()!=".tmp");
 }catch(...){destinia::fs::remove_all(dir);throw;}destinia::fs::remove_all(dir);
 std::cout<<"progression C++ OK; cap XP="<<total<<"\n";
}
