// Shared by both hosts after ARM helpers, Codex and FC declarations.
#include "inventory_panel.h"
static destinia::InventoryPanel inventoryPanel;
static bool inventoryOpen=false,inventoryTouchCaptured=false;
static u32 inventoryContext=0,inventoryNameBuffer=0;
static std::vector<std::array<uint8_t,20>> inventoryFingerprints;
static std::vector<int> inventoryNativeIndices;
static bool inventoryVisible(){if(codexOpen||!exportsMap.count("GameUI"))return false;auto ui=exportsMap.at("GameUI");auto state=rd32(ui+0x7c);return rd16(exportsMap.at("Game")+0x4854)==16&&rd16(ui+0x76)>=2&&(state==0x10000002u||state==0x20000002u||state==0x91040004u);}
static int inventoryCursorOffset(u32 state){return state==0x91040004u?0x5a:0x56;}
static bool inventoryRead(destinia::InventoryPanel& panel,std::vector<std::array<uint8_t,20>>& fingerprints,std::vector<int>& indices,int& selected){
 if(!inventoryVisible())return false;auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");auto state=rd32(ui+0x7c);int bag=state==0x10000002u?-1:rd16(ui+(state==0x91040004u?0x58:0x54));if(bag>=4)return false;
 if(state==0x91040004u&&rd32(ui+0x2d8)!=g+0x433c)return false;
 std::map<int,int> equipmentList;if(state==0x10000002u){int count=rd16(ui+0x324);if(count>200)return false;for(int i=0;i<count;i++){int pool=rd16(ui+0x2e8+2*i);if(pool>=200||equipmentList.count(pool))return false;equipmentList[pool]=i;}}
 panel.items.clear();panel.names.clear();fingerprints.clear();indices.clear();selected=-1;int cursor=rd16(ui+inventoryCursorOffset(state));if(!inventoryNameBuffer)inventoryNameBuffer=hmalloc(640);if(!inventoryNameBuffer)return false;
 for(int b=0;b<4;b++){if(bag>=0&&bag!=b)continue;int count=rd16(g+0x433c+b*0x92);if(count>24)return false;for(int slot=0;slot<count;slot++){
  auto record=g+0x433e + b*0x92+6*slot;if(!rd16(record+4)||rd16(record)!=0)continue;int pool=rd16(record+2);if(pool>=200||uint8_t(rd16(g+0x2e0c+pool)&255)==0)return false;if(bag<0&&!equipmentList.count(pool))continue;
  std::array<uint8_t,20> fingerprint{};if(!rd(g+0x2ed6+20*pool,fingerprint.data(),fingerprint.size()))return false;int header=rd16(g+0x2ed6+20*pool),rank=(header>>8)&7;if(rank>4)return false;u32 level=0,price=0;
  if(!callFn(exportsMap.at("GameUI_getEquipMinLv"),{u32(header),1},&level)||!callFn(exportsMap.at("GameUI_getItemPrice"),{0,u32(pool)},&price)||!callFn(exportsMap.at("GameUI_getEquipName"),{inventoryNameBuffer,g+0x2ed6+20*pool}))return false;
  int nativeIndex=bag<0?equipmentList.at(pool):slot;panel.items.push_back({b,slot,pool,(header>>5)&7,rank,int(level),int(price)});panel.names.push_back(cstr(inventoryNameBuffer,512));fingerprints.push_back(fingerprint);indices.push_back(nativeIndex);if(nativeIndex==cursor)selected=pool;
 }}
 panel.rebuild();return true;
}
static bool inventoryEnter(){if(!inventoryVisible())return false;try{auto panel=inventoryPanel;std::vector<std::array<uint8_t,20>> fingerprints;std::vector<int> indices;int selected=-1;if(!inventoryRead(panel,fingerprints,indices,selected))return false;if(inventoryContext==rd32(exportsMap.at("GameUI")+0x7c)&&inventoryPanel.selectedPool>=0&&std::none_of(panel.items.begin(),panel.items.end(),[&](const auto& item){return item.pool==inventoryPanel.selectedPool;}))selected=-1;panel.selectedPool=selected;panel.focus=7;panel.rebuild();inventoryPanel=std::move(panel);inventoryFingerprints=std::move(fingerprints);inventoryNativeIndices=std::move(indices);inventoryContext=rd32(exportsMap.at("GameUI")+0x7c);inventoryOpen=true;return true;}catch(const std::exception& e){hostReport("[inventario] Filtros indisponiveis: %s\n",e.what());return false;}}
static bool inventoryChoose(){auto selected=inventoryPanel.selected();if(!selected)return false;size_t index=inventoryPanel.view[*selected];if(index>=inventoryFingerprints.size())return false;auto target=inventoryPanel.items[index];
 try{if(!inventoryVisible()||rd32(exportsMap.at("GameUI")+0x7c)!=inventoryContext)throw std::runtime_error("CONTEXTO MUDOU - REABRA");destinia::InventoryPanel fresh;std::vector<std::array<uint8_t,20>> prints;std::vector<int> indices;int current=-1;if(!inventoryRead(fresh,prints,indices,current))throw std::runtime_error("INVENTARIO NAO VERIFICADO");size_t found=fresh.items.size();for(size_t i=0;i<fresh.items.size();i++)if(fresh.items[i].pool==target.pool)found=i;if(found==fresh.items.size()||!(fresh.items[found]==target)||prints[found]!=inventoryFingerprints[index])throw std::runtime_error("ITEM MUDOU - REABRA");
  auto ui=exportsMap.at("GameUI"),g=exportsMap.at("Game");int offset=inventoryCursorOffset(inventoryContext),wanted=indices[found];for(int steps=0;rd16(ui+offset)!=wanted&&steps<200;steps++){int before=rd16(ui+offset);if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(before<wanted?7:5)})||rd32(ui+0x7c)!=inventoryContext||rd16(ui+offset)==before)throw std::runtime_error("NAVEGACAO INDISPONIVEL");}
  if(rd16(ui+offset)!=wanted)throw std::runtime_error("SELECAO NAO CONFIRMADA");
  if(inventoryContext==0x10000002u){if(rd16(ui+0x2e8+2*wanted)!=target.pool||!callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)})||rd32(g+0x4584+4*rd16(ui+0x54))!=g+0x2ed6+20*target.pool)throw std::runtime_error("PREVIA NAO CONFIRMADA");}
  else{auto record=g+0x433e + target.bag*0x92+6*wanted;if(!rd16(record+4)||rd16(record+2)!=target.pool)throw std::runtime_error("ALVO NAO CONFIRMADO");}
  inventoryOpen=false;return true;
 }catch(const std::exception& e){inventoryPanel.error=e.what();return false;}}
