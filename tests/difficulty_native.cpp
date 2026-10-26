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
static std::string observedClass;
static void observeClass(uc_engine*,uint64_t,uint32_t,void*){if((regLR()&~1u)==SO_BASE+0x21c7a)observedClass=cstr(R(1));}
int main(int argc,char**argv){
 if(int rc=runHost(argc,argv))return rc;assert(difficultyReady);
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 auto sh=[&](u32 at,unsigned n){uint16_t v=uint16_t(n);assert(wr(at,&v,2));};
 sh(g+0x4854,16);sh(g+0x4886,0);sh(ui+0x76,16);sh(ui+0x54,4);wr32(ui+0x7c,0x80000001u);
 assert(difficultySystem());
 assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{0}));
 const auto screenDescriptor=rd32(exportsMap.at("frameBuffer"));std::vector<uint8_t> systemRaster(FB_BYTES);assert(rd(rd32(screenDescriptor+8),systemRaster.data(),systemRaster.size()));difficultyPainter.paint(systemRaster.data());
 if(const char* prefix=std::getenv("DESTINIA_DIFFICULTY_CAPTURE"))writeBmp(std::string(prefix)+"system-"+std::to_string(preferences.uiScale)+"_"+std::to_string(SCREEN_W)+".bmp",systemRaster.data());
assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{9}));assert(difficultyButton);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));assert(difficultyOpen);
 assert(difficultyMenuKey(9,true));assert(difficultySelected==1);difficultyMenuKey(17,true);difficultyFlush();assert(difficultyProgress.current(0)==0);
 std::vector<uint8_t> before(32056),after(before.size());assert(rd(g,before.data(),before.size()));
 std::vector<uint8_t> raster(FB_BYTES+32,0x35);difficultyPainter.paint(raster.data()+16);for(int i=0;i<16;++i)assert(raster[i]==0x35&&raster[raster.size()-1-i]==0x35);assert(rd(g,after.data(),after.size()));assert(before==after);
 if(const char* prefix=std::getenv("DESTINIA_DIFFICULTY_CAPTURE"))writeBmp(std::string(prefix)+std::to_string(preferences.uiScale)+"_"+std::to_string(SCREEN_W)+".bmp",raster.data()+16);
 const auto p=hmalloc(128);sh(p+8,108);wr32(p+0x20,0);assert(callFn(exportsMap.at("checkQuestMonster"),{p}));destinia::FileLock held;assert(held.acquire(fs::path(saveDir)/"progression-v1.lock"));difficultyFlush();assert(!difficultyKills.empty()&&!difficultyProgress.allowed(0,1));held.release();difficultyRetry=0;difficultyFlush();assert(difficultyProgress.allowed(0,1)&&!difficultyProgress.allowed(0,2));
 difficultyMenuKey(17,true);difficultyFlush();assert(difficultyProgress.current(0)==1);assert(!difficultyProgress.allowed(0,2));
 assert(callFn(exportsMap.at("checkQuestMonster"),{p}));difficultyFlush();assert(!difficultyProgress.allowed(0,2)); // Same corpse cannot unlock another tier.
 auto saved=destinia::loadDifficulty(fs::path(saveDir),destinia::progression::Mode::Classic);assert(saved.current(0)==1);
 difficultyRetry=1;difficultySelected=0;difficultyMenuKey(17,true);sh(g+0x4886,1);difficultyFlush();difficultyFlush();assert(difficultyProgress.current(0)==1&&difficultyProgress.current(1)==0);sh(g+0x4886,0);difficultyRetry=0;
 // Native font call site's label changes only when an original save exists.
 const auto menu=exportsMap.at("Menu"),text=hmalloc(64);sh(menu+0xbe,0);const char* name="Guerreiro";wr(text,name,10);
 uc_hook labelObserver;const auto fontAt=exportsMap.at("Font_drawStringChar")&~1u;assert(uc_hook_add(uc,&labelObserver,UC_HOOK_CODE,reinterpret_cast<void*>(observeClass),nullptr,fontAt,fontAt)==UC_ERR_OK);
 auto originalClassCall=[&](){observedClass.clear();u32 sp=STACK_BASE+STACK_SIZE-0x100;uc_reg_write(uc,UC_ARM_REG_SP,&sp);wr32(sp,208);wr32(sp+4,12);wr32(sp+8,2);wr32(sp+12,0x11);W(0,0);W(1,text);W(2,90);W(3,180);assert(uc_emu_start(uc,SO_BASE+0x21c77,SO_BASE+0x21c7a,5000000,0)==UC_ERR_OK);};
 originalClassCall();assert(observedClass==name);
 {std::ofstream out(fs::path(saveDir)/"save0.dat",std::ios::binary);out<<"SENTINEL";}
 originalClassCall();assert(observedClass=="Guerreiro - Dificil");
 sh(menu+0xbe,1);originalClassCall();assert(observedClass==name);
 difficultyOpen=false;const auto transform=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,preferences.uiScale);const auto hit=transform.rect((SCREEN_W-400)/2+100,(SCREEN_H-240)/2+180,1,1);assert(difficultyMenuTouch(hit.x,hit.y,0));assert(difficultyOpen);assert(difficultyMenuTouch(hit.x,hit.y,1));
 const auto close=transform.rect((SCREEN_W-400)/2+280,(SCREEN_H-240)/2+53,1,1);assert(difficultyMenuTouch(close.x,close.y,0));assert(!difficultyOpen);difficultyMenuTouch(close.x,close.y,1);assert(difficultyMenuKey(3,true));assert(!difficultyButton&&rd16(ui+0x54)==4);
 assert(!difficultyMenuKey(3,true)); // Original System rows remain available.
 // Real ARM file removal atomically resets extended state too.
 const auto path=hmalloc(64);const char* saveName="save0.dat";wr(path,saveName,10);
 assert(callFn(STUB_BASE+importId.at("remove")*4,{path}));assert(!fs::exists(fs::path(saveDir)/"save0.dat"));assert(difficultyProgress.current(0)==0&&!difficultyProgress.allowed(0,1));
 std::puts("DIFFICULTY_ARM_OK");return 0;
}
