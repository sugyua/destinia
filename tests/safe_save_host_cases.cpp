#include <cassert>
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
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
 const u32 testId=STUB_COUNT-1;bool shouldFail=false,leaveOpen=false,removeOnly=false;
 handlers[testId]=[&]{
   if(removeOnly){W(0,HEAP_BASE);handlers[importId.at("remove")]();assert(R(0)==0);if(shouldFail)throw std::runtime_error("Falha apos remover");return;}
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
 codexAvailable=true;codex.entries[0][4].kills=9;
 removeOnly=true;shouldFail=true;assert(!callFn(stubAddr(testId),{}));assert(fs::exists(slotFile));assert(codex.entries.at(0).at(4).kills==9);
 shouldFail=false;assert(callFn(stubAddr(testId),{}));assert(!fs::exists(slotFile)&&codex.entries.count(0)==0);
 wr(HEAP_BASE,path,sizeof path);std::ofstream(target)<<"anterior";removeOnly=true;shouldFail=true;
 assert(!callFn(stubAddr(testId),{}));assert(diskText(target)=="anterior");
 shouldFail=false;assert(callFn(stubAddr(testId),{}));assert(!fs::exists(target));
 assert(files.empty());uc_close(uc);uc=nullptr;std::puts("SAFE_SAVE_HOST_OK");
}
