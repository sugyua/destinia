#include <unicorn/unicorn.h>
#include <cassert>
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
int main(int argc,char**argv){
 setvbuf(stdout,nullptr,_IONBF,0);if(int rc=runHost(argc,argv))return rc;auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));auto sh=[&](u32 at,uint16_t v){wr(at,&v,2);};for(int n=0;n<6;n++)wr32(g+0x4584+4*n,0);
 uint16_t stats[4]={20,15,10,10};wr(g+0x48ba,stats,8);sh(g+0x48fa,20);sh(g+0x7574,65535);u32 weak=0,better=0,wrong=0;
 assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{1},&weak));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{2},&better));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{33},&wrong));wr32(g+0x4584,g+0x2ed6+20*weak);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 for(int b=0;b<4;b++){uint16_t h[3]={4,0,uint16_t(b?0:3)};wr(g+0x4338+b*0x92,h,6);}for(int slot=0;slot<3;slot++){uint16_t rec[3]={0,uint16_t(slot==0?weak:slot==1?better:wrong),1};wr(g+0x433e + 6*slot,rec,6);}
 assert(callFn(exportsMap.at("setMyGold"),{20000}));assert(callFn(exportsMap.at("openMainmenu"),{0}));sh(ui+0x76,16);wr32(rd32(SO_BASE+0x44240),exportsMap.at("GameCanvas"));for(int key:{7,7,9,6,9})assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));assert(rd32(ui+0x7c)==0x20000002u);
 inventoryBeginFrame();assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));inventoryRefreshUpgrades();assert(inventoryIsUpgrade(g+0x2ed6+20*better));assert(!inventoryIsUpgrade(g+0x2ed6+20*weak));assert(!inventoryIsUpgrade(g+0x2ed6+20*wrong));
 auto icon=std::find_if(inventoryIcons.begin(),inventoryIcons.end(),[&](auto i){return i.pointer==g+0x2ed6+20*better;});assert(icon!=inventoryIcons.end());
 u32 nativePixels=0;assert(callFn(exportsMap.at("MC_GRP_GET_FRAME_BUFFER_POINTER"),{rd32(exportsMap.at("Grp")+4)},&nativePixels));std::vector<uint8_t> preview(FB_BYTES);rd(nativePixels,preview.data(),preview.size());inventoryPaint(preview.data());writeBmp((fs::path(saveDir).parent_path()/"inventory-upgrade.bmp").string(),preview.data());
 std::vector<uint8_t> before(32056),after(before.size());rd(g,before.data(),before.size());
 for(int scale:{75,100,125}){preferences.uiScale=scale;std::vector<uint8_t> pixels(FB_BYTES+32,0x34);inventoryPaint(pixels.data()+16);const destinia::UiTransform t{SCREEN_W,SCREEN_H,scale,destinia::UiAnchor::Center,232,240};auto pos=t.point(icon->x+20,icon->y+3);auto at=16+2*(pos.second*SCREEN_W+pos.first);assert(pixels[at]==0xe0&&pixels[at+1]==7);for(int i=0;i<16;i++)assert(pixels[i]==0x34&&pixels[pixels.size()-1-i]==0x34);rd(g,after.data(),after.size());assert(before==after);}
 // The native cursor wins over a stale filter selection; no overlay is opened.
 sh(ui+0x56,1);inventoryPanel.query.rank=4;assert(inventoryMenuKey(10014,true));assert(!inventoryOpen&&rd16(ui+0x56)==1);std::array<uint8_t,20> bytes{};rd(g+0x2ed6+20*better,bytes.data(),20);assert(inventoryProtection.has(better,bytes));assert(inventoryMenuKey(10014,false));assert(inventoryProtection.has(better,bytes));rd(g,after.data(),after.size());assert(before==after);
 assert(inventoryMenuKey(10014,true));assert(!inventoryOpen&&!inventoryProtection.has(better,bytes));assert(inventoryMenuKey(10016,true)&&inventoryOpen);inventoryPanel.reset();auto item=std::find_if(inventoryPanel.items.begin(),inventoryPanel.items.end(),[&](auto i){return i.pool==int(better);});assert(item!=inventoryPanel.items.end()&&item->upgrade);assert(inventoryMenuKey(10016,true)&&!inventoryOpen);
 // Equipping the better item invalidates cached green marks.
 wr32(ui+0x2148,g+0x2ed6+20*better);inventoryRefreshUpgrades();assert(!inventoryIsUpgrade(g+0x2ed6+20*better));puts("INVENTORY_UPGRADE_HOTKEY_ARM_OK");
}
