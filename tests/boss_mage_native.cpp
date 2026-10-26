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
static int nativeNpcCalls=0,saveCalls=0;
static void countNpc(uc_engine*,uint64_t,uint32_t,void*){++nativeNpcCalls;}
static void countSave(uc_engine*,uint64_t,uint32_t,void*){++saveCalls;}
int main(int argc,char** argv){
 setvbuf(stdout,nullptr,_IONBF,0);if(int rc=runHost(argc,argv))return rc;const auto g=exportsMap.at("Game");
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));assert(callFn(exportsMap.at("Map_loadData"),{0,65,1}));
 auto sh=[&](u32 p,int n){uint16_t value=uint16_t(n);assert(wr(p,&value,2));};
 wr32(rd32(SO_BASE+0x44240),exportsMap.at("GameCanvas"));sh(g+0x4854,0);sh(g+0x4866,0);auto site=bossMage.visibleSite();assert(site&&site->x==328&&site->y==344);
 sh(g+0x4890,site->x);sh(g+0x4892,site->y+16);sh(g+0x4898,2);sh(g+0x4878,site->x-SCREEN_W/2);sh(g+0x487a,site->y-SCREEN_H/2);
 std::array<uint8_t,20> noDrops{};assert(wr(g+0x3e76,noDrops.data(),noDrops.size()));
 uc_hook hn,hs;const auto npc=exportsMap.at("_checkNPCNearby")&~1u,save=exportsMap.at("GameCanvas_SaveGame")&~1u;
 assert(uc_hook_add(uc,&hn,UC_HOOK_CODE,reinterpret_cast<void*>(countNpc),nullptr,npc,npc)==UC_ERR_OK);assert(uc_hook_add(uc,&hs,UC_HOOK_CODE,reinterpret_cast<void*>(countSave),nullptr,save,save)==UC_ERR_OK);
 std::vector<uint8_t> before(32056),after(before.size());assert(rd(g,before.data(),before.size()));
 int opens=0;bossMage.menu.opener=[&](){++opens;u32 gold;assert(callFn(exportsMap.at("getMyGold"),{},&gold));};
 auto press=[&](int key){assert(callFn(exportsMap.at("GameCanvas_mKeyPressed"),{u32(key)}));};auto release=[&](int key){assert(callFn(exportsMap.at("GameCanvas_mKeyReleased"),{u32(key)}));};
 press(17);assert(opens==1&&!bossMage.menu.pending);press(17);assert(opens==1);release(17);press(6);release(6);assert(opens==2&&nativeNpcCalls==0&&saveCalls==0);
 assert(rd(g,after.data(),after.size()));assert(before==after);
 std::vector<uint8_t> raster(FB_BYTES+32,0x35);bossMage.paint(raster.data()+16);for(int i=0;i<16;++i)assert(raster[i]==0x35&&raster[raster.size()-i-1]==0x35);assert(std::any_of(raster.begin()+16,raster.end()-16,[](uint8_t c){return c!=0x35;}));assert(rd(g,after.data(),after.size()));assert(before==after);
 if(const char* capture=std::getenv("DESTINIA_MAGE_CAPTURE"))writeBmp(std::string(capture)+std::to_string(SCREEN_W)+".bmp",raster.data()+16);
 // Autoloot must never trigger the host-side dialogue event.
 lootQol.autoTarget=0;press(6);lootQol.autoTarget=-1;release(6);assert(opens==2);assert(wr(g,before.data(),before.size()));
 // Keep the original action when facing away or outside the radius.
 sh(g+0x4898,3);press(17);release(17);assert(opens==2);assert(wr(g,before.data(),before.size()));
 bossMage.focused=false;press(17);release(17);assert(opens==2);bossMage.focused=true;assert(wr(g,before.data(),before.size()));
 // A newly overlapping native NPC disables the synthetic site instead of moving hitboxes.
 sh(g+0x1fac,site->x);sh(g+0x1fae,site->y);assert(!bossMage.visibleSite());assert(wr(g,before.data(),before.size()));
 sh(g+0x4854,16);assert(!bossMage.visibleSite());assert(wr(g,before.data(),before.size()));
 // Map changes invalidate an unbound request; no delayed menu in another scene.
 bossMage.menu.opener=nullptr;press(17);release(17);assert(bossMage.menu.pending);assert(callFn(exportsMap.at("Map_loadData"),{0,62,1}));assert(!bossMage.visibleSite()&&!bossMage.menu.pending);
 assert(saveCalls==0);std::puts("BOSS_MAGE_ARM_OK");return 0;
}
