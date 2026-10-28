#include "../common/refinement.h"
#include <cassert>
#include <iostream>
using namespace destinia::refinement;
int main(){
 Store s; Bytes a{},b{};b[0]=1;int64_t gold=10000000;
 for(int tier=0;tier<3;++tier)assert(s.addCrystals(0,100,tier));
 for(int i=1;i<=15;++i){auto q=s.quote(0,1,a,Kind::Weapon);assert(q.goldCost==500*i*i&&q.crystalsCost==i);auto g=gold;assert(s.upgrade(0,1,a,Kind::Weapon,gold));assert(gold==g-q.goldCost);assert(s.level(0,1,a)==i);}
 assert(s.crystals(0)==85);auto saved=s.encode();auto g=gold;assert(s.upgrade(0,1,a,Kind::Weapon,gold).error==Error::MaxLevel&&gold==g);
 assert(s.upgrade(0,1,b,Kind::Other,gold).error==Error::InvalidKind&&s.encode()==saved);
 assert(s.level(0,1,b)==0&&s.level(0,2,a)==0&&s.level(1,1,a)==0);
 assert(s.upgrade(0,1,b,Kind::Armor,gold));assert(s.level(0,1,b)==1);
 Store restored;restored.decode(s.encode());assert(restored.encode()==s.encode());
 auto before=restored.encode();try{restored.decode(before+"R 0 1 0000000000000000000000000000000000000000 1\n");assert(false);}catch(const std::exception&){}assert(restored.encode()==before);
 int64_t none=0;assert(s.upgrade(0,2,a,Kind::Weapon,none).error==Error::InsufficientGold);assert(s.level(0,2,a)==0);
 assert(scaleRelevant(101,10)==181&&scaleRelevant(101,1)==109&&tag(0).empty()&&tag(10)=="[+10]");
 assert(!s.addCrystals(-1,1)&&!s.addCrystals(0,-1));
 Store empty;auto initial=empty.encode();int64_t plenty=999999;
 assert(empty.upgrade(0,0,a,Kind::Armor,plenty).error==Error::InsufficientCrystals&&plenty==999999&&empty.encode()==initial);
 assert(empty.upgrade(10,0,a,Kind::Armor,plenty).error==Error::InvalidIdentity);
 assert(empty.addCrystals(9,MaxWallet)&&!empty.addCrystals(9,1)&&empty.crystals(9)==MaxWallet);
 Store reused;int64_t rich=10000;reused.addCrystals(0,5);assert(reused.upgrade(0,2,a,Kind::Weapon,rich));
 const auto original=reused.encode();assert(reused.quote(0,2,b,Kind::Other).error==Error::InvalidKind&&reused.encode()==original);
 int64_t poor=0;assert(reused.upgrade(0,2,b,Kind::Weapon,poor).error==Error::InsufficientGold&&reused.encode()==original);
 assert(reused.level(0,2,b)==0);assert(reused.level(0,2,a)==0);
 Store reusedLoaded;reusedLoaded.decode(reused.encode());assert(reusedLoaded.level(0,2,a)==0);
 assert(scaleRelevant(std::numeric_limits<int64_t>::max(),0)==std::numeric_limits<int64_t>::max());
 try{scaleRelevant(std::numeric_limits<int64_t>::max(),10);assert(false);}catch(const std::overflow_error&){}
 for(const std::string& bad:{saved+"\n",std::string("DESTINIA_REFINEMENT_V1\n"),std::string(120001,'x')}){auto original=s.encode();try{s.decode(bad);assert(false);}catch(const std::exception&){}assert(s.encode()==original);}
 std::cout<<s.encode();
}
