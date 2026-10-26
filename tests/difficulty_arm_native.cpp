#ifdef main
#undef main
#endif
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#include <cassert>
int main(int argc,char** argv){
 setvbuf(stdout,nullptr,_IONBF,0);if(int rc=runHost(argc,argv))return rc;
 assert(difficultyReady);const auto g=exportsMap.at("Game"),p=g+0x14fc;
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 unsigned loadedMap=0;for(unsigned map=1;map<=8;++map){assert(callFn(exportsMap.at("Map_loadData"),{0,map,1}));if(rd32(g+0x1f9c)>0){loadedMap=map;break;}}
 assert(rd32(g+0x1f9c)>0&&rd32(p));
 auto sh=[&](u32 at,unsigned n){uint16_t v=uint16_t(n);assert(wr(at,&v,2));};
 sh(g+0x4886,0);sh(g+0x4854,0);sh(g+0x48fa,20);sh(p+8,1);sh(p+0xa,0);sh(p+0x10,20);
 wr32(g+0x1f9c,1);wr32(p+0x24,101);wr32(p+0x20,101);sh(p+0x28,100);sh(g+0x48ca,0);sh(g+0x48f4,30000);
 auto choose=[&](unsigned d){difficultyProgress.book.characters[0].difficulty=destinia::progression::Difficulty(d);};
 const auto init=exportsMap.at("Mon_initMonster"),gain=exportsMap.at("GameCanvas_getGainExp"),drop=exportsMap.at("GameCanvas_dropItem");
 choose(0);u32 xpBase=0;assert(callFn(gain,{p},&xpBase));assert(xpBase>0);
 std::vector<uint8_t> baseline(32056);assert(rd(g,baseline.data(),baseline.size()));
 auto restore=[&](){assert(wr(g,baseline.data(),baseline.size()));};
 for(unsigned d=0;d<5;++d){restore();choose(d);auto m=destinia::progression::multipliers(destinia::progression::Difficulty(d),0);
  assert(callFn(init,{p}));const auto hp=destinia::progression::scale(101,m.hp);assert(rd32(p+0x24)==hp&&rd32(p+0x20)==hp);
  assert(callFn(init,{p}));assert(rd32(p+0x24)==hp); // Respawn is not another multiplier.
  u32 xp=0;assert(callFn(gain,{p},&xp));assert(xp==destinia::progression::scale(xpBase,m.xp));
  wr32(g+0x48fc,0);assert(callFn(exportsMap.at("GameCanvas_gainExpProc"),{xp,0}));assert(rd32(g+0x48fc)==xp);
  wr32(g+0x48fc,0);assert(callFn(exportsMap.at("GameCanvas_gainExpProc"),{17,0}));assert(rd32(g+0x48fc)==17); // Quest/raw awards unchanged.
  assert(callFn(exportsMap.at("setMyGold"),{12345}));u32 gold=0;assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==12345);
 }
 // Live changes preserve the fraction of remaining HP and do not revive corpses.
 restore();choose(1);assert(callFn(init,{p}));wr32(p+0x20,75);choose(4);difficultyArm.refresh();printf("LIVE_HP max=%u current=%u slot=%u diff=%u\n",rd32(p+0x24),rd32(p+0x20),rd16(g+0x4886),difficultyProgress.current(0));assert(rd32(p+0x24)==606&&rd32(p+0x20)==300);
 wr32(p+0x20,0);choose(0);difficultyArm.refresh();assert(rd32(p+0x24)==101&&rd32(p+0x20)==0);
 // Preflight refuses native overflow without mutating a monster or save.
 wr32(p+0x24,INT32_MAX);assert(callFn(init,{p}));bool rejected=false;try{difficultyArm.validateProfile(0,destinia::progression::multipliers(destinia::progression::Difficulty::Abissal,0));}catch(const std::exception&){rejected=true;}assert(rejected&&rd32(p+0x24)==INT32_MAX);
 restore();choose(4);assert(callFn(init,{p}));sh(g+0x4886,1);u32 otherXp=0;assert(callFn(gain,{p},&otherXp));assert(otherXp==xpBase);sh(g+0x4886,0);
 // A copied/pet/network monster is not a campaign target, even with the same species.
 restore();choose(4);const auto pet=hmalloc(136);std::array<uint8_t,136> copied{};assert(rd(p,copied.data(),copied.size()));assert(wr(pet,copied.data(),copied.size()));assert(callFn(init,{pet}));assert(rd32(pet+0x24)==101);assert(callFn(gain,{pet},&otherXp));assert(otherXp==xpBase);
 sh(p+0xa,200);assert(callFn(init,{p}));assert(rd32(p+0x24)==101);sh(p+0xa,0);
 // Same native RNG seed and defense/crit path: final incoming damage gets exactly the profile factor.
 const auto rng=exportsMap.at("MMain")+0x3910;
 auto seed=[&](u32 value){wr32(rng,value);wr32(rng+4,0);};
 auto hit=[&](unsigned d,u32 randomSeed,unsigned defense=0,unsigned block=0){restore();choose(d);sh(g+0x48ca,defense);sh(g+0x4974,block);seed(randomSeed);assert(callFn(init,{p}));assert(callFn(exportsMap.at("monsterHitProc"),{p,0}));return 30000-rd16(g+0x48f4);};
 u32 hitSeed=1,baseDamage=0;while(hitSeed<100&&!(baseDamage=hit(0,hitSeed)))++hitSeed;assert(baseDamage>0);
 for(unsigned d=0;d<5;++d){const auto m=destinia::progression::multipliers(destinia::progression::Difficulty(d),0);const auto actual=hit(d,hitSeed);printf("PROFILE %u HP=%llu DAMAGE=%u XP=%llu\n",d,(unsigned long long)destinia::progression::scale(101,m.hp),actual,(unsigned long long)destinia::progression::scale(xpBase,m.xp));assert(actual==destinia::progression::scale(baseDamage,m.attack));}
 for(unsigned defense:{0u,60u,500u})for(unsigned block:{0u,1u}){const auto original=hit(0,hitSeed,defense,block);for(unsigned d=0;d<5;++d){const auto actual=hit(d,hitSeed,defense,block);assert(actual==destinia::progression::scale(original,destinia::progression::multipliers(destinia::progression::Difficulty(d),0).attack));}}
 // Exhaust the same deterministic sample in Normal and Abissal. Every item record,
 // flag and RNG state must match. Only gold's amount/text may differ, by exactly 4x.
 unsigned goldSeen=0,itemsSeen=0,emptySeen=0;u32 goldSeed=0,baseGold=0;
 std::array<uint8_t,20> normalFlags{},abyssFlags{};std::array<uint8_t,1200> normalDrops{},abyssDrops{};std::array<uint8_t,8> normalRng{},abyssRng{};
 for(u32 n=1;n<=256;++n){
  auto generate=[&](unsigned d,auto& flags,auto& records,auto& state){restore();choose(d);seed(n);std::array<uint8_t,20> empty{};assert(wr(g+0x3e76,empty.data(),empty.size()));assert(callFn(drop,{100,100,p}));assert(rd(g+0x3e76,flags.data(),flags.size()));assert(rd(g+0x3e90,records.data(),records.size()));assert(rd(rng,state.data(),state.size()));};
  generate(0,normalFlags,normalDrops,normalRng);generate(4,abyssFlags,abyssDrops,abyssRng);assert(normalFlags==abyssFlags&&normalRng==abyssRng);
  bool any=false;for(unsigned i=0;i<20;++i)if(normalFlags[i]){any=true;const unsigned at=i*60;const auto kind=normalDrops[at+48]|normalDrops[at+49]<<8;const auto otherKind=abyssDrops[at+48]|abyssDrops[at+49]<<8;assert(kind==otherKind);
   if(kind==5){++goldSeen;const auto raw=normalDrops[at+50]|normalDrops[at+51]<<8;const auto actual=abyssDrops[at+50]|abyssDrops[at+51]<<8;assert(actual==raw*4);if(!goldSeed){goldSeed=n;baseGold=raw;}}else{++itemsSeen;assert(std::equal(normalDrops.begin()+at,normalDrops.begin()+at+60,abyssDrops.begin()+at));}
  }if(!any)++emptySeen;
 }
 assert(goldSeen&&itemsSeen&&emptySeen);printf("DROP_PARITY seeds=256 gold=%u items=%u empty=%u\n",goldSeen,itemsSeen,emptySeen);
 // Gold is multiplied on creation only, never again on pickup or setMyGold.
 for(unsigned d:{0u,4u}){restore();choose(d);seed(goldSeed);std::array<uint8_t,20> empty{};assert(wr(g+0x3e76,empty.data(),empty.size()));sh(g+0x4890,100);sh(g+0x4892,100);sh(g+0x4866,0);assert(callFn(drop,{100,100,p}));assert(callFn(exportsMap.at("setMyGold"),{1000}));assert(callFn(exportsMap.at("GameCanvas_mKeyPressed"),{6}));assert(callFn(exportsMap.at("GameCanvas_mKeyReleased"),{6}));u32 credited=0;assert(callFn(exportsMap.at("getMyGold"),{},&credited));assert(credited==1000+baseGold*(d?4:1));}
 // Freeze rewards at death, not at the later drop/XP animation callback.
 for(const auto pair:std::array<std::pair<unsigned,unsigned>,3>{{{0,4},{1,0},{4,0}}}){
  restore();choose(pair.first);assert(callFn(init,{p}));wr32(p+0x20,0);choose(pair.second);difficultyArm.refresh();
  u32 xp=0;assert(callFn(gain,{p},&xp));const auto m=destinia::progression::multipliers(destinia::progression::Difficulty(pair.first),0);assert(xp==destinia::progression::scale(xpBase,m.xp));
  std::array<uint8_t,20> empty{};assert(wr(g+0x3e76,empty.data(),empty.size()));seed(goldSeed);assert(callFn(drop,{100,100,p}));assert(rd16(g+0x3e90+48)==5);assert(rd16(g+0x3e90+50)==destinia::progression::scale(baseGold,m.gold));
 }
 // The Part-1 unlock uses the captured killing tier even after a legitimate UI switch.
 for(unsigned before:{0u,1u}){
  restore();choose(0);difficultyProgress.unlocked[0]=1;destinia::saveDifficulty(fs::path(saveDir),difficultyProgress);
  sh(p+8,108);sh(p+0xa,108);assert(callFn(init,{p}));
  if(before){difficultyRequest=1;difficultyRequestSlot=0;difficultyFlush();assert(difficultyProgress.current(0)==1&&rd32(p+0x24)==151);}
  wr32(p+0x20,0);difficultyRequest=before?0:1;difficultyRequestSlot=0;difficultyFlush();
  assert(callFn(exportsMap.at("checkQuestMonster"),{p}));difficultyFlush();
  assert(difficultyProgress.unlocked[0]==(before?2u:1u));assert(difficultyProgress.current(0)==(before?0u:1u));
 }
 std::puts("LATE_REWARD_AND_DIABLO_TIER_OK");
 // Boss fixture follows the same native initializer and campaign array guard.
 choose(4);wr32(p+0x24,101);assert(callFn(init,{p}));assert(rd16(p+8)==108&&rd32(p+0x24)==606);
 // Reload actual map data with identical RNG; native spawn bases are regenerated once.
 choose(0);seed(100);assert(callFn(exportsMap.at("Map_loadData"),{0,loadedMap,1}));const auto originalMapHP=rd32(p+0x24);
 choose(4);seed(100);assert(callFn(exportsMap.at("Map_loadData"),{0,loadedMap,1}));assert(rd32(p+0x24)==uint64_t(originalMapHP)*6);assert(callFn(init,{p}));assert(rd32(p+0x24)==uint64_t(originalMapHP)*6);
 printf("MAP_SPAWN hp=%u abissal=%u\n",originalMapHP,rd32(p+0x24));
 assert(difficultyArm.error.empty());std::puts("DIFFICULTY_GAMEPLAY_ARM_OK");return 0;
}
