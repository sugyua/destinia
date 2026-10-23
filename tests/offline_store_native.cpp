#include <cassert>
#include <unicorn/unicorn.h>
static uc_err keepUc(uc_engine*){return UC_ERR_OK;}
#define uc_close keepUc
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
int main(int argc,char**argv){
 if(int result=originalMain(argc,argv))return result;
 for(unsigned callback:{0,1,2})for(u32 product:{0u,1u,2u,3u,4u,5u,999u}){
  assert(callFn(fnSetCash,{12345}));
  assert(callFn(exportsMap.at("Comm_SetProcessCB"),{callback==1?exportsMap.at("Menu_netProc"):callback==2?0x1234:0}));
  assert(callFn(exportsMap.at("Comm_buyCash"),{product}));
  assert(pendingBuyCash);assert(rd16(exportsMap.at("MMain")+14624)==1);
  assert(finishOfflinePurchase());assert(!pendingBuyCash);
  u32 cash=0;assert(callFn(fnGetCash,{},&cash));assert(cash==12345);
  assert(rd16(exportsMap.at("MMain")+14624)==0);
 }
 assert(callFn(exportsMap.at("MMain_SaveData"),{}));
 assert(fs::is_regular_file(fs::path(saveDir)/"msave.dat"));
 assert(callFn(fnSetCash,{1}));
 assert(callFn(exportsMap.at("MMain_LoadData"),{}));
 u32 restored=0;assert(callFn(fnGetCash,{},&restored));assert(restored==12345);
 std::puts("OFFLINE_STORE_ARM_OK");uc_close(uc);uc=nullptr;
}
