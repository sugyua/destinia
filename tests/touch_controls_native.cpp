// Real APK code; synthetic canvas states, temporary saves. No campaign claim.
#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEmulator(uc_engine*) {return UC_ERR_OK;}
#define uc_close keepEmulator
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static int imageCalls=0,physicalPressed=0,physicalReleased=0;
static void countPhysical(uc_engine*,uint64_t,uint32_t,void* data) {
  if(R(0)==2) ++*static_cast<int*>(data);
}
static void countImage(uc_engine*,uint64_t,uint32_t,void*) {++imageCalls;}
int main(int argc,char**argv){
 if(int result=originalMain(argc,argv)) return result;
 const bool hidden=std::getenv("DESTINIA_EXPECT_HIDDEN")!=nullptr;
 const u32 main=exportsMap.at("MMain"),game=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 const u32 runner=rd32(SO_BASE+0x44240);
 wr32(runner,exportsMap.at("GameCanvas"));
 uc_hook hook1,hook2;
 auto count=[&](uc_hook*hook,const char*symbol){const u32 pc=exportsMap.at(symbol)&~1u;return uc_hook_add(uc,hook,UC_HOOK_CODE,(void*)countImage,nullptr,pc,pc)==UC_ERR_OK;};
 assert(count(&hook1,"Grp_drawWPNImage") && count(&hook2,"Grp_drawWPNImage2"));
 const u32 point=DATA_BASE+0x3000;
 // Set every original touch rectangle to an isolated location, independent of
 // whether the title screen has already prepared the gameplay button layout.
 for(int key=0;key<14;++key){int16_t rectangle[]={int16_t(10+key*20),10,16,16};wr(main+0xea+8*key,rectangle,sizeof rectangle);}
 for(int gameState:{0,-1,16,17}){
  uint16_t state=static_cast<uint16_t>(gameState==17?16:gameState);wr(game+0x4854,&state,2);wr32(ui+0x7c,gameState==17?0x20000001:0);
  u32 touchState=0;assert(callFn(exportsMap.at("_getTouchState"),{},&touchState));
  assert(touchState==(gameState==0?2u:gameState==-1?3u:gameState==17?5u:4u));
  bool gameplay=gameState==0 || gameState==-1;
  // key 5 (index 4) belongs to both gameplay and menu touch routing.
  wr32(point,gameState==17?254:94);wr32(point+4,14);u32 key=0;
  assert(callFn(exportsMap.at("getKeyPadIdx"),{point},&key));
  if(hidden && gameplay) assert(key==0); else assert(key==(gameState==17?13u:5u));
  imageCalls=0;
  assert(callFn(exportsMap.at("MMain_drawTouchButton"),{0}));
  if(hidden && gameplay) assert(imageCalls==0); else assert(imageCalls>0);
  // Keep the original state-write epilogue, even while drawing is suppressed.
  assert(rd16(main+0xd0)==touchState);
 }
 // Physical input bypasses touch hit testing; observe entry into the original
 // GameCanvas callbacks, not a replacement input implementation.
 uc_hook pressedHook,releasedHook;
 auto physical=[&](uc_hook* hook,const char* symbol,int* count){u32 pc=exportsMap.at(symbol)&~1u;return uc_hook_add(uc,hook,UC_HOOK_CODE,(void*)countPhysical,count,pc,pc)==UC_ERR_OK;};
 assert(physical(&pressedHook,"GameCanvas_mKeyPressed",&physicalPressed));
 assert(physical(&releasedHook,"GameCanvas_mKeyReleased",&physicalReleased));
 uint16_t active=0;wr(game+0x4854,&active,2);
 wr32(ui+0x7c,0);
 assert(callFn(exportsMap.at("MMain_keyPressed"),{2}));
 assert(callFn(exportsMap.at("MMain_keyReleased"),{2}));
 assert(physicalPressed==1 && physicalReleased==1);
 printf("TOUCH_CONTROLS_ARM_OK hidden=%d\n",hidden);
 return 0;
}
