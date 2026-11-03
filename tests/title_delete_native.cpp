#include <cassert>
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef assert
#define assert(e) do { if(!(e)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#e);std::_Exit(1);} } while(false)
static std::string diskText(const fs::path&p){std::ifstream f(p);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char**argv){
 assert(argc==2);saveDir=argv[1];fs::create_directories(saveDir);
 saveTransaction.configure(saveDir);saveTransaction.recover();
 assert(uc_open(UC_ARCH_ARM,UC_MODE_ARM,&uc)==UC_ERR_OK);
 for(auto p:{std::pair<u32,u32>{HEAP_BASE,4096},{STUB_BASE,STUB_COUNT*4},{MAGIC_RET,4096},{STACK_BASE,STACK_SIZE}})assert(uc_mem_map(uc,p.first,p.second,UC_PROT_ALL)==UC_ERR_OK);
 std::vector<u32> bx(STUB_COUNT,0xe12fff1e);wr(STUB_BASE,bx.data(),bx.size()*4);wr32(MAGIC_RET,0xe12fff1e);
 setupStubs();uc_hook h1,h2;uc_hook_add(uc,&h1,UC_HOOK_CODE,(void*)stubHook,nullptr,STUB_BASE,STUB_BASE+STUB_COUNT*4-1);uc_hook_add(uc,&h2,UC_HOOK_CODE,(void*)stubHook,nullptr,MAGIC_RET,MAGIC_RET+3);
 const char path[]="/data/data/game.destiniaeng/files/hero.dat",mode[]="wb",data[]="novo";
 wr(HEAP_BASE,path,sizeof path);wr(HEAP_BASE+256,mode,sizeof mode);wr(HEAP_BASE+512,data,sizeof data);
 const u32 testId=STUB_COUNT-1;bool shouldFail=false,leaveOpen=false,removeOnly=false,removeTwo=false,tryConcurrentTitle=false,checkCodexStaged=false;
 handlers[testId]=[&]{
   if(removeOnly){W(0,HEAP_BASE);handlers[importId.at("remove")]();assert(R(0)==0);if(removeTwo){W(0,HEAP_BASE+768);handlers[importId.at("remove")]();assert(R(0)==0);}if(tryConcurrentTitle){destinia::TitlesStore rival;assert(rival.load(fs::path(saveDir)));rival.unlock(1,destinia::TitleId::SenhorDoAbismo);assert(!rival.save(fs::path(saveDir)));auto old=destinia::survival::readText(fs::path(saveDir));assert(old);auto competing=destinia::survival::decode(*old);competing.slots[1].pendingGold=999;bool denied=false;try{destinia::survival::save(fs::path(saveDir),competing,destinia::AtomicSaveOps{},old);}catch(const std::exception&){denied=true;}assert(denied);}if(checkCodexStaged){FILE*f=saveTransaction.open(fs::path(saveDir)/"codex-v1.txt","rb");assert(f);char buf[512]{};const auto n=std::fread(buf,1,sizeof buf,f);assert(saveTransaction.close(f)==0);const std::string staged(buf,n);assert(staged.find("0 4 9")==std::string::npos&&staged.find("1 5 8")!=std::string::npos);}if(shouldFail)throw std::runtime_error("Falha apos remover");return;}
   W(0,HEAP_BASE);W(1,HEAP_BASE+256);handlers[importId.at("fopen")]();const u32 f=R(0);assert(f);
   W(0,HEAP_BASE+512);W(1,1);W(2,4);W(3,f);handlers[importId.at("fwrite")]();assert(R(0)==4);
   if(!leaveOpen){W(0,f);handlers[importId.at("fclose")]();assert(R(0)==0);}
   assert(diskText(fs::path(saveDir)/"hero.dat")=="anterior");
   if(shouldFail)throw std::runtime_error("Falha ARM depois de fechar o save");
 };
 const auto target=fs::path(saveDir)/"hero.dat";std::ofstream(target)<<"anterior";
 shouldFail=true;assert(!callFn(stubAddr(testId),{}));assert(diskText(target)=="anterior");
 leaveOpen=true;assert(!callFn(stubAddr(testId),{}));assert(diskText(target)=="anterior");
 for(auto&entry:files)assert(saveTransaction.close(entry.second)!=0);files.clear();filePaths.clear();leaveOpen=false;
 shouldFail=false;assert(callFn(stubAddr(testId),{}));assert(diskText(target)=="novo");
 const char slotPath[]="/data/data/game.destiniaeng/files/save0.dat";wr(HEAP_BASE,slotPath,sizeof slotPath);
 const auto slotFile=fs::path(saveDir)/"save0.dat";std::ofstream(slotFile)<<"personagem";
 difficultyProgress=destinia::loadDifficulty(fs::path(saveDir),destinia::progression::Mode::Classic);difficultyReady=true;
 codexAvailable=true;codex.names[4]="um";codex.names[5]="dois";codex.entries[0][4].kills=9;codex.entries[1][5].kills=7;codex.dirty=true;codexFlush();assert(!codex.dirty);
 destinia::survival::Ledger ledger;ledger.slots[0].bestWave=20;ledger.slots[0].titleUnlocked=true;ledger.slots[0].pendingGold=123;
 ledger.slots[1].bestWave=25;ledger.slots[1].titleUnlocked=true;ledger.slots[1].pendingGold=456;
 ledger.slots[2].bestWave=20;ledger.slots[2].titleUnlocked=true;ledger.slots[2].pendingGold=789;
 destinia::survival::save(fs::path(saveDir),ledger);const auto initialSurvival=destinia::survival::readText(fs::path(saveDir));
 destinia::TitlesStore title;title.load(fs::path(saveDir));title.unlock(0,destinia::TitleId::SenhorDoAbismo);title.setEquipped(0,destinia::TitleId::SenhorDoAbismo);title.setExclusive(0,true);title.unlock(1,destinia::TitleId::MestreDaSobrevivencia);title.setEquipped(1,destinia::TitleId::MestreDaSobrevivencia);assert(title.save(fs::path(saveDir)));assert(bossRushUI.titlesStore.load(fs::path(saveDir)));const auto titlesPath=fs::path(saveDir)/"titles-v1.txt";const auto initialTitles=diskText(titlesPath);
 {destinia::FileLock held;assert(held.acquire(fs::path(saveDir)/"titles-v1.lock"));removeOnly=true;shouldFail=false;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(titlesPath)==initialTitles);held.release();}
 {destinia::FileLock held;assert(held.acquire(fs::path(saveDir)/"survival-v1.lock"));removeOnly=true;shouldFail=false;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(titlesPath)==initialTitles&&destinia::survival::readText(fs::path(saveDir))==initialSurvival);held.release();}
 {destinia::FileLock held;assert(held.acquire(fs::path(saveDir)/"codex-v1.lock"));removeOnly=true;shouldFail=false;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(titlesPath)==initialTitles);held.release();}
 const auto codexPath=fs::path(saveDir)/"codex-v1.txt";const auto initialCodex=diskText(codexPath);
 codex.entries[1][5].kills=8;codex.dirty=true;
 std::ofstream(codexPath,std::ios::binary|std::ios::trunc)<<"DESTINIA_CODEX_1\n0 4 10\n1 5 7\n";
 const auto externalCodex=diskText(codexPath);assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(codexPath)==externalCodex);
 std::ofstream(codexPath,std::ios::binary|std::ios::trunc)<<initialCodex;
 removeOnly=true;shouldFail=true;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile));assert(codex.entries.at(0).at(4).kills==9&&codex.entries.at(1).at(5).kills==8&&codex.dirty);assert(diskText(codexPath)==initialCodex);assert(diskText(titlesPath)==initialTitles);assert(bossRushUI.titlesStore.isExclusive(0));assert(destinia::survival::readText(fs::path(saveDir))==initialSurvival);
 shouldFail=false;tryConcurrentTitle=true;checkCodexStaged=true;assert(callFn(stubAddr(testId),{}));tryConcurrentTitle=false;checkCodexStaged=false;assert(!fs::exists(slotFile)&&codex.entries.count(0)==0&&codex.entries.at(1).at(5).kills==8&&!codex.dirty);
 // The surviving character must be able to record new Codex progress after
 // the atomic deletion refreshed the persisted snapshot.
 codex.select(1);codex.observe(1234,5,true);codexFlush();assert(!codex.dirty);
 destinia::Codex surviving;surviving.names=codex.names;surviving.load(codexPath);assert(surviving.entries.at(1).at(5).kills==9&&surviving.entries.count(0)==0);
 destinia::TitlesStore after;assert(after.load(fs::path(saveDir)));assert(after.getEquipped(0)==destinia::TitleId::Nenhum);assert(!after.isUnlocked(0,destinia::TitleId::SenhorDoAbismo));assert(!after.isExclusive(0));assert(after.getEquipped(1)==destinia::TitleId::MestreDaSobrevivencia);assert(after.isUnlocked(1,destinia::TitleId::MestreDaSobrevivencia));assert(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Nenhum);
 auto afterSurvival=destinia::survival::load(fs::path(saveDir));assert(afterSurvival);assert(afterSurvival->slots[0].bestWave==0&&!afterSurvival->slots[0].titleUnlocked&&afterSurvival->slots[0].pendingGold==0);assert(afterSurvival->slots[1].bestWave==25&&afterSurvival->slots[1].pendingGold==456);assert(afterSurvival->slots[2].pendingGold==789);
 // Corrupt auxiliary data must refuse deletion and leave both files untouched.
 std::ofstream(slotFile)<<"personagem";std::ofstream(titlesPath)<<"DESTINIA_TITLES_V1\nequipped 0 7\n";
 const auto corrupt=diskText(titlesPath);assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(titlesPath)==corrupt);
 std::ofstream(titlesPath)<<after.encode();assert(after.load(fs::path(saveDir)));assert(bossRushUI.titlesStore.load(fs::path(saveDir)));
 // Invalid survival progress must also abort the same batch unchanged.
 const auto survivalPath=destinia::survival::file(fs::path(saveDir));const auto validSurvival=destinia::survival::readText(fs::path(saveDir));assert(validSurvival);
 std::ofstream(survivalPath,std::ios::binary|std::ios::trunc)<<"invalido";const auto titlesBeforeCorrupt=diskText(titlesPath);
 assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&diskText(titlesPath)==titlesBeforeCorrupt&&diskText(survivalPath)=="invalido");
 std::ofstream(survivalPath,std::ios::binary|std::ios::trunc)<<*validSurvival;
 // A single ARM call may remove multiple slots; a third slot must survive.
 after.unlock(0,destinia::TitleId::SenhorDoAbismo);after.setEquipped(0,destinia::TitleId::SenhorDoAbismo);after.unlock(2,destinia::TitleId::MestreDaSobrevivencia);after.setEquipped(2,destinia::TitleId::MestreDaSobrevivencia);assert(after.save(fs::path(saveDir)));assert(bossRushUI.titlesStore.load(fs::path(saveDir)));
 const char slotTwo[]="/data/data/game.destiniaeng/files/save2.dat";wr(HEAP_BASE+768,slotTwo,sizeof slotTwo);std::ofstream(fs::path(saveDir)/"save2.dat")<<"dois";const auto beforeMulti=diskText(titlesPath);
 removeTwo=true;shouldFail=true;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile)&&fs::exists(fs::path(saveDir)/"save2.dat")&&diskText(titlesPath)==beforeMulti);
 shouldFail=false;assert(callFn(stubAddr(testId),{}));assert(!fs::exists(slotFile)&&!fs::exists(fs::path(saveDir)/"save2.dat"));
 destinia::TitlesStore multi;assert(multi.load(fs::path(saveDir)));assert(multi.getEquipped(0)==destinia::TitleId::Nenhum&&multi.getEquipped(2)==destinia::TitleId::Nenhum&&multi.getEquipped(1)==destinia::TitleId::MestreDaSobrevivencia);removeTwo=false;
 auto multiSurvival=destinia::survival::load(fs::path(saveDir));assert(multiSurvival);assert(multiSurvival->slots[2].bestWave==0&&!multiSurvival->slots[2].titleUnlocked&&multiSurvival->slots[2].pendingGold==0);assert(multiSurvival->slots[1].bestWave==25&&multiSurvival->slots[1].pendingGold==456);
 wr(HEAP_BASE,path,sizeof path);std::ofstream(target)<<"anterior";removeOnly=true;shouldFail=true;
 assert(!callFn(stubAddr(testId),{}));assert(diskText(target)=="anterior");
 shouldFail=false;assert(callFn(stubAddr(testId),{}));assert(!fs::exists(target));
 assert(files.empty());uc_close(uc);uc=nullptr;std::puts("TITLE_DELETE_ARM_OK");
}
