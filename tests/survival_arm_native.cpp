#ifdef DESTINIA_TEST_ZOOM
#define main destinia_main_hidden
#include "../host_zoom/main.cpp"
#else
#define main destinia_main_hidden
#include "../host/main.cpp"
#endif
#undef main
#include <cassert>
int main(int argc,char**argv){
 setvbuf(stdout,nullptr,_IONBF,0);if(int rc=runHost(argc,argv))return rc;
 assert(survivalHost.ready);const auto g=exportsMap.at("Game");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 assert(callFn(exportsMap.at("Map_loadData"),{0,1,1}));
 auto sh=[&](u32 at,unsigned n){uint16_t v=uint16_t(n);assert(wr(at,&v,2));};
 sh(g+0x4886,0);sh(g+0x4854,0);sh(g+0x48fa,20);sh(g+0x48f4,100);sh(g+0x48c4,100);
 printf("ARENA_MAP before=%u/%u\n",rd16(g+0x4870),rd16(g+0x4872));
 assert(survivalHost.start());
 for(unsigned n=1;n<=51;n++){
  auto w=destinia::survival::wave(n);assert(survivalHost.arena.currentWave==n&&survivalHost.arena.remaining==w.count);
  for(unsigned j=0;j<w.count;j++){auto p=g+0x14fc+j*136;assert(rd32(p+0x20)>0&&rd32(p+0x24)>0);if(w.elite)assert(rd32(p+0x4c)==1);wr32(p+0x20,0);}
  survivalHost.tick(1000);
  if(!survivalHost.error.empty()){printf("ARENA_ERROR %s\n",survivalHost.error.c_str());return 8;}
 }
 assert(survivalHost.arena.active&&survivalHost.arena.currentWave==52);
 assert(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
 survivalHost.arena.ledger.slots[0].lastStand=true;auto p=g+0x14fc;
 sh(p+0x28,1000);sh(g+0x48ca,0);sh(g+0x4974,0);sh(g+0x48c4,100);sh(g+0x48f4,20);
 for(unsigned i=1;i<=100&&!survivalHost.combat.immuneMs;++i){wr32(exportsMap.at("MMain")+0x3910,i);wr32(exportsMap.at("MMain")+0x3914,0);assert(callFn(exportsMap.at("monsterHitProc"),{p,0}));}
 assert(survivalHost.combat.immuneMs==2000&&rd16(g+0x48f4)==1);
 assert(callFn(exportsMap.at("monsterHitProc"),{p,0}));assert(rd16(g+0x48f4)==1);
 survivalHost.stop(true);survivalHost.flush();
 assert(destinia::survival::load(fs::path(saveDir))->slots[0].max_wave==52);
 printf("SURVIVAL_ARM_OK waves=51 last_stand=2s title=20 atomic_record=52\n");return 0;
}
