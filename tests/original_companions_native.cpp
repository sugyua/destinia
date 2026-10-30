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
#include "../common/original_companions_host.h"
#undef assert
#define assert(expr) do{if(!(expr)){fprintf(stderr,"Assertion line %d: %s\n",__LINE__,#expr);exit(1);}}while(false)
int main(int argc,char**argv){
 if(int code=originalMain(argc,argv))return code;
 OriginalCompanionsHost companions;companions.install();
 const auto g=exportsMap.at("Game");std::vector<uint8_t> before(32056),after(32056);rd(g,before.data(),before.size());
 std::set<std::vector<uint8_t>> distinct;bool advanced=false;
 for(int id=0;id<4;id++){
  std::vector<uint8_t> output(FB_BYTES);assert(companions.paint(output.data(),SCREEN_W,SCREEN_H,id,100,100,0,5));
  assert(std::any_of(output.begin(),output.end(),[](uint8_t c){return c;}));distinct.insert(output);rd(g,after.data(),after.size());assert(before==after);
  std::vector<uint8_t> held(FB_BYTES);assert(companions.paint(held.data(),SCREEN_W,SCREEN_H,id,100,100,5,5));assert(held==output);
  for(uint64_t tick=6;tick<96;tick+=6){std::vector<uint8_t> animated(FB_BYTES);assert(companions.paint(animated.data(),SCREEN_W,SCREEN_H,id,100,100,tick,5));if(animated!=output)advanced=true;}
  rd(g,after.data(),after.size());assert(before==after);
  const auto captures=originalUiHost.draws;assert(companions.paint(output.data(),SCREEN_W,SCREEN_H,id,100,100,0,5));assert(originalUiHost.draws==captures);
  std::vector<uint8_t> guarded(FB_BYTES+32,0xa5);assert(companions.paint(guarded.data()+16,SCREEN_W,SCREEN_H,id,1,1,20,5));for(int n=0;n<16;n++)assert(guarded[n]==0xa5&&guarded[FB_BYTES+16+n]==0xa5);
 }
 assert(distinct.size()==4);assert(advanced);
 const auto oldAssets=assetsDir;assetsDir="missing-original-companion-assets";OriginalCompanionsHost missing;std::vector<uint8_t> empty(FB_BYTES);assert(missing.paint(empty.data(),SCREEN_W,SCREEN_H,0,100,100,0,1));assetsDir=oldAssets;puts("ORIGINAL_COMPANIONS_ARM_OK");return 0;
}
