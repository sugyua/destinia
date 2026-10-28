// Shared by both hosts after ARM helpers, Codex and FC declarations.
#include "inventory_panel.h"
#include "inventory_protection_store.h"
static destinia::InventoryPanel inventoryPanel;
static destinia::InventoryProtection inventoryProtection;
static bool inventoryProtectionReady=false;
static bool inventoryOpen=false,inventoryTouchCaptured=false;
static u32 inventoryContext=0,inventoryNameBuffer=0;
static std::vector<std::array<uint8_t,20>> inventoryFingerprints;
static std::vector<int> inventoryNativeIndices;
struct InventoryIcon {u32 pointer;int x,y;};
static std::vector<InventoryIcon> inventoryIcons;
static std::vector<uint8_t> inventoryUpgradeState;
static std::map<u32,bool> inventoryUpgradeCache;
static void inventoryBeginFrame(){inventoryIcons.clear();}
static void inventoryRefreshUpgrades(){
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");std::vector<uint8_t> state(32056+24);
 if(!rd(g,state.data(),32056)||!rd(ui+0x2148,state.data()+32056,24))throw std::runtime_error("FC: estado indisponivel");
 if(state!=inventoryUpgradeState){inventoryUpgradeState=std::move(state);inventoryUpgradeCache.clear();}
}
static bool inventoryIsUpgrade(u32 pointer){
 auto found=inventoryUpgradeCache.find(pointer);if(found!=inventoryUpgradeCache.end())return found->second;
 const auto preview=fcCompareEquipment(pointer,true);return inventoryUpgradeCache[pointer]=preview.valid&&preview.projected>preview.current;
}
static void inventoryIconHook(uc_engine*,uint64_t,uint32_t,void*){
 if(inventoryIcons.size()<200)inventoryIcons.push_back({R(3),int32_t(R(1)),int32_t(R(2))});
}
static bool inventoryRefreshProtection(){
 try{destinia::InventoryProtection next;destinia::loadInventoryProtection(fs::path(saveDir),next);inventoryProtection=std::move(next);inventoryProtectionReady=true;return true;}
 catch(const std::exception& e){if(inventoryProtectionReady)hostReport("[inventario] %s; sidecar preservado.\n",e.what());inventoryProtectionReady=false;inventoryPanel.error="SIDECAR INVALIDO - VENDA BLOQUEADA";return false;}
}
static bool inventoryVisible(){if(codexOpen||!exportsMap.count("GameUI"))return false;auto ui=exportsMap.at("GameUI");auto state=rd32(ui+0x7c);return rd16(exportsMap.at("Game")+0x4854)==16&&rd16(ui+0x76)>=2&&(state==0x10000002u||state==0x20000002u||state==0x91040004u);}
static int inventoryCursorOffset(u32 state){return state==0x91040004u?0x5a:0x56;}
static bool inventoryRead(destinia::InventoryPanel& panel,std::vector<std::array<uint8_t,20>>& fingerprints,std::vector<int>& indices,int& selected){
 if(!inventoryVisible())return false;auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");auto state=rd32(ui+0x7c);int bag=state==0x10000002u?-1:rd16(ui+(state==0x91040004u?0x58:0x54));if(bag>=4)return false;
 if(state==0x91040004u&&rd32(ui+0x2d8)!=g+0x433c)return false;
 std::map<int,int> equipmentList;if(state==0x10000002u){int count=rd16(ui+0x324);if(count>200)return false;for(int i=0;i<count;i++){int pool=rd16(ui+0x2e8+2*i);if(pool>=200||equipmentList.count(pool))return false;equipmentList[pool]=i;}}
 inventoryRefreshProtection();inventoryRefreshUpgrades();inventoryProtection.select(rd16(g+0x4886));panel.sale=state==0x91040004u;panel.items.clear();panel.names.clear();fingerprints.clear();indices.clear();selected=-1;int cursor=rd16(ui+inventoryCursorOffset(state));if(!inventoryNameBuffer)inventoryNameBuffer=hmalloc(640);if(!inventoryNameBuffer)return false;
 for(int b=0;b<4;b++){if(bag>=0&&bag!=b)continue;int count=rd16(g+0x433c+b*0x92);if(count>24)return false;for(int slot=0;slot<count;slot++){
  auto record=g+0x433e + b*0x92+6*slot;if(!rd16(record+4)||rd16(record)!=0)continue;int pool=rd16(record+2);if(pool>=200||uint8_t(rd16(g+0x2e0c+pool)&255)==0)return false;if(bag<0&&!equipmentList.count(pool))continue;
  std::array<uint8_t,20> fingerprint{};if(!rd(g+0x2ed6+20*pool,fingerprint.data(),fingerprint.size()))return false;int header=rd16(g+0x2ed6+20*pool),rank=(header>>8)&7;if(rank>4)return false;u32 level=0,price=0;
  if(!callFn(exportsMap.at("GameUI_getEquipMinLv"),{u32(header),1},&level)||!callFn(exportsMap.at("GameUI_getItemPrice"),{0,u32(pool)},&price)||!callFn(exportsMap.at("GameUI_getEquipName"),{inventoryNameBuffer,g+0x2ed6+20*pool}))return false;
  int nativeIndex=bag<0?equipmentList.at(pool):slot;panel.items.push_back({b,slot,pool,(header>>5)&7,rank,int(level),int(state==0x91040004u?price/5:price),inventoryProtection.has(pool,fingerprint),inventoryIsUpgrade(g+0x2ed6+20*pool)});panel.names.push_back(cstr(inventoryNameBuffer,512)+refinementHost.suffix(g+0x2ed6+20*pool));fingerprints.push_back(fingerprint);indices.push_back(nativeIndex);if(nativeIndex==cursor)selected=pool;
 }}
 panel.rebuild();return true;
}
static bool inventoryEnter(){if(!inventoryVisible())return false;try{auto panel=inventoryPanel;std::vector<std::array<uint8_t,20>> fingerprints;std::vector<int> indices;int selected=-1;if(!inventoryRead(panel,fingerprints,indices,selected))return false;if(inventoryContext==rd32(exportsMap.at("GameUI")+0x7c)&&inventoryPanel.selectedPool>=0&&std::none_of(panel.items.begin(),panel.items.end(),[&](const auto& item){return item.pool==inventoryPanel.selectedPool;}))selected=-1;panel.selectedPool=selected;panel.focus=7;panel.rebuild();inventoryPanel=std::move(panel);inventoryFingerprints=std::move(fingerprints);inventoryNativeIndices=std::move(indices);inventoryContext=rd32(exportsMap.at("GameUI")+0x7c);inventoryOpen=true;if(!inventoryProtectionReady)inventoryPanel.error="SIDECAR INVALIDO - VENDA BLOQUEADA";return true;}catch(const std::exception& e){hostReport("[inventario] Filtros indisponiveis: %s\n",e.what());return false;}}
static bool inventoryChoose(){auto selected=inventoryPanel.selected();if(!selected||!inventoryPanel.canChoose())return false;size_t index=inventoryPanel.view[*selected];if(index>=inventoryFingerprints.size())return false;auto target=inventoryPanel.items[index];
 try{if(!inventoryVisible()||rd32(exportsMap.at("GameUI")+0x7c)!=inventoryContext)throw std::runtime_error("CONTEXTO MUDOU - REABRA");destinia::InventoryPanel fresh;std::vector<std::array<uint8_t,20>> prints;std::vector<int> indices;int current=-1;if(!inventoryRead(fresh,prints,indices,current))throw std::runtime_error("INVENTARIO NAO VERIFICADO");size_t found=fresh.items.size();for(size_t i=0;i<fresh.items.size();i++)if(fresh.items[i].pool==target.pool)found=i;if(found==fresh.items.size()||!(fresh.items[found]==target)||prints[found]!=inventoryFingerprints[index])throw std::runtime_error("ITEM MUDOU - REABRA");
  if(inventoryContext==0x91040004u&&(!inventoryProtectionReady||fresh.items[found].protectedItem))throw std::runtime_error("ITEM PROTEGIDO OU SIDECAR INVALIDO");
  auto ui=exportsMap.at("GameUI"),g=exportsMap.at("Game");int offset=inventoryCursorOffset(inventoryContext),wanted=indices[found];for(int steps=0;rd16(ui+offset)!=wanted&&steps<200;steps++){int before=rd16(ui+offset);if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(before<wanted?7:5)})||rd32(ui+0x7c)!=inventoryContext||rd16(ui+offset)==before)throw std::runtime_error("NAVEGACAO INDISPONIVEL");}
  if(rd16(ui+offset)!=wanted)throw std::runtime_error("SELECAO NAO CONFIRMADA");
  if(inventoryContext==0x10000002u){if(rd16(ui+0x2e8+2*wanted)!=target.pool||!callFn(exportsMap.at("GameUI_drawMainMenu"),{rd32(exportsMap.at("Grp")+4)})||rd32(g+0x4584+4*rd16(ui+0x54))!=g+0x2ed6+20*target.pool)throw std::runtime_error("PREVIA NAO CONFIRMADA");}
  else{auto record=g+0x433e + target.bag*0x92+6*wanted;if(!rd16(record+4)||rd16(record+2)!=target.pool)throw std::runtime_error("ALVO NAO CONFIRMADO");}
  inventoryOpen=false;return true;
 }catch(const std::exception& e){inventoryPanel.error=e.what();return false;}}
