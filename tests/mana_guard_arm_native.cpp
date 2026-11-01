#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepManaEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepManaEngine
#define main manaOriginalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
int main(int argc,char** argv){
 if(int result=manaOriginalMain(argc,argv))return result;
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 const auto g=exportsMap.at("Game");
 wr16(g+0x4886,0);wr16(g+0x4854,0);wr16(g+0x48fa,19);wr16(g+0x48f4,100);wr16(g+0x48f2,769);wr16(g+0x48f6,189);
 if(!callFn(exportsMap.at("GameCanvas_SaveGame"),{0}))return 8;
 for(int i=0;i<59;i++)if(correctExcessMana(g))return 2;
 if(!callFn(exportsMap.at("MMain_LoadData"),{}))return 9;
 if(!callFn(exportsMap.at("GameCanvas_LoadGame"),{0,0}))return 10;
 if(correctExcessMana(g))return 11;
 for(int i=0;i<58;i++)if(correctExcessMana(g))return 12;
 if(!correctExcessMana(g)||rd16(g+0x48f2)!=189)return 3;
 if(!callFn(exportsMap.at("GameCanvas_SaveGame"),{0}))return 4;
 wr16(g+0x48f2,1);
 if(!callFn(exportsMap.at("MMain_LoadData"),{}))return 5;
 if(!callFn(exportsMap.at("GameCanvas_LoadGame"),{0,0}))return 6;
 if(rd16(g+0x48f2)!=189||rd16(g+0x48f6)!=189)return 7;
 std::puts("MANA_ARM_OK");return 0;
}
