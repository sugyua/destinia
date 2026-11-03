#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepTitleEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepTitleEngine
#define main originalTitleMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static u32 observedPages=0;
static void observePages(uc_engine*,uint64_t,uint32_t,void*){observedPages=rd32(regSP());}
int main(int argc,char**argv){
 if(int r=originalTitleMain(argc,argv))return r;
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 const u32 g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 auto table=exportsMap.at("frameBuffer")+16,desc=hmalloc(12),pixels=hmalloc(FB_BYTES);wr32(desc,SCREEN_W);wr32(desc+4,SCREEN_H);wr32(desc+8,pixels);wr32(table,desc);
 uc_hook hook{};const auto at=SO_BASE+0x2ca06;if(uc_hook_add(uc,&hook,UC_HOOK_CODE,(void*)observePages,nullptr,at,at)!=UC_ERR_OK)return 8;
 wr16(ui+0x56,0);if(!callFn(exportsMap.at("_drawNickSel"),{4,84,25,232})||observedPages!=9){fprintf(stderr,"PAGE_COUNT got=%u expected=9\n",observedPages);return 9;}
 wr16(g+0x4854,16);wr16(g+0x4886,0);wr16(ui+0x76,16);wr32(ui+0x7c,2);wr16(ui+0x56,48);
 destinia::TitlesStore external;external.load(fs::path(saveDir));external.unlock(0,destinia::TitleId::SenhorDoAbismo);if(!external.save(fs::path(saveDir)))return 16;
 if(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::SenhorDoAbismo))return 17;
 codex.select(0);uint64_t instance=1000;for(const auto& entry:codex.names)codex.observe(++instance,entry.first,false);
 if(!destinia::isCodexComplete(codex,0))return 19;
 // Exercise original ARM death callback for ten distinct Fungus instances.
 for(int i=0;i<10;++i){
  const u32 monster=DATA_BASE+0x4000+u32(i)*136;std::vector<uint8_t> blank(136);if(!wr(monster,blank.data(),blank.size()))return 33;
  wr16(monster+8,1);wr16(monster+0x12,1);wr32(monster+0x20,0);
  if(!callFn(exportsMap.at("checkQuestMonster"),{monster}))return 34;
 }
 if(codex.entries[0][1].kills!=10)return 35;
 // A write failure must not grant a title based only on unsaved progress.
 codexWritable=false;titleMenu.slot=0;titleEnterExtra();
 if(bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::Complecionista)||!codex.dirty)return 22;
 codexWritable=true;
 destinia::FileLock held;if(!held.acquire(fs::path(saveDir)/"codex-v1.lock"))return 24;
 titleEnterExtra();
 if(!codex.dirty||!codexWritable||bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::Complecionista))return 25;
 if(codexFinalize())return 26;
 int recoveries=0;for(const auto& item:fs::directory_iterator(fs::path(saveDir))){
  if(item.path().filename().string().find("codex-recovery-")!=0)continue;
  std::ifstream in(item.path(),std::ios::binary);const std::string data((std::istreambuf_iterator<char>(in)),{});
  if(data!=codex.encode())return 27;++recoveries;
 }
 if(recoveries!=1||!codex.dirty)return 28;
 held.release();
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||!titleMenu.open||titleMenu.selected!=0)return 2;
 if(codex.dirty||!fs::is_regular_file(fs::path(saveDir)/"codex-v1.txt"))return 23;
 if(!bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::SenhorDoAbismo))return 18;
 if(!bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::Complecionista))return 20;
 if(!bossRushUI.titlesStore.isUnlocked(0,destinia::TitleId::CacadorDeFungos))return 29;
 destinia::TitlesStore confirmed;if(!confirmed.load(fs::path(saveDir))||!confirmed.isUnlocked(0,destinia::TitleId::Complecionista))return 21;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||titleMenu.selected!=8)return 30;
 if(titleMenu.selectedTitle()!=destinia::TitleId::CacadorDeFungos)return 31;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{5})||titleMenu.selected!=0)return 32;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{9})||titleMenu.selected!=1)return 3;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{5})||titleMenu.open||rd16(ui+0x56)!=49||rd32(ui+0x7c)!=2)return 4;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||!titleMenu.open)return 5;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{27})||titleMenu.open||rd32(ui+0x7c)!=1)return 6;
 wr32(ui+0x7c,2);wr16(ui+0x56,0);
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||titleMenu.open||rd16(ui+0x56)!=8)return 7;
 wr16(ui+0x56,49);auto tr=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,100);
 auto next=destinia::titleExtraNextOriginal(SCREEN_W,SCREEN_H);auto p=tr.point(next.x+2,next.y+2);
 if(!titleMenuTouch(p.first,p.second,0,SCREEN_W,SCREEN_H,100)||!titleMenu.open)return 10;
 if(!titleMenuTouch(p.first,p.second,1,SCREEN_W,SCREEN_H,100))return 11;
 auto row=destinia::titleExtraRow(1,SCREEN_W,SCREEN_H);p=tr.point(row.x+2,row.y+2);
 if(!titleMenuTouch(p.first,p.second,0,SCREEN_W,SCREEN_H,100)||titleMenu.selected!=1)return 12;
 if(!titleMenuTouch(p.first,p.second,1,SCREEN_W,SCREEN_H,100))return 13;
 auto prev=destinia::titleExtraPrevious(SCREEN_W,SCREEN_H);p=tr.point(prev.x+2,prev.y+2);
 if(!titleMenuTouch(p.first,p.second,0,SCREEN_W,SCREEN_H,100)||titleMenu.open||rd16(ui+0x56)!=49)return 14;
 if(!titleMenuTouch(p.first,p.second,1,SCREEN_W,SCREEN_H,100))return 15;
 // Navigate through the real menu to equip the newly unlocked title.
 wr16(g+0x4854,16);wr16(g+0x4886,0);wr16(ui+0x76,16);wr32(ui+0x7c,2);wr16(ui+0x56,49);
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||!titleMenu.open)return 36;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||titleMenu.selectedTitle()!=destinia::TitleId::CacadorDeFungos)return 37;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{17})||titleMenu.open)return 38;
 destinia::TitlesStore equipped;if(!equipped.load(fs::path(saveDir))||equipped.getEquipped(0)!=destinia::TitleId::CacadorDeFungos||!equipped.isExclusive(0))return 39;
 puts("TITLE_MENU_NAVIGATION_ARM_OK");return 0;
}
