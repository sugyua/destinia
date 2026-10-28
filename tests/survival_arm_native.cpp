#include <unicorn/unicorn.h>
static uc_err keepSurvivalTestEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepSurvivalTestEngine
#define main originalSurvivalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
#include <cassert>
int main(int argc,char**argv){
 setvbuf(stdout,nullptr,_IONBF,0);
 std::string temporarySave;std::vector<std::string> isolatedArgs;std::vector<char*> isolatedArgv;
 bool hasSave=false;for(int i=1;i<argc;++i)if(std::string(argv[i])=="--save")hasSave=true;
 int hostArgc=argc;char** hostArgv=argv;
 if(!hasSave){temporarySave=(fs::temp_directory_path()/("destinia-survival-arm-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();fs::create_directories(temporarySave);for(int i=0;i<argc;++i)isolatedArgs.emplace_back(argv[i]);isolatedArgs.emplace_back("--save");isolatedArgs.push_back(temporarySave);for(auto&arg:isolatedArgs)isolatedArgv.push_back(arg.data());hostArgc=int(isolatedArgv.size());hostArgv=isolatedArgv.data();}
 if(int rc=originalSurvivalMain(hostArgc,hostArgv)){if(!temporarySave.empty())fs::remove_all(temporarySave);return rc;}
 assert(survivalHost.ready);const auto g=exportsMap.at("Game");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 // Match the stable arena fixture used by update_0130_native: map 61 has a
 // fully initialized walkable grid for the pre-start host checks.
 assert(callFn(exportsMap.at("Map_loadData"),{0,61,1}));
 assert(callFn(exportsMap.at("Map_loadTile"),{0}));
 auto sh=[&](u32 at,unsigned n){uint16_t v=uint16_t(n);assert(wr(at,&v,2));};
 sh(g+0x4886,0);sh(g+0x4854,0);sh(g+0x48fa,20);sh(g+0x48f4,100);sh(g+0x48c4,100);
 assert(survivalHost.start());
 for(unsigned n=1;n<=51;n++){
  auto w=destinia::survival::wave(n);assert(survivalHost.arena.currentWave==n&&survivalHost.arena.remaining==w.count);
  for(unsigned j=0;j<w.count;j++){auto p=g+0x14fc+j*136;assert(rd32(p+0x20)>0&&rd32(p+0x24)>0);assert(rd16(p+0x10)==std::min(99u,20+n-1));if(w.elite)assert(rd32(p+0x4c)==1);wr32(p+0x20,0);}
  survivalHost.tick(1000);
  if(!survivalHost.error.empty()){printf("ARENA_ERROR %s\n",survivalHost.error.c_str());return 8;}
 }
 assert(survivalHost.arena.active&&survivalHost.arena.currentWave==52);
 assert(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
 survivalHost.combat.enabled=true;auto p=g+0x14fc;
 sh(p+0x28,1000);sh(g+0x48ca,0);sh(g+0x4974,0);sh(g+0x48c4,100);sh(g+0x48f4,20);
 for(unsigned i=1;i<=100&&!survivalHost.combat.immuneMs;++i){wr32(exportsMap.at("MMain")+0x3910,i);wr32(exportsMap.at("MMain")+0x3914,0);assert(callFn(exportsMap.at("monsterHitProc"),{p,0}));}
 assert(survivalHost.combat.immuneMs==2000&&rd16(g+0x48f4)==1);
 assert(callFn(exportsMap.at("monsterHitProc"),{p,0}));assert(rd16(g+0x48f4)==1);
 survivalHost.stop(true);survivalHost.flush();
 assert(rd32(g+0x1f9c)==0);
 assert(destinia::survival::load(fs::path(saveDir))->slots[0].bestWave==51);
 survivalHost.combat.elixirMs=9000;survivalHost.combat.cooldownMs=120000;survivalHost.combat.immuneMs=2000;
 sh(g+0x4886,1);survivalHost.tick(0);
 assert(survivalHost.combat.enabled&&survivalHost.combat.elixirMs==0&&survivalHost.combat.cooldownMs==0&&survivalHost.combat.immuneMs==0);
 printf("SURVIVAL_ARM_OK waves=51 last_stand=2s title=20 atomic_record=51 slot_reset=ok\n");if(!temporarySave.empty())fs::remove_all(temporarySave);return 0;
}
