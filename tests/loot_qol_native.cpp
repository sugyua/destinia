#include <unicorn/unicorn.h>
#include <cassert>
#ifdef main
#undef main
#endif
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
static int npcCalls=0,attackCalls=0,saveCalls=0;
static void countSave(uc_engine*,uint64_t,uint32_t,void*){saveCalls++;}
static void countNpc(uc_engine*,uint64_t,uint32_t,void*){npcCalls++;}
static void countAttack(uc_engine*,uint64_t,uint32_t,void*){attackCalls++;}
int main(int argc,char** argv){
 setvbuf(stdout,nullptr,_IONBF,0);if(int rc=runHost(argc,argv))return rc;
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));assert(callFn(exportsMap.at("Map_loadData"),{2,49,1}));auto g=exportsMap.at("Game");
 auto sh=[&](u32 at,uint16_t v){assert(wr(at,&v,2));};
 sh(g+0x4854,0);sh(g+0x4866,0);sh(g+0x4890,100);sh(g+0x4892,100);sh(g+0x4878,0);sh(g+0x487a,0);wr32(rd32(SO_BASE+0x44240),exportsMap.at("GameCanvas"));
 for(int b=0;b<4;b++){std::array<uint8_t,144> empty{};wr(g+0x433e + b*0x92,empty.data(),empty.size());uint16_t bag[3]={4,0,4};wr(g+0x4338+b*0x92,bag,6);}assert(callFn(exportsMap.at("setMyGold"),{20000}));
 std::array<uint8_t,20> flags{};wr(g+0x3e76,flags.data(),flags.size());
 auto drop=[&](int i,int type,int id,int x,int y){auto p=g+0x3e90+60*i;std::array<uint8_t,60> empty{};wr(p,empty.data(),empty.size());const char label[]="LOOT TEST";wr(p,label,sizeof label);sh(p+40,x);sh(p+42,y);sh(p+48,type);sh(p+50,id);uint8_t flag=1;wr(g+0x3e76+i,&flag,1);};
 drop(0,5,50,100,100);drop(1,1,79,110,100);drop(2,1,0,100,100);drop(3,0,2,100,100);drop(4,2,0,100,100);drop(5,5,70,120,100);
 uc_hook hn,ha,hs;auto saveAt=exportsMap.at("GameCanvas_SaveGame")&~1u;uc_hook_add(uc,&hs,UC_HOOK_CODE,(void*)countSave,nullptr,saveAt,saveAt);auto npc=exportsMap.at("_checkNPCNearby")&~1u,attack=exportsMap.at("reserveHeroAct")&~1u;uc_hook_add(uc,&hn,UC_HOOK_CODE,(void*)countNpc,nullptr,npc,npc);uc_hook_add(uc,&ha,UC_HOOK_CODE,(void*)countAttack,nullptr,attack,attack);
 std::vector<uint8_t> before(32056),after(before.size());rd(g,before.data(),before.size());assert(lootQol.collect(0,true,[&](u32 fn){return callFn(fn,{6});}));rd(g,after.data(),after.size());assert(before==after);
 lootQol.options.autoGold=true;assert(lootQol.collect(4,true,[&](u32 fn){return callFn(fn,{6});}));u32 gold=0;assert(callFn(exportsMap.at("getMyGold"),{},&gold));printf("QOL_GOLD=%u flags=%u,%u\n",gold,rd16(g+0x3e76)&255,rd16(g+0x3e7b)&255);assert(gold==20050);assert(!(rd16(g+0x3e76)&255));
 assert(lootQol.collect(8,true,[&](u32 fn){return callFn(fn,{6});}));assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20050);assert(rd16(g+0x3e7b)&255);assert(npcCalls==0&&attackCalls==0);
 lootQol.options.autoMaterials=true;printf("PRE_MATERIAL state=%u act=%u count=%u cap=%u\n",rd16(g+0x4854),rd16(g+0x4866),rd16(g+0x433c),rd16(g+0x4338));assert(lootQol.collect(12,true,[&](u32 fn){printf("MATERIAL_TARGET=%d\n",lootQol.autoTarget);bool ok=callFn(fn,{6});printf("POST_MATERIAL flag=%u count=%u state=%u act=%u\n",rd16(g+0x3e77)&255,rd16(g+0x433c),rd16(g+0x4854),rd16(g+0x4866));return ok;}));assert(!(rd16(g+0x3e77)&255));assert(rd16(g+0x433c)==4);assert(rd16(g+0x433e)==1&&rd16(g+0x4340)==79&&rd16(g+0x4342)==1);
 for(int i:{2,3,4,5})assert(rd16(g+0x3e76+i)&255);
 // A full bag must retain the material, not delete or duplicate it.
 drop(6,1,80,100,100);for(int b=0;b<4;b++)for(int slot=0;slot<4;slot++){uint16_t full[3]={1,0,99};wr(g+0x433e + b*0x92+6*slot,full,6);}assert(lootQol.collect(16,true,[&](u32 fn){return callFn(fn,{6});}));assert(rd16(g+0x3e7c)&255);assert(rd16(g+0x433c)==4);
 sh(g+0x4854,16);rd(g,before.data(),before.size());assert(lootQol.collect(20,true,[&](u32 fn){return callFn(fn,{6});}));rd(g,after.data(),after.size());assert(before==after);sh(g+0x4854,0);
 rd(g,before.data(),before.size());assert(lootQol.collect(24,false,[&](u32 fn){return callFn(fn,{6});}));rd(g,after.data(),after.size());assert(before==after);
 lootQol.options.visual=true;std::vector<uint8_t> fb(FB_BYTES+32,0x34);lootQol.paint(fb.data()+16);for(int i=0;i<16;i++)assert(fb[i]==0x34&&fb[fb.size()-1-i]==0x34);rd(g,after.data(),after.size());assert(before==after);assert(npcCalls==0&&attackCalls==0);
 wr32(exportsMap.at("MMain")+4,rd32(exportsMap.at("Grp")+4));u32 pixels=0;assert(callFn(exportsMap.at("MC_GRP_GET_FRAME_BUFFER_POINTER"),{rd32(exportsMap.at("Grp")+4)},&pixels));
 rd(g,before.data(),before.size());std::vector<uint8_t> black(FB_BYTES),expected(FB_BYTES),actual(FB_BYTES);lootQol.options.visual=false;wr(pixels,black.data(),black.size());assert(callFn(exportsMap.at("GameCanvas_mPaint"),{}));rd(pixels,expected.data(),expected.size());rd(g,after.data(),after.size());
 wr(g,before.data(),before.size());lootQol.options.visual=true;lootQol.options.categories=0;wr(pixels,black.data(),black.size());assert(callFn(exportsMap.at("GameCanvas_mPaint"),{}));rd(pixels,actual.data(),actual.size());assert(expected!=actual);rd(g,before.data(),before.size());assert(before==after);assert(saveCalls==0);
 puts("LOOT_QOL_ARM_OK");return 0;
}
