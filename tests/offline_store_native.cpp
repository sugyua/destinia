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
 for(bool callback:{false,true})for(u32 product:{0u,1u,2u,3u,4u,5u,999u}){
  assert(callFn(fnSetCash,{12345}));
  assert(callFn(exportsMap.at("Comm_SetProcessCB"),{callback?exportsMap.at("Menu_netProc"):0}));
  assert(callFn(exportsMap.at("Comm_buyCash"),{product}));
  assert(pendingBuyCash);assert(rd16(exportsMap.at("MMain")+14624)==1);
  assert(finishOfflinePurchase());assert(!pendingBuyCash);
  u32 cash=0;assert(callFn(fnGetCash,{},&cash));assert(cash==12345);
  assert(rd16(exportsMap.at("MMain")+14624)==0);
 }
 std::puts("OFFLINE_STORE_ARM_OK");uc_close(uc);uc=nullptr;
}
