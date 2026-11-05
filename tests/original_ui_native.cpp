#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
#include "../common/original_ui_host.h"
#undef assert
#define assert(expr) do { if(!(expr)){fprintf(stderr,"Assertion failed at line %d: %s\n",__LINE__,#expr);exit(1);} } while(false)
int main(int argc,char**argv){
 if(int result=originalMain(argc,argv))return result;
 originalUiHost.install();
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 const auto game=exportsMap.at("Game"),grp=exportsMap.at("Grp"),table=exportsMap.at("frameBuffer")+16;
 std::vector<uint8_t> before(32056);rd(game,before.data(),before.size());
 const auto slot=rd32(table),desc=hmalloc(12),pixels=hmalloc(FB_BYTES);wr32(desc,SCREEN_W);wr32(desc+4,SCREEN_H);wr32(desc+8,pixels);
 std::vector<uint8_t> blank(FB_BYTES),direct(FB_BYTES),actual(FB_BYTES);wr(pixels,blank.data(),blank.size());wr32(table,desc);
 const bool oldPass=uiRenderer.originalPass;uiRenderer.originalPass=true;
 assert(callFn(exportsMap.at("_drawDescBox"),{4,0,0,120,60}));rd(pixels,direct.data(),direct.size());uiRenderer.originalPass=oldPass;wr32(table,slot);
 assert(destinia::paintOriginalPanel(actual.data(),SCREEN_W,SCREEN_H,0,0,120,60));
 for(int y=0;y<60;y++)for(int x=0;x<120;x++)for(int c=0;c<2;c++)assert(actual[(y*SCREEN_W+x)*2+c]==direct[(y*SCREEN_W+x)*2+c]);
 auto draws=originalUiHost.draws;destinia::paintOriginalPanel(actual.data(),SCREEN_W,SCREEN_H,-4,-3,120,60);assert(originalUiHost.draws==draws);
 for(int y=0;y<57;y++)for(int x=0;x<116;x++)for(int c=0;c<2;c++)assert(actual[(y*SCREEN_W+x)*2+c]==direct[((y+3)*SCREEN_W+x+4)*2+c]);
 assert(destinia::paintOriginalCursor(actual.data(),SCREEN_W,SCREEN_H,200,100));
 assert(destinia::paintOriginalText(actual.data(),SCREEN_W,SCREEN_H,12,72,"ORIGINAL FONT",0xffc0,180,1));
 // Independent original-menu oracle: compare four untouched bevel corners.
 const auto ui=exportsMap.at("GameUI"),main=exportsMap.at("MMain");
 std::vector<uint8_t> uiBefore(12088),mainBefore(0x3922),menu(FB_BYTES);rd(ui,uiBefore.data(),uiBefore.size());rd(main,mainBefore.data(),mainBefore.size());
 wr32(table,desc);wr(pixels,blank.data(),blank.size());wr32(ui+0x7c,0);uint16_t anim=16,hidden=2;wr(ui+0x76,&anim,2);wr(main+0x3920,&hidden,2);uiRenderer.originalPass=true;
 assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{4}));rd(pixels,menu.data(),menu.size());wr(ui,uiBefore.data(),uiBefore.size());wr(main,mainBefore.data(),mainBefore.size());wr32(table,slot);uiRenderer.originalPass=oldPass;
 std::fill(actual.begin(),actual.end(),0);assert(destinia::paintOriginalPanel(actual.data(),SCREEN_W,SCREEN_H,0,0,180,100));
 const int ox=(SCREEN_W-400)/2,oy=(SCREEN_H-240)/2;
 const int targetCorners[4][2]={{0,0},{176,0},{0,96},{176,96}},sourceCorners[4][2]={{84,25},{312,25},{84,236},{312,236}};
 for(int n=0;n<4;n++)for(int y=0;y<4;y++)for(int x=0;x<4;x++)for(int c=0;c<2;c++)assert(actual[((targetCorners[n][1]+y)*SCREEN_W+targetCorners[n][0]+x)*2+c]==menu[((oy+sourceCorners[n][1]+y)*SCREEN_W+ox+sourceCorners[n][0]+x)*2+c]);
 for(int c=0;c<2;c++)assert(actual[(50*SCREEN_W+90)*2+c]==menu[((oy+132)*SCREEN_W+ox+306)*2+c]);
 std::vector<uint8_t> uiAfter(12088),mainAfter(0x3922);rd(ui,uiAfter.data(),uiAfter.size());rd(main,mainAfter.data(),mainAfter.size());assert(uiBefore==uiAfter);assert(mainBefore==mainAfter);
 std::vector<uint8_t> after(32056);rd(game,after.data(),after.size());assert(before==after);
 std::array<uint8_t,124> graphics{};rd(grp,graphics.data(),graphics.size());u32 regBefore=R(7);const auto oldSlot=rd32(table);
 const auto stackWord=rd32(STACK_BASE+128);
 bool failed=false;try{originalUiHost.capture(16,16,[&](){wr32(STACK_BASE+128,777);uint16_t badMode=999;wr(exportsMap.at("MMain")+0x3920,&badMode,2);wr32(exportsMap.at("GameUI"),888);wr32(game,123);wr32(grp,456);W(7,999);throw std::runtime_error("injected");},false);}catch(...){failed=true;}assert(failed);
 rd(game,after.data(),after.size());assert(before==after);std::array<uint8_t,124> graphicsAfter{};rd(grp,graphicsAfter.data(),graphicsAfter.size());assert(graphics==graphicsAfter);assert(R(7)==regBefore);assert(rd32(STACK_BASE+128)==stackWord);assert(rd32(table)==oldSlot);rd(ui,uiAfter.data(),uiAfter.size());rd(main,mainAfter.data(),mainAfter.size());assert(uiBefore==uiAfter);assert(mainBefore==mainAfter);
 assert(std::any_of(actual.begin(),actual.end(),[](uint8_t b){return b!=0;}));
 OriginalUiHost unavailable;u32 savedPc=regPC(),badPc=SO_BASE;uc_reg_write(uc,UC_ARM_REG_PC,&badPc);
 bool declined=false;try{declined=!unavailable.panel(actual.data(),SCREEN_W,SCREEN_H,0,0,127,57)&&!unavailable.text(actual.data(),SCREEN_W,SCREEN_H,0,0,"UNCACHED UNAVAILABLE",0xffff,123,1)&&!unavailable.cursor(actual.data(),SCREEN_W,SCREEN_H,20,20);}catch(...){declined=false;}uc_reg_write(uc,UC_ARM_REG_PC,&savedPc);assert(declined);
 puts("ORIGINAL_UI_ARM_OK");return 0;
}
