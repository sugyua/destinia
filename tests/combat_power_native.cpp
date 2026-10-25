#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine
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
#undef uc_close
int main(int argc,char**argv){if(int status=originalMain(argc,argv))return status;const auto game=exportsMap.at("Game");assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));for(int i=0;i<6;i++)wr32(game+0x4584+4*i,0);uint16_t base[4]={20,15,10,10},level=20,none=65535;wr(game+0x48ba,base,8);wr(game+0x48fa,&level,2);wr(game+0x7574,&none,2);u32 first=0,second=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{1},&first));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{2},&second));auto candidate=game+0x2ed6+20*second;wr32(game+0x4584,game+0x2ed6+20*first);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));assert(callFn(exportsMap.at("setMyGold"),{20000}));std::vector<uint8_t> before(32056),after(before.size());rd(game,before.data(),before.size());auto preview=fcCompareEquipment(candidate,false);rd(game,after.data(),after.size());assert(before==after);u32 gold=0;assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);assert(preview.valid&&preview.current==488&&preview.projected==608&&preview.delta()==120);assert(preview.after[7]-preview.before[7]==30);
 wr32(game+0x4584,candidate);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));auto downgrade=fcCompareEquipment(game+0x2ed6+20*first,false);assert(downgrade.delta()==-120);wr32(game+0x4584,game+0x2ed6+20*first);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 uint16_t hp=1;wr(game+0x48f4,&hp,2);assert(fcCompareEquipment(candidate,false).current==488);uint16_t timer=30;wr(game+0x4972,&timer,2);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));auto buffed=fcCompareEquipment(candidate,false);assert(buffed.current==500&&buffed.projected==624);timer=0;wr(game+0x4972,&timer,2);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));assert(fcCompareEquipment(candidate,false).current==488);
 u32 other=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{33},&other));assert(!fcCompareEquipment(game+0x2ed6+20*other,false).valid);assert(!fcCompareEquipment(game-1,false).valid);
 assert(callFn(exportsMap.at("_backupHeroStat"),{}));uint16_t menu=16;wr(game+0x4854,&menu,2);const auto target=rd32(exportsMap.at("Grp")+4);fcBeginFrame();assert(callFn(exportsMap.at("_drawMenuStatus"),{target,u32(SCREEN_W/2-116),25,232}));assert(fcStatusSeen&&fcPanel.w==232);fcBeginFrame();assert(callFn(exportsMap.at("_drawEquipDesc"),{target,u32(SCREEN_W/2-109),157,candidate,0}));assert(fcCandidateCount&&fcCandidates[0]==candidate);

 const auto ui=exportsMap.at("GameUI");const uint16_t ready=16;wr(ui+0x76,&ready,2);wr32(ui+0x7c,0);
 const auto framePtr=rd32(rd32(exportsMap.at("frameBuffer")+4*target)+8);const int ox=(SCREEN_W-400)/2,oy=(SCREEN_H-240)/2;
 int fonts=0;uc_hook fontHook;const auto fontAt=exportsMap.at("Font_drawStringChar")&~1u;
 assert(uc_hook_add(uc,&fontHook,UC_HOOK_CODE,(void*)+[](uc_engine*,uint64_t,uint32_t,void* n){++*static_cast<int*>(n);},&fonts,fontAt,fontAt)==UC_ERR_OK);// Late font observer needs fresh translated blocks.
 assert(uc_ctl_remove_cache(uc,fontAt,fontAt+72)==UC_ERR_OK);
 fcBeginFrame();assert(callFn(exportsMap.at("_drawMenuStatus"),{target,u32(ox+84),u32(oy+25),232}));
 std::vector<uint8_t> status(FB_BYTES),painted;rd(framePtr,status.data(),status.size());painted=status;const int oldFonts=fonts;fcPaint(painted.data(),100);if(fonts<=oldFonts){const auto info=fcCompareEquipment(0);std::fprintf(stderr,"FC_STATUS x=%d y=%d seen=%d width=%d level=%u state=%u anim=%u ui=%x valid=%d reason=%s\n",fcPanel.x,fcPanel.y,int(fcStatusSeen),fcPanel.w,rd16(game+0x48fa),rd16(game+0x4854),rd16(ui+0x76),rd32(ui+0x7c),int(info.valid),info.reason.c_str());}assert(fonts>oldFonts);const auto freePixel=((oy+47)*SCREEN_W+ox+182)*2;assert(painted[freePixel]==status[freePixel]&&painted[freePixel+1]==status[freePixel+1]);
 fcBeginFrame();wr32(ui+0x7c,0x20000002);assert(callFn(exportsMap.at("_drawEquipDesc"),{target,u32(ox+91),u32(oy+157),candidate,0}));
 std::vector<uint8_t> item(FB_BYTES),itemPage;rd(framePtr,item.data(),item.size());itemPage=item;fcComparisonPage=false;fcPaint(itemPage.data(),100);
 for(int y=0;y<oy+225;y++)for(int x=0;x<SCREEN_W;x++){auto i=(y*SCREEN_W+x)*2;assert(itemPage[i]==item[i]&&itemPage[i+1]==item[i+1]);}
 assert(fcMenuKey(11,true)&&fcComparisonPage);std::vector<uint8_t> uiBefore(12088),uiAfter(12088);rd(ui,uiBefore.data(),uiBefore.size());rd(game,before.data(),before.size());
 std::array<uint8_t,124> grpBefore{},grpAfter{};rd(exportsMap.at("Grp"),grpBefore.data(),grpBefore.size());const auto oldSlot=rd32(exportsMap.at("frameBuffer")+16),oldFont=rd32(exportsMap.at("Font"));
 for(int scale:{75,100,125}){std::vector<uint8_t> guarded(FB_BYTES+32,0x5a);std::copy(item.begin(),item.end(),guarded.begin()+16);fcPaint(guarded.data()+16,scale);for(int i=0;i<16;i++)assert(guarded[i]==0x5a&&guarded[guarded.size()-1-i]==0x5a);const auto region=destinia::UiTransform{SCREEN_W,SCREEN_H,scale,destinia::UiAnchor::Center,232,240}.rect(ox+91,oy+157,218,82);for(int y=0;y<SCREEN_H;y++)for(int x=0;x<SCREEN_W;x++)if(x<region.x||x>=region.x+region.w||y<region.y||y>=region.y+region.h){auto i=(y*SCREEN_W+x)*2;assert(guarded[i+16]==item[i]&&guarded[i+17]==item[i+1]);}}
 rd(game,after.data(),after.size());rd(ui,uiAfter.data(),uiAfter.size());rd(exportsMap.at("Grp"),grpAfter.data(),grpAfter.size());assert(before==after&&uiBefore==uiAfter&&grpBefore==grpAfter&&oldFont==rd32(exportsMap.at("Font"))&&oldSlot==rd32(exportsMap.at("frameBuffer")+16));
 const auto pagePoint=destinia::UiTransform{SCREEN_W,SCREEN_H,100,destinia::UiAnchor::Center,232,240}.point(ox+110,oy+232);assert(fcMenuTouch(pagePoint.first,pagePoint.second,0)&&!fcComparisonPage);assert(fcMenuTouch(-1,-1,1)&&!fcTouchCaptured);assert(fcMenuKey(10015,true)&&fcComparisonPage);
 for(uint16_t job:{1,2}){wr(game+0x4886,&job,2);u32 a=0,b=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{u32((job<<5)|1)},&a));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{u32((job<<5)|2)},&b));wr32(game+0x4584,game+0x2ed6+20*a);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));rd(game,before.data(),before.size());auto cross=fcCompareEquipment(game+0x2ed6+20*b,false);assert(cross.valid&&cross.delta()>0);rd(game,after.data(),after.size());assert(before==after);}puts("FC_COMPARATOR_ARM_OK");return 0;}
