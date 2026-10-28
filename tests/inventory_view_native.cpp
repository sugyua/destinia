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
static int breakItemCalls=0;static uint32_t breakItemR0=0,breakItemR1=0;
static void captureBreakItem(uc_engine* engine,uint64_t,uint32_t,void*){++breakItemCalls;uc_reg_read(engine,UC_ARM_REG_R0,&breakItemR0);uc_reg_read(engine,UC_ARM_REG_R1,&breakItemR1);}
int main(int argc,char**argv){
 setvbuf(stdout,nullptr,_IONBF,0);
 if(int rc=originalMain(argc,argv))return rc;
 uc_hook breakHook=0;const auto breakEntry=exportsMap.at("_breakItem")&~1u;assert(uc_hook_add(uc,&breakHook,UC_HOOK_CODE,reinterpret_cast<void*>(captureBreakItem),nullptr,breakEntry,breakEntry)==UC_ERR_OK);
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
 // Integrated panel: opening/filtering/cancelling never changes Game.
 rd(g,before.data(),before.size());assert(inventoryMenuKey(10016,true));assert(inventoryOpen);
 assert(inventoryPanel.items.size()==3 && inventoryPanel.names.size()==3 && !inventoryPanel.names[0].empty());
 inventoryPanel.query.rank=4;inventoryPanel.rebuild();assert(inventoryPanel.view.empty());assert(!inventoryChoose());
 rd(g,after.data(),after.size());assert(before==after);inventoryPanel.reset();inventoryPanel.selectedPool=ids[1];
 assert(inventoryChoose() && !inventoryOpen && rd16(ui+0x56)==1);
 assert(inventoryEnter());auto old=inventoryPanel.items[0];auto address=g+0x2ed6+20*old.pool;auto raw=inventoryFingerprints[0];auto changed=raw;changed[19]^=1;wr(address,changed.data(),changed.size());inventoryPanel.selectedPool=old.pool;assert(!inventoryChoose()&&inventoryOpen);wr(address,raw.data(),raw.size());inventoryEvent(destinia::InventoryEvent::Close);assert(!inventoryOpen);
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
 // Equipment is a separate original tab. Include identical swords to catch
 // accidental selection by attributes or display position instead of pool.
 u32 duplicate=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{2},&duplicate));
 std::array<uint8_t,20> twin{};assert(rd(g+0x2ed6+20*ids[0],twin.data(),twin.size()));assert(wr(g+0x2ed6+20*duplicate,twin.data(),twin.size()));
 uint16_t count=4,record[3]={0,uint16_t(duplicate),1};wr(g+0x433c,&count,2);wr(g+0x433e + 18,record,6);
 assert(callFn(exportsMap.at("openMainmenu"),{0}));wr(ui+0x76,&anim,2);
 for(int key:{7,9,6}){assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));printf("EQUIP_NAV key=%d state=%08x\n",key,rd32(ui+0x7c));}
 printf("EQUIP_LIST count=%u pools=%u,%u,%u indices=%u,%u,%u cursor=%u\n",rd16(ui+0x324),rd16(ui+0x2e8),rd16(ui+0x2ea),rd16(ui+0x2ec),rd16(ui+0x52),rd16(ui+0x54),rd16(ui+0x56),rd16(ui+0x70));
 assert(rd32(ui+0x7c)==0x10000002u);
 int target=-1;for(int i=0;i<rd16(ui+0x324);i++)if(rd16(ui+0x2e8+2*i)==duplicate)target=i;assert(target>=0);
 assert(inventoryEnter());inventoryPanel.query.sort=destinia::InventorySort::Price;inventoryPanel.rebuild();inventoryPanel.selectedPool=duplicate;assert(inventoryChoose());
 assert(rd16(ui+0x56)==target);fcBeginFrame();
 assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));
 auto expected=g+0x2ed6+20*duplicate;
 assert(std::find(fcCandidates.begin(),fcCandidates.begin()+fcCandidateCount,expected)!=fcCandidates.begin()+fcCandidateCount);
 // Preview cancellation must restore the original unarmed slot.
 assert(rd32(ui+0x2148)==0);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{27}));assert(rd32(ui+0x7c)==0x10000001u);assert(rd32(g+0x4584)==0);
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));assert(rd16(ui+0x56)==0);
 for(int step=0;step<target;step++)assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{7}));
 assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 printf("EQUIP_CONFIRMED state=%08x live=%08x backup=%08x expected=%08x\n",rd32(ui+0x7c),rd32(g+0x4584),rd32(ui+0x2148),expected);
 assert(rd32(ui+0x2148)==expected);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{27}));assert(rd32(g+0x4584)==expected);
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);
 for(int slot=0;slot<3;slot++)assert(rd16(g+0x433e + 6*slot+2)==ids[slot]);
 assert(rd16(g+0x433e + 20)==duplicate);
 // Controlled local vendor entry; no NPC/network interaction or real save.
 assert(callFn(exportsMap.at("openMainmenu"),{0x91000001u}));wr(ui+0x76,&anim,2);
 for(int key:{9,6,9}){assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));printf("SELL_NAV key=%d state=%08x\n",key,rd32(ui+0x7c));}
 printf("SELL_FOCUS indices=%u,%u,%u,%u\n",rd16(ui+0x54),rd16(ui+0x56),rd16(ui+0x58),rd16(ui+0x5a));
 assert(rd32(ui+0x7c)==0x91040004u);
 // Protection toggles and the actual ARM confirmation must not mutate Game.
 // A sidecar changed externally after startup fails closed and is preserved.
 const auto sidecar=destinia::inventoryProtectionFile(fs::path(saveDir));
 {std::ofstream out(sidecar);out<<"CORRUPTED";}
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));assert(rd32(ui+0x7c)==0x91040004u);assert(!inventoryProtectionReady);
 {std::ifstream in(sidecar);std::string value{std::istreambuf_iterator<char>(in),{}};assert(value=="CORRUPTED");}
 {std::ofstream out(sidecar);out<<"DESTINIA_INVENTORY_PROTECT_1\n";}
 assert(inventoryRefreshProtection());
 assert(inventoryProtectionReady);assert(inventoryEnter());inventoryPanel.reset();inventoryPanel.selectedPool=ids[0];
 rd(g,before.data(),before.size());assert(inventoryToggleProtection());assert(inventoryPanel.isProtected());assert(!inventoryChoose());
 rd(g,after.data(),after.size());assert(before==after);inventoryOpen=false;
 uint16_t first=0;wr(ui+0x5a,&first,2);std::vector<uint8_t> uiBefore(12088),uiAfter(uiBefore.size());rd(ui,uiBefore.data(),uiBefore.size());
 for(int key:{6,17}){assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));assert(rd32(ui+0x7c)==0x91040004u);}
 rd(g,after.data(),after.size());assert(before==after);rd(ui,uiAfter.data(),uiAfter.size());assert(uiBefore==uiAfter);
 // Identical equipment in another pool remains independently selectable.
 assert(inventoryEnter());inventoryPanel.selectedPool=duplicate;assert(!inventoryPanel.isProtected());assert(inventoryChoose());
 assert(inventoryEnter());inventoryPanel.selectedPool=ids[0];assert(inventoryToggleProtection());assert(!inventoryPanel.isProtected());inventoryOpen=false;
 assert(inventoryEnter());assert(inventoryPanel.items[0].price==records[0].price/5);inventoryPanel.reset();inventoryPanel.selectedPool=duplicate;assert(inventoryChoose()&&!inventoryOpen);assert(rd16(ui+0x5a)==3);assert(inventoryEnter());inventoryPanel.query.sort=destinia::InventorySort::Price;inventoryPanel.rebuild();inventoryPanel.selectedPool=ids[0];assert(inventoryChoose()&&!inventoryOpen);
 // Displayed price-order index2 points back to original slot0, not slot2.
 auto projected=destinia::inventorySelection(records,view,int(ids[0]));assert(projected && *projected==2);
 const auto sale=records[view[*projected]];assert(sale.slot==0 && sale.pool==int(ids[0]));
 assert(rd16(ui+0x5a)==sale.slot);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 printf("SELL_CONFIRM state=%08x record=%08x expected=%08x\n",rd32(ui+0x7c),rd32(ui+0x2e0),g+0x433e + 6*sale.slot);
 assert(rd32(ui+0x2e0)==g+0x433e + 6*sale.slot);
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);assert(rd16(g+0x433c)==4);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{27}));
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));assert(gold==20000);assert(rd16(g+0x433c)==4);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 assert(callFn(exportsMap.at("getMyGold"),{},&gold));
 printf("SOLD gold=%u count=%u flag=%u twin=%u gear=%08x\n",gold,rd16(g+0x433c),uint8_t(rd16(g+0x2e0c+sale.pool)&255),uint8_t(rd16(g+0x2e0c+duplicate)&255),rd32(g+0x4584));
 assert(gold==20000+u32(sale.price/5));assert(rd16(g+0x433e + 6*sale.slot+4)==0);
 std::vector<destinia::InventoryItem> remaining;
 for(int slot=0;slot<rd16(g+0x433c);slot++)if(rd16(g+0x433e + 6*slot+4)){
  int pool=rd16(g+0x433e + 6*slot+2);u32 price=0;assert(callFn(exportsMap.at("GameUI_getItemPrice"),{0,u32(pool)},&price));
  auto header=rd16(g+0x2ed6+20*pool);u32 level=0;assert(callFn(exportsMap.at("GameUI_getEquipMinLv"),{header,1},&level));
  remaining.push_back({0,slot,pool,int((header>>5)&7),int((header>>8)&7),int(level),int(price)});
 }
 assert(remaining.size()==3);auto nextView=destinia::inventoryView(remaining,query);
 assert(!destinia::inventorySelection(remaining,nextView,sale.pool));
 assert(destinia::inventorySelection(remaining,nextView,duplicate));
 assert(uint8_t(rd16(g+0x2e0c+sale.pool)&255)==0 && uint8_t(rd16(g+0x2e0c+duplicate)&255)!=0);
 assert(rd32(g+0x4584)==expected);
 std::array<uint8_t,20> retained{};assert(rd(expected,retained.data(),retained.size()));assert(retained==twin);
 // Acknowledge the completed sale; resolve the twin again in the current
 // original bag, since the native menu may compact zero-quantity records.
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 printf("SELL_ACK state=%08x gold=%u\n",rd32(ui+0x7c),gold);
 assert(rd32(ui+0x7c)==0x91040004u);
 assert(inventoryEnter());assert(inventoryPanel.selectedPool==-1);inventoryEvent(destinia::InventoryEvent::Close);
 int equippedSlot=-1;for(int slot=0;slot<rd16(g+0x433c);slot++)if(rd16(g+0x433e + 6*slot+4)&&rd16(g+0x433e + 6*slot+2)==duplicate)equippedSlot=slot;assert(equippedSlot>=0);
 for(int moves=0;rd16(ui+0x5a)!=equippedSlot && moves<24;moves++)assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{7}));
 assert(rd16(ui+0x5a)==equippedSlot);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));
 printf("EQUIPPED_SALE_GUARD state=%08x\n",rd32(ui+0x7c));
 assert(rd32(ui+0x7c)==0x91040106u);
 u32 guardedGold=0;assert(callFn(exportsMap.at("getMyGold"),{},&guardedGold));assert(guardedGold==gold);
 assert(rd32(g+0x4584)==expected && uint8_t(rd16(g+0x2e0c+duplicate)&255)!=0);
 assert(callFn(exportsMap.at("openMainmenu"),{0}));wr(ui+0x76,&anim,2);
 for(int key:{7,7,9,6,9})assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(key)}));
 assert(rd32(ui+0x7c)==0x20000002u);
 int salvageSlot=-1;for(int slot=0;slot<rd16(g+0x433c);++slot)if(rd16(g+0x433e + 6*slot + 4)&&rd16(g+0x433e + 6*slot + 2)==ids[1])salvageSlot=slot;
 assert(salvageSlot>=0);const uint16_t bag=0,slot=uint16_t(salvageSlot);wr(ui+0x54,&bag,2);wr(ui+0x56,&slot,2);
 inventoryProtection.select(rd16(g+0x4886));std::array<uint8_t,20> salvageFingerprint{};assert(rd(g+0x2ed6+20*ids[1],salvageFingerprint.data(),salvageFingerprint.size()));assert(inventoryProtection.toggle(ids[1],salvageFingerprint));destinia::saveInventoryProtection(fs::path(saveDir),inventoryProtection);assert(inventoryRefreshProtection());
 assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));assert(rd32(ui+0x7c)==0x20000003u);
 printf("BREAK_MENU options=%u,%u,%u,%u cursor=%u\n",rd16(ui+0x326),rd16(ui+0x328),rd16(ui+0x32a),rd16(ui+0x32c),rd16(ui+0x58));
 const uint16_t dismantleOption=1;wr(ui+0x58,&dismantleOption,2);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));
 printf("BREAK_CONFIRM state=%08x calls=%d count=%u\n",rd32(ui+0x7c),breakItemCalls,rd16(g+0x433c));
 assert(rd32(ui+0x7c)==0x20030003u&&breakItemCalls==0);
 const auto protectedCount=rd16(g+0x433c);assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));
 assert(breakItemCalls==0&&rd32(ui+0x7c)==0x20030003u&&rd16(g+0x433c)==protectedCount);
 assert(rd16(g+0x433e + 6*salvageSlot + 2)==ids[1]&&rd16(g+0x433e + 6*salvageSlot + 4)==1);
 assert(!inventoryProtection.toggle(ids[1],salvageFingerprint));destinia::saveInventoryProtection(fs::path(saveDir),inventoryProtection);assert(inventoryRefreshProtection());
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));
 printf("BREAK_RESULT state=%08x calls=%d args=%u,%u count=%u active=%u rows=",rd32(ui+0x7c),breakItemCalls,breakItemR0,breakItemR1,rd16(g+0x433c),uint8_t(rd16(g+0x2e0c+ids[1])&255));
 for(int slot=0;slot<rd16(g+0x433c);++slot)printf("[%u,%u,%u]",rd16(g+0x433e + 6*slot),rd16(g+0x433e + 6*slot + 2),rd16(g+0x433e + 6*slot + 4));puts("");
 assert(breakItemCalls==1&&breakItemR0==0&&breakItemR1==uint32_t(salvageSlot));
 assert(uint8_t(rd16(g+0x2e0c+ids[1])&255)==0);
 bool materialReward=false;for(int slot=0;slot<rd16(g+0x433c);++slot){auto record=g+0x433e + 6*slot;if(rd16(record)==1&&rd16(record+4)>0)materialReward=true;}assert(materialReward);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));printf("BREAK_ACK state=%08x\n",rd32(ui+0x7c));assert(rd32(ui+0x7c)==0x20000002u);
 int secondSalvageSlot=-1;for(int at=0;at<rd16(g+0x433c);++at)if(rd16(g+0x433e + 6*at + 4)&&rd16(g+0x433e + 6*at + 2)==ids[2])secondSalvageSlot=at;assert(secondSalvageSlot>=0);
 const uint16_t secondBag=0,secondSlot=uint16_t(secondSalvageSlot);wr(ui+0x54,&secondBag,2);wr(ui+0x56,&secondSlot,2);assert(callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)}));
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{6}));const uint16_t secondDismantle=1;wr(ui+0x58,&secondDismantle,2);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17})&&rd32(ui+0x7c)==0x20030003u);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));assert(breakItemCalls==2&&breakItemR0==0&&breakItemR1==uint32_t(secondSalvageSlot));assert(uint8_t(rd16(g+0x2e0c+ids[2])&255)==0);
 assert(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17})&&rd32(ui+0x7c)==0x20000002u);
 u32 protectedBatchPool=0,salvageBatchPool=0;assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{2},&protectedBatchPool));assert(callFn(exportsMap.at("GameUI_getNewEquipByIdx"),{33},&salvageBatchPool));assert(protectedBatchPool!=salvageBatchPool);
 const uint16_t batchCount=6;wr(g+0x433c,&batchCount,2);uint16_t protectedRecord[3]={0,uint16_t(protectedBatchPool),1},salvageRecord[3]={0,uint16_t(salvageBatchPool),1};wr(g+0x433e + 24,protectedRecord,6);wr(g+0x433e + 30,salvageRecord,6);
 std::array<uint8_t,20> protectedBatchFingerprint{};assert(rd(g+0x2ed6+20*protectedBatchPool,protectedBatchFingerprint.data(),protectedBatchFingerprint.size()));inventoryProtection.select(rd16(g+0x4886));assert(inventoryProtection.toggle(protectedBatchPool,protectedBatchFingerprint));destinia::saveInventoryProtection(fs::path(saveDir),inventoryProtection);assert(inventoryRefreshProtection());
 inventoryOpen=false;assert(inventoryEnter());inventoryPanel.reset();const auto batchEvent=inventoryPanel.hit(13,rd16(g+0x48fa));assert(batchEvent==destinia::InventoryEvent::BatchPreview);inventoryEvent(batchEvent);
 assert(inventoryPanel.batchPreview&&inventoryPanel.batchCount==1&&inventoryPanel.batchProtected==1&&inventoryPanel.batchEquipped==1);
 assert(inventoryMenuKey(10016,true));assert(!inventoryOpen&&!inventoryPanel.batchPreview&&inventoryBatchTargets.empty());
 assert(inventoryEnter());inventoryPanel.reset();inventoryEvent(inventoryPanel.hit(13,rd16(g+0x48fa)));
 assert(inventoryPanel.batchPreview&&!inventoryBatchTargets.empty());
 inventoryProtectionHook(nullptr,0,0,reinterpret_cast<void*>(2));
 assert(!inventoryOpen&&!inventoryPanel.batchPreview&&inventoryBatchTargets.empty());
 assert(inventoryEnter());inventoryPanel.reset();inventoryEvent(inventoryPanel.hit(13,rd16(g+0x48fa)));
 assert(inventoryPanel.batchPreview&&!inventoryBatchTargets.empty());
 rd(g,before.data(),before.size());inventoryEvent(destinia::InventoryEvent::BatchCancel);rd(g,after.data(),after.size());assert(before==after&&!inventoryPanel.batchPreview&&inventoryBatchTargets.empty());
 inventoryEvent(inventoryPanel.hit(13,rd16(g+0x48fa)));assert(inventoryPanel.batchPreview&&inventoryPanel.batchCount==1);
 const auto materialBefore=rd16(g+0x433e + 4);const auto equippedPool=duplicate;
 const bool batchRan=inventoryRunBatch();printf("BATCH_RESULT ok=%d count=%zu msg=%s\n",batchRan,inventoryPanel.batchCount,inventoryPanel.error.c_str());assert(batchRan);assert(!inventoryPanel.batchPreview&&inventoryPanel.error=="LOTE CONCLUIDO: 1 ITENS");
 assert(uint8_t(rd16(g+0x2e0c+protectedBatchPool)&255)!=0&&uint8_t(rd16(g+0x2e0c+salvageBatchPool)&255)==0&&uint8_t(rd16(g+0x2e0c+equippedPool)&255)!=0);
 assert(rd16(g+0x433e + 4)>materialBefore);
 puts("INVENTORY_SELECTION_ARM_OK");
}
