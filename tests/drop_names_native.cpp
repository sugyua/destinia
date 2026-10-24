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
int main(int argc,char** argv){
 if(int result=originalMain(argc,argv))return result;
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 const auto game=exportsMap.at("Game");std::vector<uint8_t> before(0x4900),after(before.size());
 uint8_t flags[20]{};flags[0]=1;wr(game+0x3e76,flags,20);uint16_t coordinates[2]={160,100},hero[2]={100,100},camera[2]={0,0},kind[2]={1,0};
 wr(game+0x3eb8,coordinates,4);wr(game+0x3ec0,kind,4);wr(game+0x4890,hero,4);wr(game+0x4878,camera,4);const char name[32]="TEST DROP";wr(game+0x3e90,name,sizeof name);
 wr32(gMMain+4,rd32(exportsMap.at("Grp")+4));
 u32 pixels=0;assert(callFn(exportsMap.at("MC_GRP_GET_FRAME_BUFFER_POINTER"),{rd32(gMMain+4)},&pixels));
 
 std::vector<uint8_t> background(FB_BYTES,0x34),expected(FB_BYTES),actual(FB_BYTES);
 dropNamesRenderer.enabled=false;wr(pixels,background.data(),FB_BYTES);assert(callFn(exportsMap.at("drawDropItemName"),{}));rd(pixels,actual.data(),FB_BYTES);assert(actual==background);
 // Reference: native renderer with the hero inside the original pickup radius.
 wr(game+0x4890,coordinates,4);u32 check=0;assert(callFn(exportsMap.at("_checkItemNearby"),{0},&check));assert(check==1);wr(pixels,background.data(),FB_BYTES);assert(callFn(exportsMap.at("drawDropItemName"),{}));rd(pixels,expected.data(),FB_BYTES);assert(expected!=background);
 wr(game+0x4890,hero,4);rd(game,before.data(),before.size());dropNamesRenderer.enabled=true;wr(pixels,background.data(),FB_BYTES);assert(callFn(exportsMap.at("drawDropItemName"),{}));assert(dropNamesRenderer.error.empty());rd(pixels,actual.data(),FB_BYTES);assert(actual==expected);rd(game,after.data(),after.size());assert(before==after);
 // A caller outside drawing must retain the real collection-distance result.
 u32 nearbyResult=1;assert(callFn(exportsMap.at("_checkItemNearby"),{0},&nearbyResult));assert(nearbyResult==0);
 for(bool inactive:{true,false}){if(inactive){flags[0]=0;wr(game+0x3e76,flags,20);}else{flags[0]=1;wr(game+0x3e76,flags,20);coordinates[0]=uint16_t(SCREEN_W+40);wr(game+0x3eb8,coordinates,4);}wr(pixels,background.data(),FB_BYTES);assert(callFn(exportsMap.at("drawDropItemName"),{}));rd(pixels,actual.data(),FB_BYTES);assert(actual==background);}
 puts("DROP_NAMES_ARM_OK");return 0;
}
