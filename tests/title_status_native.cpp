#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepTitleEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepTitleEngine
#define main originalTitleMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static std::string found;
static void captureTitle(uc_engine*,uint64_t,uint32_t,void*){if((regLR()&~1u)==SO_BASE+0x31c40)found=cstr(R(1));}
#define CHECK(e) do{if(!(e)){std::fprintf(stderr,"FAIL %d: %s; text=%s\n",__LINE__,#e,found.c_str());return 1;}}while(false)
int main(int argc,char**argv){
 if(int r=originalTitleMain(argc,argv))return r;CHECK(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 const u32 g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");wr16(g+0x4854,16);wr16(g+0x4886,0);wr16(g+0x48fa,20);wr16(g+0x48f0,1);wr16(ui+0x76,16);wr32(ui+0x7c,1);
 auto table=exportsMap.at("frameBuffer")+16,desc=hmalloc(12),pixels=hmalloc(FB_BYTES);wr32(desc,SCREEN_W);wr32(desc+4,SCREEN_H);wr32(desc+8,pixels);wr32(table,desc);
 uc_hook h;auto at=exportsMap.at("Font_drawStringChar")&~1u;CHECK(uc_hook_add(uc,&h,UC_HOOK_CODE,(void*)captureTitle,nullptr,at,at)==UC_ERR_OK);
 found.clear();CHECK(callFn(exportsMap.at("_drawMenuStatus"),{4,84,25,232}));CHECK(found=="Novice");
 bossRushUI.titlesStore.setEquipped(0,destinia::TitleId::Iniciante);found.clear();CHECK(callFn(exportsMap.at("_drawMenuStatus"),{4,84,25,232}));CHECK(found=="Iniciante");
 bossRushUI.titlesStore.setEquipped(0,destinia::TitleId::SenhorDoAbismo);found.clear();CHECK(callFn(exportsMap.at("_drawMenuStatus"),{4,84,25,232}));CHECK(found=="Novice");
 puts("TITLE_STATUS_ARM_OK");return 0;
}
