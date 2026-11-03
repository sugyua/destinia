#include <unicorn/unicorn.h>
#ifdef DESTINIA_TEST_SAVE_OPS
#include "../common/safe_save.h"
struct SurvivalCrashOps : destinia::AtomicSaveOps {
 mutable int count=0;int stop=0;
 void replace(const destinia::fs::path& a,const destinia::fs::path& b)const override {
  destinia::AtomicSaveOps::replace(a,b);if(stop>0&&++count==stop)std::_Exit(71);
 }
 void afterCommitMarker(const destinia::fs::path&)const override {if(stop==99)std::_Exit(72);}
};
static SurvivalCrashOps testSaveOps;
#endif
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
 // Credit must respect the same lock held during character deletion; a
 // rejected attempt may not change money or consume pending rewards.
 survivalHost.arena.ledger.slots[0].pendingGold=123;
 u32 goldBefore=0,goldAfter=0;assert(callFn(exportsMap.at("getMyGold"),{},&goldBefore));
 destinia::FileLock held;assert(held.acquire(fs::path(saveDir)/"survival-v1.lock"));
 assert(!survivalHost.credit());assert(survivalHost.arena.ledger.slots[0].pendingGold==123);
 assert(callFn(exportsMap.at("getMyGold"),{},&goldAfter));assert(goldAfter==goldBefore);
 held.release();assert(survivalHost.credit());assert(survivalHost.arena.ledger.slots[0].pendingGold==0);
 assert(callFn(exportsMap.at("getMyGold"),{},&goldAfter));assert(goldAfter==goldBefore+123);
 if(std::getenv("DESTINIA_CREDIT_ONLY")){std::puts("SURVIVAL_CREDIT_LOCK_OK");return 0;}
 for(unsigned n=1;n<=51;n++){
  auto w=destinia::survival::wave(n);assert(survivalHost.arena.currentWave==n&&survivalHost.arena.remaining==w.count);
  for(unsigned j=0;j<w.count;j++){auto p=g+0x14fc+j*136;assert(rd32(p+0x20)>0&&rd32(p+0x24)>0);assert(rd16(p+0x10)==std::min(99u,20+n-1));if(w.elite)assert(rd32(p+0x4c)==1);wr32(p+0x20,0);}
#ifdef DESTINIA_TEST_SAVE_OPS
  if(n==20 && std::getenv("DESTINIA_SURVIVAL_CRASH_POINT")) {
   const auto baseline=fs::path(std::getenv("DESTINIA_CRASH_BASELINE"));fs::create_directories(baseline);
   for(const char* name:{"save0.dat","refinement-v1.txt","titles-v1.txt","codex-v1.txt","survival-v1.txt"}){
    const auto source=fs::path(saveDir)/name;if(fs::exists(source))fs::copy_file(source,baseline/name,fs::copy_options::overwrite_existing);
   }
   if(std::atoi(std::getenv("DESTINIA_SURVIVAL_CRASH_POINT"))==100){
    destinia::FileLock titleLock;assert(titleLock.acquire(fs::path(saveDir)/"titles-v1.lock"));
    survivalHost.tick(1000);assert(survivalHost.ready&&survivalHost.arena.currentWave==20);
    assert(!survivalHost.canDeleteCharacter());std::_Exit(73);
   }
   testSaveOps.stop=std::atoi(std::getenv("DESTINIA_SURVIVAL_CRASH_POINT"));testSaveOps.count=0;
   survivalHost.tick(1000);std::puts("SURVIVAL_CRASH_NOT_TRIGGERED");return 13;
  }
#endif
  if(n==20 && std::getenv("DESTINIA_TITLE_LOCK_PROBE")) {
   const auto codexPath=fs::path(saveDir)/"codex-v1.txt";
   std::string beforeCodex;{std::ifstream baselineCodex(codexPath,std::ios::binary);beforeCodex.assign(std::istreambuf_iterator<char>(baselineCodex),{});}assert(!beforeCodex.empty());
   destinia::FileLock titleLock;assert(titleLock.acquire(fs::path(saveDir)/"titles-v1.lock"));
   survivalHost.tick(1000);
   codexFlush();assert(codex.dirty);
   std::string duringCodex;{std::ifstream lockedCodex(codexPath,std::ios::binary);duringCodex.assign(std::istreambuf_iterator<char>(lockedCodex),{});}assert(duringCodex==beforeCodex);
   assert(survivalHost.ready && survivalHost.arena.active && survivalHost.arena.currentWave==20);
   assert(!bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
   assert(!survivalHost.canDeleteCharacter());
   if(std::getenv("DESTINIA_TITLE_STOP_PROBE")){
    survivalHost.stop(true);assert(!survivalHost.credit());assert(!survivalHost.canDeleteCharacter());
    titleLock.release();sh(g+0x4886,1);assert(!survivalHost.start());survivalHost.tick(0);
    assert(!bossRushUI.titlesStore.isUnlocked(1,destinia::TitleId::MestreDaSobrevivencia));
    sh(g+0x4886,0);survivalHost.tick(0);
    assert(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
    auto saved=destinia::survival::load(fs::path(saveDir));assert(saved&&saved->slots[0].bestWave==20);
    puts("TITLE_STOP_RETRY_OK");return 0;
   }
   titleLock.release();
   const auto titlePath=fs::path(saveDir)/"titles-v1.txt";assert(!fs::exists(titlePath));
   {std::ofstream external(titlePath,std::ios::binary);external<<"DESTINIA_TITLES_V1\nend\n";assert(external.good());}
   survivalHost.tick(1000);
   assert(survivalHost.ready && survivalHost.arena.active && survivalHost.arena.currentWave==20);
   assert(!bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
   {std::ifstream external(titlePath,std::ios::binary);std::string text((std::istreambuf_iterator<char>(external)),{});assert(text=="DESTINIA_TITLES_V1\nend\n");}
   assert(fs::remove(titlePath));survivalHost.tick(1000);
   assert(survivalHost.ready && survivalHost.arena.active && survivalHost.arena.currentWave==21);
   assert(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
   auto persisted=destinia::survival::load(fs::path(saveDir));assert(persisted&&persisted->slots[0].bestWave==20);
   destinia::TitlesStore reloaded;assert(reloaded.load(fs::path(saveDir)));assert(reloaded.isUnlocked(0,destinia::TitleId::MestreDaSobrevivencia));
   puts("TITLE_LOCK_RETRY_OK");return 0;
  }
  if(n==1&&std::getenv("DESTINIA_FUNGUS_CODEX_LOCK_TEST")){
   destinia::FileLock codexLock;assert(codexLock.acquire(fs::path(saveDir)/"codex-v1.lock"));
   survivalHost.tick(1000);assert(survivalHost.ready&&survivalHost.arena.currentWave==1);
   assert(codex.entries[0][1].kills==w.count&&codex.dirty);
   auto before=destinia::survival::load(fs::path(saveDir));assert(before&&before->slots[0].bestWave==0);
   if(std::getenv("DESTINIA_FUNGUS_CODEX_STOP_TEST")){
    survivalHost.stop(true);assert(!survivalHost.credit());assert(!survivalHost.canDeleteCharacter());
    codexLock.release();sh(g+0x4886,1);assert(!survivalHost.start());survivalHost.tick(0);
    sh(g+0x4886,0);survivalHost.tick(0);assert(!codex.dirty);
    destinia::Codex saved;saved.names=codex.names;saved.load(fs::path(saveDir)/"codex-v1.txt");assert(saved.entries[0][1].kills==w.count);
    auto after=destinia::survival::load(fs::path(saveDir));assert(after&&after->slots[0].bestWave==1);
    puts("FUNGUS_CODEX_STOP_OK");return 0;
   }
   codexLock.release();survivalHost.tick(0);assert(survivalHost.ready&&survivalHost.arena.currentWave==2);
   assert(!codex.dirty);destinia::Codex saved;saved.names=codex.names;saved.load(fs::path(saveDir)/"codex-v1.txt");assert(saved.entries[0][1].kills==w.count);
   auto after=destinia::survival::load(fs::path(saveDir));assert(after&&after->slots[0].bestWave==1);
   puts("FUNGUS_CODEX_LOCK_OK");return 0;
  }
  survivalHost.tick(1000);
  if(!survivalHost.error.empty()){printf("ARENA_ERROR %s\n",survivalHost.error.c_str());return 8;}
  if(n==1&&std::getenv("DESTINIA_FUNGUS_INVALID_CODEX_TEST")){
   assert(!codexWritable);assert(codex.entries[0].count(1)==0);assert(survivalHost.ready&&survivalHost.arena.currentWave==2);
   std::ifstream in(fs::path(saveDir)/"codex-v1.txt",std::ios::binary);std::string contents((std::istreambuf_iterator<char>(in)),{});assert(contents=="invalid-personal-codex");
   puts("FUNGUS_INVALID_CODEX_OK");return 0;
  }
  if(n==1&&std::getenv("DESTINIA_FUNGUS_ARENA_TEST")){
   assert(codex.entries[0][1].kills==w.count);survivalHost.tick(0);assert(codex.entries[0][1].kills==w.count);
   assert(!codex.dirty);destinia::Codex saved;saved.names=codex.names;saved.load(fs::path(saveDir)/"codex-v1.txt");assert(saved.entries[0][1].kills==w.count);
   puts("FUNGUS_ARENA_COUNT_OK");return 0;
  }
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