static void inventoryEvent(destinia::InventoryEvent event){if(event==destinia::InventoryEvent::Close)inventoryOpen=false;else if(event==destinia::InventoryEvent::Choose)inventoryChoose();}
static bool inventoryMenuKey(int key,bool down){if(!down)return false;if(inventoryOpen){inventoryEvent(inventoryPanel.key(key,rd16(exportsMap.at("Game")+0x48fa)));return true;}if(key==12&&inventoryVisible())return inventoryEnter();return false;}
static bool inventoryMenuTouch(int x,int y,int phase){if(inventoryTouchCaptured&&phase!=0){if(phase==1)inventoryTouchCaptured=false;return true;}if(!inventoryOpen&&!inventoryVisible())return false;int hit=destinia::inventoryHit(x,y,SCREEN_W,SCREEN_H,preferences.uiScale,inventoryOpen);if(inventoryOpen){if(phase==0){inventoryTouchCaptured=true;inventoryEvent(inventoryPanel.hit(hit,rd16(exportsMap.at("Game")+0x48fa)));}return true;}if(phase==0&&hit==20){inventoryTouchCaptured=inventoryEnter();return inventoryTouchCaptured;}return false;}
static void inventoryPaint(uint8_t* pixels){destinia::paintInventory(pixels,SCREEN_W,SCREEN_H,preferences.uiScale,inventoryPanel,inventoryOpen,inventoryVisible());}
