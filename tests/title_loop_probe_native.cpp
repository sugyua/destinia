#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepLoopEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepLoopEngine
#define main originalLoopMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static int nativeTitlePaints=0,redirectedTitlePaints=0;
static bool abortTitleNow=false;
static void countTitlePaint(uc_engine* engine,uint64_t,uint32_t,void*){++nativeTitlePaints;if(titleDrawRedirected)++redirectedTitlePaints;if(abortTitleNow)uc_emu_stop(engine);}
int main(int argc,char**argv){
 if(int r=originalLoopMain(argc,argv))return r;
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 if(!callFn(exportsMap.at("Map_loadData"),{0,1,1}))return 5;
 wr32(exportsMap.at("MMain"),exportsMap.at("GameCanvas"));
 wr16(g+0x4854,16);wr16(g+0x4886,0);wr16(ui+0x76,16);wr32(ui+0x7c,2);wr16(ui+0x56,49);
 uc_hook observer{};const auto nick=exportsMap.at("_drawNickSel")&~1u;
 if(uc_hook_add(uc,&observer,UC_HOOK_CODE,(void*)countTitlePaint,nullptr,nick,nick)!=UC_ERR_OK)return 6;
 if(!titleMenu.enter(16,2,49,0,7))return 3;titleEnterExtra();
 const u32 screen=nextArr++;arrays[screen].assign(FB_BYTES,0);u32 wait=0;
 {TitleNativeFrameScope scope;if(!callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniRun"),{ENV_OBJ,0x10,screen,1},&wait))return 2;}
 if(!nativeTitlePaints||!redirectedTitlePaints){std::fprintf(stderr,"ORIGINAL_TITLE_STATE_NOT_UPDATED\n");return 7;}
 if(titleDrawRedirected||!titleDrawSlot||rd32(titleDrawSlot)!=titleDrawPrevious){std::fprintf(stderr,"FRAMEBUFFER_NOT_RESTORED\n");return 15;}
 auto& frame=arrays[screen];nativeCodexPainter.paint(frame.data(),true);if(!destinia::paintTitleExtra(frame.data(),SCREEN_W,SCREEN_H,preferences.uiScale,titleMenu,bossRushUI.titlesStore,titleMenuMessage))return 4;
 if(const char* capture=std::getenv("DESTINIA_TITLE_CAPTURE"))writeBmp(capture,frame.data());
 for(u32 tick=2;tick<6;++tick){
  nativeTitlePaints=redirectedTitlePaints=0;
  {TitleNativeFrameScope scope;if(!callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniRun"),{ENV_OBJ,0x10,screen,tick},&wait))return 19;}
  if(!titleMenu.open||!nativeTitlePaints||!redirectedTitlePaints||titleDrawRedirected||rd32(titleDrawSlot)!=titleDrawPrevious)return 20;
 }
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{9})||titleMenu.selected!=1)return 21;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{3})||titleMenu.selected!=0)return 22;
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7})||titleMenu.selected!=8)return 23;
 if(!destinia::paintTitleExtra(frame.data(),SCREEN_W,SCREEN_H,preferences.uiScale,titleMenu,bossRushUI.titlesStore,titleMenuMessage))return 24;
 if(titleMenu.selectedTitle()!=destinia::TitleId::CacadorDeFungos)return 25;
 if(const char* capture=std::getenv("DESTINIA_TITLE_PAGE9_CAPTURE"))writeBmp(capture,frame.data());
 if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{5})||titleMenu.selected!=0||!titleMenu.open)return 26;
 const auto tr=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,preferences.uiScale);
 const auto row=destinia::titleExtraRow(1,SCREEN_W,SCREEN_H);auto point=tr.point(row.x+2,row.y+2);
 if(!titleMenuTouch(point.first,point.second,0,SCREEN_W,SCREEN_H,preferences.uiScale)||titleMenu.selected!=1)return 10;
 if(!titleMenuTouch(point.first,point.second,1,SCREEN_W,SCREEN_H,preferences.uiScale))return 11;
 const auto back=destinia::titleExtraPrevious(SCREEN_W,SCREEN_H);point=tr.point(back.x+2,back.y+2);
 if(!titleMenuTouch(point.first,point.second,0,SCREEN_W,SCREEN_H,preferences.uiScale)||titleMenu.open||rd16(ui+0x56)!=49)return 12;
 if(!titleMenuTouch(point.first,point.second,1,SCREEN_W,SCREEN_H,preferences.uiScale))return 13;
 wr16(g+0x4854,16);wr32(ui+0x7c,2);nativeTitlePaints=0;
 {TitleNativeFrameScope scope;if(!callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniRun"),{ENV_OBJ,0x10,screen,6},&wait))return 8;}
 if(!nativeTitlePaints){std::fprintf(stderr,"ORIGINAL_TITLE_PAGE_MISSING\n");return 9;}
 if(std::getenv("DESTINIA_TITLE_DRAW_AUDIT")){
  std::vector<uint8_t> beforeGame(32056),afterGame(32056),beforeUi(12088),afterUi(12088);
  rd(g,beforeGame.data(),beforeGame.size());rd(ui,beforeUi.data(),beforeUi.size());
  if(!callFn(exportsMap.at("GameUI_drawMainMenu"),{0}))return 14;
  rd(g,afterGame.data(),afterGame.size());rd(ui,afterUi.data(),afterUi.size());
  for(size_t i=0;i<beforeGame.size();++i)if(beforeGame[i]!=afterGame[i])std::printf("PAINT_GAME_DIFF %zx %u %u\n",i,beforeGame[i],afterGame[i]);
  for(size_t i=0;i<beforeUi.size();++i)if(beforeUi[i]!=afterUi[i])std::printf("PAINT_UI_DIFF %zx %u %u\n",i,beforeUi[i],afterUi[i]);
 }
 if(std::getenv("DESTINIA_TITLE_TEST_ABORT")){
  abortTitleNow=true;
  wr16(g+0x4854,16);wr32(ui+0x7c,2);wr16(ui+0x56,49);if(!titleMenu.enter(16,2,49,0,7))return 17;
  bool succeeded=false;{TitleNativeFrameScope scope;succeeded=callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniRun"),{ENV_OBJ,0x10,screen,7},&wait);}
  if(succeeded||titleDrawRedirected||rd32(titleDrawSlot)!=titleDrawPrevious)return 18;
 }
 std::printf("TITLE_LOOP_PROBE_OK wait=%u draws=%llu\n",wait,(unsigned long long)uiRenderer.draws);return 0;
}
