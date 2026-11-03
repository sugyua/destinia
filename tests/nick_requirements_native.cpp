#include <unicorn/unicorn.h>
#include <cstdio>
#include <string>
static uc_err keepNickEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepNickEngine
#define main originalNickMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static int hits=0; static std::string captured;
static void observe(uc_engine*,uint64_t,uint32_t,void*){++hits;char b[256]{};if(rd(R(7),b,sizeof(b)-1))captured=b;}
int main(int argc,char**argv){
 if(int r=originalNickMain(argc,argv))return r;
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 const auto ui=exportsMap.at("GameUI"),game=exportsMap.at("Game");
 auto table=exportsMap.at("frameBuffer")+16,desc=hmalloc(12),pixels=hmalloc(FB_BYTES);wr32(desc,SCREEN_W);wr32(desc+4,SCREEN_H);wr32(desc+8,pixels);wr32(table,desc);
 uc_hook h;auto pc=SO_BASE+0x2cac6;if(uc_hook_add(uc,&h,UC_HOOK_CODE,(void*)observe,nullptr,pc,pc)!=UC_ERR_OK)return 2;
 const char* forms[]={"Achieve Level %u","Complete %u sub-quests.","Kill at least %u monsters.","Obtain at least %u gold from hunting.","Craft at least %u times.","Reinforce at least %u times.","Resurrect at least %u times."};
 const char* formsPt[]={"Alcance o nivel %u.","Conclua %u missoes secundarias.","Derrote %u monstros.","Ganhe %u de ouro cacando.","Fabrique %u vezes.","Reforce %u vezes.","Ressuscite %u vezes."};
 for(int index=1;index<50;++index)for(int unlocked: {0,1}){
  wr16(ui+0x56,index);wr32(game+0x7758+index*4,unlocked);hits=0;captured.clear();
  if(!callFn(exportsMap.at("_drawNickSel"),{4,84,25,232}))return 3;
  char expected[256]{};snprintf(expected,sizeof expected,(portugueseTextTest?formsPt:forms)[(index-1)/7],rd32(exportsMap.at("NICK_CON_ACQUIRE")+index*4));
  if(hits!=1||captured!=expected||rd32(game+0x7758+index*4)!=unsigned(unlocked)){fprintf(stderr,"FAIL index=%d unlocked=%d got=%s expected=%s hits=%d\n",index,unlocked,captured.c_str(),expected,hits);return 4;}
 }
 puts("ORIGINAL_TITLE_REQUIREMENTS_OK");return 0;
}
