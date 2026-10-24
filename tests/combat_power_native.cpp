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
 auto snapshot=before;std::vector<uint8_t> guarded(size_t(SCREEN_W)*SCREEN_H*2+32,0x5a);for(int scale:{75,100,125}){auto pixels=guarded.data()+16;destinia::paintCombatPower(pixels,SCREEN_W,SCREEN_H,scale,{SCREEN_W/2-116,SCREEN_H/2-90,232},preview,true);for(int i=0;i<16;i++)assert(guarded[i]==0x5a&&guarded[guarded.size()-1-i]==0x5a);}for(uint16_t job:{1,2}){wr(game+0x4886,&job,2);u32 a=0,b=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{u32((job<<5)|1)},&a));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{u32((job<<5)|2)},&b));wr32(game+0x4584,game+0x2ed6+20*a);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));rd(game,before.data(),before.size());auto cross=fcCompareEquipment(game+0x2ed6+20*b,false);assert(cross.valid&&cross.delta()>0);rd(game,after.data(),after.size());assert(before==after);}puts("FC_COMPARATOR_ARM_OK");return 0;}
