#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEngine0130(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine0130
#define main originalMain0130
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
int main(int argc,char** argv){
 if(int status=originalMain0130(argc,argv))return status;
 const auto g=exportsMap.at("Game");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 wr16(g+0x4886,0);wr16(g+0x48fa,20);wr16(g+0x4854,0);wr16(g+0x48f4,100);
 for(int i=0;i<6;++i)wr32(g+0x4584+4*i,0);
 const uint16_t attributes[4]={20,15,10,10};wr(g+0x48ba,attributes,8);
 u32 pool=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{1},&pool));
 const auto equipment=g+0x2ed6+20*pool;wr32(g+0x4584,equipment);
 assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 const auto originalAttack=rd16(g+0x48c8);
 assert(refinementHost.ready);
 for(int tier=0;tier<3;++tier)assert(refinementHost.store.addCrystals(0,1000,tier));
 assert(callFn(exportsMap.at("setMyGold"),{5000000}));
 for(int level=1;level<=15;++level){assert(refinementHost.upgrade(equipment));assert(refinementHost.itemLevel(equipment)==level);}
 assert(refinementHost.suffix(equipment)=="[+15]");
 const auto refinedAttack=rd16(g+0x48c8);assert(refinedAttack>originalAttack);
 assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));assert(rd16(g+0x48c8)==refinedAttack);
 assert(!refinementHost.upgrade(equipment));
 assert(fs::is_regular_file(fs::path(saveDir)/"refinement-v1.txt"));
 assert(fs::is_regular_file(fs::path(saveDir)/"save0.dat"));
 // Use a real native armor record: this fixture carries DEF, not a max-HP
 // attribute. HP refinement is covered at the shared policy boundary.
 u32 armorPool=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{97},&armorPool));const auto armor=g+0x2ed6+20*armorPool;wr32(g+0x4588,armor);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));const auto armorDEF=rd16(g+0x48ca);assert(refinementHost.upgrade(armor));assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));assert(rd16(g+0x48ca)>armorDEF);
 // Actual ARM-created enemies. Deaths are synthetic, so this is not gameplay QA.
 assert(callFn(exportsMap.at("Map_loadData"),{0,61,1}));assert(callFn(exportsMap.at("Map_loadTile"),{0}));wr16(g+0x4854,0);wr16(g+0x48f4,100);
 const auto originalArea=rd16(g+0x4870),originalMap=rd16(g+0x4872);
 assert(survivalHost.start());
 survivalHost.combat.enabled=true;wr16(g+0x48f4,1);const auto rng=exportsMap.at("MMain")+0x3910;wr32(rng,1);wr32(rng+4,0);
 for(unsigned attempt=0;attempt<100&&!survivalHost.combat.immuneMs;++attempt)assert(callFn(exportsMap.at("monsterHitProc"),{g+0x14fc,0}));
 assert(rd16(g+0x48f4)==1&&survivalHost.combat.immuneMs==2000&&survivalHost.combat.cooldownMs==120000);survivalHost.combat.enabled=false;survivalHost.combat.reset();wr16(g+0x48f4,100);
 for(unsigned wave=1;wave<=51;++wave){
  assert(survivalHost.arena.currentWave==wave);
  const auto plan=destinia::survival::wave(wave);
  assert(rd32(g+0x1f9c)==plan.count);
  if(plan.bossSpecies)assert(rd16(g+0x14fc+8)==plan.bossSpecies);
  for(unsigned i=0;i<plan.count;++i)assert((rd32(g+0x14fc+136*i+0x4c)!=0)==plan.elite);
  for(unsigned i=0;i<plan.count;++i){const auto p=g+0x14fc+136*i;assert(int32_t(rd32(p+0x24))>0);wr32(p+0x20,0);}
  survivalHost.tick(62);
  if(!survivalHost.error.empty()){std::fprintf(stderr,"ARENA_ERROR %s\n",survivalHost.error.c_str());return 3;}
  assert(survivalHost.arena.ledger.slots[0].bestWave==wave);
  assert(survivalHost.arena.ledger.slots[0].titleUnlocked==(wave>=20));
  assert(survivalHost.arena.ledger.slots[0].pendingGold==0);
 }
 survivalHost.stop(true);survivalHost.flush();
 assert(rd16(g+0x4870)==originalArea&&rd16(g+0x4872)==originalMap);
 assert(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
 bossRushUI.titlesStore.setEquipped(0,destinia::TitleId::MestreDaSobrevivencia);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));std::array<uint16_t,13> titled{},repeated{};rd(g+0x48ba,titled.data(),26);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));rd(g+0x48ba,repeated.data(),26);assert(titled==repeated);
 bossRushUI.currentTab=5;assert(bossRushUI.getItemsCountInCurrentTab()==8);
 assert(fs::is_regular_file(fs::path(saveDir)/"survival-v1.txt"));
 std::puts("UPDATE_0130_NATIVE_ARM_OK");return 0;
}