static bool inventoryToggleProtection(){
 if(!inventoryProtectionReady){inventoryPanel.error="PROTECAO INDISPONIVEL";return false;}
 auto at=inventoryPanel.selected();if(!at||!inventoryPanel.canProtect())return false;size_t index=inventoryPanel.view[*at];
 try{
  if(!inventoryVisible()||rd32(exportsMap.at("GameUI")+0x7c)!=inventoryContext)throw std::runtime_error("CONTEXTO MUDOU - REABRA");
  destinia::InventoryPanel fresh;std::vector<std::array<uint8_t,20>> prints;std::vector<int> indices;int current=-1;
  if(!inventoryRead(fresh,prints,indices,current))throw std::runtime_error("INVENTARIO NAO VERIFICADO");
  const auto target=inventoryPanel.items.at(index);size_t found=fresh.items.size();
  for(size_t i=0;i<fresh.items.size();i++)if(fresh.items[i].pool==target.pool)found=i;
  if(!inventoryProtectionReady)throw std::runtime_error("SIDECAR INVALIDO - PROTECAO INDISPONIVEL");
  if(found==fresh.items.size()||!(fresh.items[found]==target)||fresh.items[found].protectedItem!=target.protectedItem||prints[found]!=inventoryFingerprints.at(index))throw std::runtime_error("ITEM MUDOU - REABRA");
  auto next=inventoryProtection;const bool marked=next.toggle(target.pool,prints[found]);
  destinia::saveInventoryProtection(fs::path(saveDir),next);inventoryProtection=std::move(next);inventoryPanel.items[index].protectedItem=marked;inventoryPanel.error.clear();return true;
 }catch(const std::exception& e){inventoryRefreshProtection();inventoryPanel.error=e.what();return false;}
}
// Resolve the CURRENT native cursor independently of any saved filter/query.
static bool inventoryToggleNativeProtection(){
 if(!inventoryVisible())return false;
 auto previous=inventoryPanel;auto oldPrints=inventoryFingerprints;auto oldIndices=inventoryNativeIndices;auto oldContext=inventoryContext;
 bool result=false;std::string message;
 try{destinia::InventoryPanel fresh;std::vector<std::array<uint8_t,20>> prints;std::vector<int> indices;int selected=-1;
  if(!inventoryRead(fresh,prints,indices,selected))throw std::runtime_error("INVENTARIO NAO VERIFICADO");
  fresh.selectedPool=selected;fresh.rebuild();inventoryPanel=std::move(fresh);inventoryFingerprints=std::move(prints);inventoryNativeIndices=std::move(indices);inventoryContext=rd32(exportsMap.at("GameUI")+0x7c);
  result=inventoryToggleProtection();message=result?(inventoryPanel.isProtected()?"ITEM PROTEGIDO":"ITEM DESPROTEGIDO"):inventoryPanel.error;
 }catch(const std::exception& e){message=e.what();}
 inventoryPanel=std::move(previous);inventoryFingerprints=std::move(oldPrints);inventoryNativeIndices=std::move(oldIndices);inventoryContext=oldContext;inventoryPanel.error=message;return result;
}
// Called at the native menu boundary, also covering keys generated by ARM touch.
// Only reads Game; returning to LR suppresses this confirmation, not navigation.
static bool inventoryNativeBlocked(int key){
 if(key!=6&&key!=17)return false;
 const auto ui=exportsMap.at("GameUI"),g=exportsMap.at("Game");const auto state=rd32(ui+0x7c);
 const bool sale=state==0x91040004u,dismantle=state==0x20030003u;if(!sale&&!dismantle)return false;
 if(!inventoryRefreshProtection())return true;if(inventoryProtection.empty())return false;
 inventoryProtection.select(rd16(g+0x4886));const int bag=rd16(ui+(sale?0x58:0x54)),slot=rd16(ui+(sale?0x5a:0x56));if(bag>=4)return true;
 const int count=rd16(g+0x433c+bag*0x92);if(count>24||slot>=count)return true;
 if(sale&&rd32(ui+0x2d8)!=g+0x433c)return true;
 const auto record=g+0x433e + bag*0x92+6*slot;
 if(!rd16(record+4)||rd16(record)!=0)return false;
 const int pool=rd16(record+2);if(pool>=200)return true;
 if(!(rd16(g+0x2e0c+pool)&255)){inventoryProtection.forget(pool);return false;}
 std::array<uint8_t,20> bytes{};if(!rd(g+0x2ed6+20*pool,bytes.data(),bytes.size()))return true;
 if(!inventoryProtection.has(pool,bytes))return false;
 inventoryPanel.error="ITEM PROTEGIDO - USE FILTROS";return true;
}
static void inventoryProtectionHook(uc_engine*,uint64_t,uint32_t,void* reset){
 try{
  if(reset){if(reinterpret_cast<uintptr_t>(reset)==1)inventoryProtection.select(int(R(0)));inventoryOpen=false;inventoryPanel.items.clear();inventoryPanel.view.clear();return;}
  if(!inventoryNativeBlocked(int(R(0))))return;
  u32 lr=0;uc_reg_read(uc,UC_ARM_REG_LR,&lr);uc_reg_write(uc,UC_ARM_REG_PC,&lr);
 }catch(const std::exception& e){hostReport("[inventario] Guarda falhou: %s\n",e.what());fatal=true;uc_emu_stop(uc);}
}
static void installInventoryProtection(){
 // Codex already checks the exact original library fingerprint/layout.
 if(!codexAvailable)return;
 {uc_hook hook;const auto at=exportsMap.at("_drawEquipIcon")&~1u;if(uc_hook_add(uc,&hook,UC_HOOK_CODE,(void*)inventoryIconHook,nullptr,at,at)!=UC_ERR_OK)throw std::runtime_error("FC: captura de icones indisponivel");}
 for(const char* name:{"MainMenu_mKeyPressed","GameCanvas_LoadGame","GameCanvas_init"}){
  uc_hook hook;const auto at=exportsMap.at(name)&~1u;
  if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(inventoryProtectionHook),std::string(name)=="MainMenu_mKeyPressed"?nullptr:reinterpret_cast<void*>(std::string(name)=="GameCanvas_LoadGame"?1:2),at,at)!=UC_ERR_OK)throw std::runtime_error("Falha ao instalar protecao de inventario.");
 }
 try{destinia::loadInventoryProtection(fs::path(saveDir),inventoryProtection);inventoryProtectionReady=true;}
 catch(const std::exception& e){inventoryPanel.error="SIDECAR INVALIDO - PROTECAO INDISPONIVEL";hostReport("[inventario] %s; sidecar preservado.\n",e.what());}
}
static void inventoryEvent(destinia::InventoryEvent event){if(event==destinia::InventoryEvent::Protect)inventoryToggleProtection();else if(event==destinia::InventoryEvent::Close)inventoryOpen=false;else if(event==destinia::InventoryEvent::Choose)inventoryChoose();}
static bool inventoryMenuKey(int key,bool down){
 if(key==10014){if(down){if(inventoryOpen)inventoryToggleProtection();else inventoryToggleNativeProtection();}return true;}
 if(key==10016){if(down&&inventoryVisible()){if(inventoryOpen)inventoryOpen=false;else inventoryEnter();}return true;}
 if(!down)return false;if(inventoryOpen){inventoryEvent(inventoryPanel.key(key,rd16(exportsMap.at("Game")+0x48fa)));return true;}if(key==12&&inventoryVisible())return inventoryEnter();return false;
}
static bool inventoryMenuTouch(int x,int y,int phase){if(inventoryTouchCaptured&&phase!=0){if(phase==1)inventoryTouchCaptured=false;return true;}if(!inventoryOpen&&!inventoryVisible())return false;int hit=destinia::inventoryHit(x,y,SCREEN_W,SCREEN_H,preferences.uiScale,inventoryOpen);if(inventoryOpen){if(phase==0){inventoryTouchCaptured=true;inventoryEvent(inventoryPanel.hit(hit,rd16(exportsMap.at("Game")+0x48fa)));}return true;}if(phase==0&&hit==20){inventoryTouchCaptured=inventoryEnter();return inventoryTouchCaptured;}return false;}
static void inventoryPaint(uint8_t* pixels){
 if(!inventoryOpen&&inventoryVisible()&&!inventoryIcons.empty()){
  inventoryRefreshUpgrades();const destinia::UiTransform t{SCREEN_W,SCREEN_H,preferences.uiScale,destinia::UiAnchor::Center,232,240};
  for(const auto& icon:inventoryIcons)if(inventoryIsUpgrade(icon.pointer))destinia::paintInventoryUpgradeDot(pixels,SCREEN_W,SCREEN_H,t,icon.x+20,icon.y+3);
 }
 destinia::paintInventory(pixels,SCREEN_W,SCREEN_H,preferences.uiScale,inventoryPanel,inventoryOpen,inventoryVisible());}
