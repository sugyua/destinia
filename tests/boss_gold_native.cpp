#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepGoldEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepGoldEngine
#define main goldOriginalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
int main(int argc,char** argv){
 if(int result=goldOriginalMain(argc,argv))return result;
 if(!callFn(exportsMap.at("setMyGold"),{15113}))return 1;
 const int observed=bossRushUI.getGold();
 if(observed!=15113){std::fprintf(stderr,"expected 15113, got %d\n",observed);return 2;}
 std::puts("BOSS_GOLD_READ_OK");return 0;
}
