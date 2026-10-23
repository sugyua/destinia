// Original ARM routing; controlled state, skill execution intercepted. No campaign claim.
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
static int slot=-1;
static void capture(uc_engine*,uint64_t,uint32_t,void*) {
 slot=R(0);u32 lr=0;uc_reg_read(uc,UC_ARM_REG_LR,&lr);uc_reg_write(uc,UC_ARM_REG_PC,&lr);
}
int main(int argc,char**argv){
 if(int r=originalMain(argc,argv))return r;
 auto game=exportsMap.at("Game"),ui=exportsMap.at("GameUI");uint16_t active=0;wr(game+0x4854,&active,2);wr32(ui+0x7c,0);
 uc_hook hook;auto pc=exportsMap.at("_pressedQuickSlot")&~1u;assert(uc_hook_add(uc,&hook,UC_HOOK_CODE,(void*)capture,nullptr,pc,pc)==UC_ERR_OK);
 for(const auto binding:std::vector<std::pair<u32,int>>{{2,0},{11,2},{4,3},{8,1}}){
  slot=-1;assert(callFn(exportsMap.at("GameCanvas_mKeyPressed"),{binding.first}));assert(slot==binding.second);
 }
 printf("QUICKSLOT_MAPPING_ARM_OK\n");
 return 0;
}
