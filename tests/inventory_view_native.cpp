#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine
#ifdef main
#undef main
#endif
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#include "../common/inventory_view.h"
#undef main
#undef uc_close
int main(int argc,char**argv){
 setvbuf(stdout,nullptr,_IONBF,0);
 if(int rc=originalMain(argc,argv))return rc;
 auto g=exportsMap.at("Game");assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 u32 ids[3]={};int encoded[3]={2,1,33};for(int i=0;i<3;i++)assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{u32(encoded[i])},ids+i));
 for(int b=0;b<4;b++){uint16_t header[3]={4,0,uint16_t(b?0:3)};wr(g+0x4338+b*0x92,header,6);}
 for(int i=0;i<3;i++){uint16_t record[3]={0,uint16_t(ids[i]),1};wr(g+0x433e + 6*i,record,6);}
 assert(callFn(exportsMap.at("setMyGold"),{20000}));std::vector<uint8_t> before(32056),after(before.size());rd(g,before.data(),before.size());
 for(int i=0;i<3;i++){u32 price=0;assert(callFn(exportsMap.at("GameUI_getItemPrice"),{0,ids[i]},&price));printf("PRICE pool=%u encoded=%d gold=%u\n",ids[i],encoded[i],price);}
 rd(g,after.data(),after.size());assert(before==after);
 std::vector<destinia::InventoryItem> records;
 for(int slot=0;slot<3;slot++){
  auto pool=rd16(g+0x433e + 6*slot+2);auto header=rd16(g+0x2ed6+20*pool);u32 price=0,level=0;
  assert(callFn(exportsMap.at("GameUI_getItemPrice"),{0,pool},&price));
  assert(callFn(exportsMap.at("GameUI_getEquipMinLv"),{header,1},&level));
  records.push_back({0,slot,int(pool),int((header>>5)&7),int((header>>8)&7),int(level),int(price)});
 }
 destinia::InventoryQuery query;query.sort=destinia::InventorySort::Price;auto view=destinia::inventoryView(records,query);
 rd(g,after.data(),after.size());assert(before==after);assert(records[view[0]].pool==int(ids[1]));
 // Prove original menu selection follows the original slot, never display rank.
 auto ui=exportsMap.at("GameUI");uint16_t base[4]={20,15,10,10},heroLevel=20,none=65535;
 wr(g+0x48ba,base,8);wr(g+0x48fa,&heroLevel,2);wr(g+0x7574,&none,2);
 for(int i=0;i<6;i++)wr32(g+0x4584+4*i,0);assert(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 assert(callFn(exportsMap.at("openMainmenu"),{0}));uint16_t anim=16;wr(ui+0x76,&anim,2);wr32(rd32(SO_BASE+0x44240),exportsMap.at("GameCanvas"));
 for(int key:{7,7,9,6,9}){assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));}
 printf("MENU state=%08x indices=%u,%u,%u\n",rd32(ui+0x7c),rd16(ui+0x52),rd16(ui+0x54),rd16(ui+0x56));
 assert(rd32(ui+0x7c)==0x20000002u);
 for(auto displayed:view){
  const auto& item=records[displayed];uint16_t bag=item.bag,slot=item.slot;
  wr(ui+0x54,&bag,2);wr(ui+0x56,&slot,2);fcBeginFrame();
  assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));
  printf("SELECT shown=%zu original=%d pool=%d candidates=%zu\n",displayed,slot,item.pool,fcCandidateCount);
  assert(std::find(fcCandidates.begin(),fcCandidates.begin()+fcCandidateCount,g+0x2ed6+20*item.pool)!=fcCandidates.begin()+fcCandidateCount);
 }
 assert(rd16(g+0x433c)==3);for(int slot=0;slot<3;slot++)assert(rd16(g+0x433e + 6*slot+2)==ids[slot]);
 u32 gold=0;assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);
 // Inspect and cancel the original action popup. This menu offers compound,
 // disassemble, move and discard; do not mistake its first action for equip.
 const auto chosen=records[view.back()];
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 assert(rd32(ui+0x7c)==0x20000003u);
 const uint16_t actions[4]={1,2,0,3};for(int i=0;i<4;i++)assert(rd16(ui+0x326+2*i)==actions[i]);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{27}));
 assert(rd32(ui+0x7c)==0x20000002u && rd16(ui+0x56)==chosen.slot);
 for(int slot=0;slot<3;slot++)assert(rd16(g+0x433e + 6*slot+2)==ids[slot]);
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);
 puts("INVENTORY_SELECTION_ARM_OK");
}
