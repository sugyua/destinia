#include <unicorn/unicorn.h>
#include <cassert>
#include <cstdio>
static uc_err keepEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine
#define main originalTitleMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
static u32 roll=0;static int outcome=-1;
static bool badMemory(uc_engine* e,uc_mem_type,uint64_t at,int,int64_t,void*){u32 pc=0;uc_reg_read(e,UC_ARM_REG_PC,&pc);std::fprintf(stderr,"INVALID at=%llx pc=%x\n",(unsigned long long)at,pc);return false;}
static void fixedRandom(uc_engine* e,uint64_t,uint32_t,void*){u32 lr=0;uc_reg_read(e,UC_ARM_REG_LR,&lr);uc_reg_write(e,UC_ARM_REG_R0,&roll);uc_reg_write(e,UC_ARM_REG_PC,&lr);}
static void critical(uc_engine* e,uint64_t,uint32_t,void*){outcome=1;uc_emu_stop(e);}
static void normal(uc_engine* e,uint64_t,uint32_t,void*){outcome=0;uc_emu_stop(e);}
static int runBranch(u32 game,unsigned randomValue){
 u32 table=hmalloc(8),sp=STACK_BASE+STACK_SIZE-0x200,r5=0,r8=game+0x14fc;
 wr32(table,game);wr32(sp+0x30,0);wr32(sp+0xc,table);
 roll=randomValue;outcome=-1;
 if(uc_reg_write(uc,UC_ARM_REG_R4,&table)!=UC_ERR_OK||uc_reg_write(uc,UC_ARM_REG_R5,&r5)!=UC_ERR_OK||uc_reg_write(uc,UC_ARM_REG_R12,&r5)!=UC_ERR_OK||uc_reg_write(uc,UC_ARM_REG_R8,&r8)!=UC_ERR_OK||uc_reg_write(uc,UC_ARM_REG_SP,&sp)!=UC_ERR_OK)return -3;
 uc_hook a=0,b=0,c=0,invalid=0;uc_hook_add(uc,&invalid,UC_HOOK_MEM_INVALID,(void*)badMemory,nullptr,1,0);const auto random=exportsMap.at("getRandom")&~1u;
 if(uc_hook_add(uc,&a,UC_HOOK_CODE,(void*)fixedRandom,nullptr,random,random)!=UC_ERR_OK||uc_hook_add(uc,&b,UC_HOOK_CODE,(void*)critical,nullptr,SO_BASE+0x1379e,SO_BASE+0x1379e)!=UC_ERR_OK||uc_hook_add(uc,&c,UC_HOOK_CODE,(void*)normal,nullptr,SO_BASE+0x137ca,SO_BASE+0x137ca)!=UC_ERR_OK)return -4;
 const auto err=uc_emu_start(uc,(SO_BASE+0x13778)|1u,SO_BASE+0x137d0,0,1000);uc_hook_del(uc,a);uc_hook_del(uc,b);uc_hook_del(uc,c);uc_hook_del(uc,invalid);
 if(err!=UC_ERR_OK){std::fprintf(stderr,"ARM_CRIT_ERROR %d\n",int(err));return -5;}return outcome;
}
int main(int argc,char**argv){
 if(int r=originalTitleMain(argc,argv))return r;
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 const u32 g=exportsMap.at("Game");wr16(g+0x4886,0);wr16(g+0x48fa,20);wr16(g+0x7574,65535);for(int i=0;i<6;i++)wr32(g+0x4584+4*i,0);
 uint16_t stats[4]={20,15,10,10};if(!wr(g+0x48ba,stats,sizeof stats))return 2;
 if(!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))return 3;
 const unsigned before=rd16(g+0x48d0);if(before>=900)return 4;
 const unsigned chosen=before+50;if(runBranch(g,chosen)!=0)return 5;
 auto& store=bossRushUI.titlesStore;store.unlock(0,destinia::TitleId::CacadorDeFungos);store.setEquipped(0,destinia::TitleId::CacadorDeFungos);store.setExclusive(0,true);
 if(!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))return 6;
 if(rd16(g+0x48d0)!=before+100)return 7;
 if(runBranch(g,chosen)!=1)return 8;
 if(runBranch(g,before+100)!=0)return 9;
 std::puts("FUNGUS_CRITICAL_ARM_OK");return 0;
}
