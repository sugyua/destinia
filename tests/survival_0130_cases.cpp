#include "../common/survival_arena.h"
#include <cassert>
#include <iostream>
using namespace destinia::survival;
int main(int argc,char**){if(argc>1){std::cout<<encode(Ledger{});return 0;}
 assert(wave(3).elite&&!wave(4).elite&&!wave(5).elite&&wave(6).elite);
 assert(wave(5).bossSpecies==100&&wave(10).bossSpecies==101&&wave(15).bossSpecies==102&&wave(15).count==12);
 Arena a;assert(a.start(9,1,42));
 for(unsigned n=1;n<=51;++n){auto w=wave(n);assert(!a.beginNextWave());for(unsigned j=0;j<w.count;++j)assert(a.registerMonster(n*100+j,j==0&&w.bossSpecies?w.bossSpecies:1));assert(!a.observeKill(2,n*100,42,9));assert(!a.observeKill(1,n*100,41,9));assert(!a.observeKill(1,n*100,42,8));for(unsigned j=0;j<w.count;++j){assert(a.observeKill(1,n*100+j,42,9));assert(!a.observeKill(1,n*100+j,42,9));}if(n<51)assert(a.beginNextWave());}
 assert(a.active&&a.ledger.slots[9].titleUnlocked&&a.ledger.slots[9].completions==1);assert(a.ledger.slots[9].totalKills==576&&a.ledger.slots[9].survivalScore>0);auto text=encode(a.ledger);assert(encode(decode(text))==text);auto reward=a.takePending(9);assert(reward.gold==663000&&reward.crystals==17&&reward.pure>0&&reward.abyssal>0&&reward.cash>0);assert(a.takePending(9).gold==0);
 Arena timed;assert(timed.start(2,1));timed.tick(1000,true);assert(timed.ledger.slots[2].bestTimeSeconds==0);timed.tick(2100);assert(timed.ledger.slots[2].bestTimeSeconds==2);timed.abort();assert(timed.ledger.slots[2].bestTimeSeconds==2);
 for(auto bad:{text+"x",std::string(4097,'x'),std::string("DESTINIA_SURVIVAL_V1\nend\n")}){bool fail=false;try{decode(bad);}catch(...){fail=true;}assert(fail);}
 bool fail=false;try{wave(0);}catch(...){fail=true;}assert(fail);assert(!a.start(9,1));assert(a.start(0,2));assert(!a.registerMonster(1,100));assert(a.registerMonster(1,1));a.abort();assert(a.ledger.slots[0].pendingGold==0);std::cout<<"SURVIVAL_51_OK\n";
}
