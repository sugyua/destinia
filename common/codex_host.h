// Included by both hosts after their ARM helpers and ELF exports are declared.
#include "codex.h"
#include "codex_ui.h"
#include "codex_native_layout.h"
#include "loot_economy_host.h"
static destinia::Codex codex;
static bool codexAvailable=false,codexWritable=true,codexOpen=false;
static size_t codexPage=0,codexSelection=0;
static bool codexDetailsOpen=false,codexMetadataAttempted=false,codexMetadataReady=false;
static size_t codexDetailPage=0,codexDetailScroll=0;
static destinia::CodexMetadata codexMetadata;
static destinia::CodexInfoView codexDetailInfo;
// Runtime facts are session-only; codex-v1.txt and original saves stay compatible.
static std::map<int,std::map<int,destinia::CodexMonsterFacts>> codexFacts;
static void codexChangeDetailPage(int direction){
 const auto count=codexDetailInfo.sections.size();if(!count){codexDetailPage=codexDetailScroll=0;return;}
 const auto next=static_cast<long long>(codexDetailPage)+direction;
 codexDetailPage=size_t(std::max<long long>(0,std::min<long long>(next,count-1)));codexDetailScroll=0;
}
static void codexScrollDetails(int direction){
 if(codexDetailPage>=codexDetailInfo.sections.size()){codexDetailScroll=0;return;}
 const auto count=destinia::codexDetailPages(codexDetailInfo.sections[codexDetailPage]).size();
 const auto next=static_cast<long long>(codexDetailScroll)+direction;
 codexDetailScroll=size_t(std::max<long long>(0,std::min<long long>(next,count-1)));
}
static void codexEnterDetails(){
 codexDetailsOpen=true;codexDetailPage=codexDetailScroll=0;codexDetailInfo={};
 if(codexSelection>=codex.names.size())return;
 auto name=codex.names.begin();std::advance(name,codexSelection);const int species=name->first;
 const auto character=codex.entries.find(codex.slot);if(character==codex.entries.end() || !character->second.count(species))return;
 if(!codexMetadataAttempted){
  codexMetadataAttempted=true;
  try{codexMetadata.load(fs::path(assetsDir));codexMetadataReady=true;}
  catch(const std::exception& e){hostReport("[codex] Metadados indisponiveis: %s; grade e contadores preservados.\n",e.what());}
 }
 if(codexMetadataReady){
  const auto records=codexFacts.find(codex.slot);const destinia::CodexMonsterFacts* facts=nullptr;
  if(records!=codexFacts.end()){const auto found=records->second.find(species);if(found!=records->second.end())facts=&found->second;}
  codexDetailInfo=codexMetadata.view(species,facts);
 }else{
  codexDetailInfo.hp="NAO VERIFICADO";codexDetailInfo.notes="DADOS ORIGINAIS INDISPONIVEIS";
  for(const auto& section:std::vector<std::pair<std::string,std::string>>{{"drops","DROPS"},{"spawns","LOCAIS DE SPAWN"},{"variants","VARIANTES"}})codexDetailInfo.sections.push_back({section.first,section.second,{{"","",{{"DADOS ORIGINAIS INDISPONIVEIS",0xffff}}}}});
 }
}
static void codexObserveFacts(u32 monster,u32 game,int species){
 if(!codex.names.count(species))return;
 int32_t hp=0;uint16_t level=0;u32 elite=0;
 if(!rd(monster+0x24,&hp,4) || !rd(monster+0x10,&level,2) || !rd(monster+0x4c,&elite,4) || hp<=0 || level==0 || level>999)return;
 auto& facts=codexFacts[codex.slot][species];facts.maxHP=hp;facts.level=level;facts.elite=elite!=0;
 uint16_t area=0,map=0;
 if(rd(game+0x4870,&area,2) && rd(game+0x4872,&map,2) && area<=2 && map<=255 && facts.locations.size()<256)facts.locations.insert("m"+std::to_string(area)+"_"+std::to_string(map)+".mpd");
}
static destinia::CodexPortraits codexPortraits;
static void codexEnsurePortraits() {
 auto character=codex.entries.find(codex.slot);if(character==codex.entries.end())return;
 size_t row=0;for(const auto& entry:codex.names) {
  const size_t index=row++;if(index<codexPage*destinia::codexEntriesPerPage || index>=(codexPage+1)*destinia::codexEntriesPerPage || !character->second.count(entry.first))continue;
  const int id=entry.first;if(codexPortraits.count(id))continue;
  destinia::CodexPortrait picture;auto paths=destinia::codexPortraitAssets(id);std::vector<uint8_t> sheet,pose;
  auto bounded=[&](const std::string& name){std::error_code ec;auto size=fs::file_size(assetsDir+"/"+name,ec);return !ec && size>0 && size<=2*1024*1024;};
  if(!paths.first.empty() && bounded(paths.first) && bounded(paths.second) && readFile(assetsDir+"/"+paths.first,sheet) && readFile(assetsDir+"/"+paths.second,pose))picture=destinia::codexDecodePortrait(destinia::codexUnpackResource(sheet),destinia::codexUnpackResource(pose),id>=100?0:1);
  if(picture.pixels.empty())hostReport("[codex] Imagem indisponivel para especie %d; nome e contador preservados.\n",id);
  codexPortraits.emplace(id,std::move(picture));
 }
}
static void codexMoveSelection(int delta) {
 if(codex.names.empty()){codexSelection=codexPage=0;return;}
 const auto next=static_cast<long long>(codexSelection)+delta;
 codexSelection=size_t(std::max<long long>(0,std::min<long long>(next,codex.names.size()-1)));
 codexPage=codexSelection/destinia::codexEntriesPerPage;codexEnsurePortraits();
}
static void codexChangePage(int direction) {
 if(codexDetailsOpen){codexChangeDetailPage(direction);return;}
 if(codex.names.empty()){codexSelection=codexPage=0;return;}
 const size_t pages=(codex.names.size()+destinia::codexEntriesPerPage-1)/destinia::codexEntriesPerPage;
 codexPage=std::min(codexPage,pages-1);if(direction>0 && codexPage+1<pages)++codexPage;else if(direction<0 && codexPage)--codexPage;
 const size_t start=codexPage*destinia::codexEntriesPerPage,last=std::min(destinia::codexEntriesPerPage,codex.names.size()-start)-1;
 size_t offset=codexSelection%destinia::codexEntriesPerPage;
 if(offset>last){const size_t col=offset%destinia::codexGridColumns;offset=col<=last?last-(last-col)%destinia::codexGridColumns:last;}
 codexSelection=start+offset;codexEnsurePortraits();
}
static void codexFlush() {
  if(!codexAvailable || !codexWritable || !codex.dirty) return;
  try {codex.save(fs::path(saveDir)/"codex-v1.txt");}
  catch(const std::exception& e) {codexWritable=false;hostReport("[codex] AVISO: %s; contagem apenas nesta sessao.\n",e.what());}
}
// Host-owned extension of the original root menu. Never writes a new ARM tab ID.
static bool codexTabSelected=false,codexTouchCaptured=false,codexNativeTouchDown=false;
static int codexNativeMenuPending=-1,codexNativeMenuBypass=0;
static bool codexNativeMenuDraining=false;
static u32 codexNativeMenuTarget=0;
struct CodexNativeMenuGuard{CodexNativeMenuGuard(){++codexNativeMenuBypass;}~CodexNativeMenuGuard(){--codexNativeMenuBypass;}};

static int codexNativeTouchX=0,codexNativeTouchY=0;
static bool codexMenuVisible() {
 if(!codexAvailable || !exportsMap.count("Game") || !exportsMap.count("GameUI"))return false;
 const auto ui=exportsMap.at("GameUI");const auto state=rd32(ui+0x7c);
 return rd16(exportsMap.at("Game")+0x4854)==16 && rd16(ui+0x76)>=2 && (state&0x0fffffff)<=1 && (state>>28)<=8;
}
static bool codexSelectNativeTab(int tab){
 CodexNativeMenuGuard bypass;
 const auto ui=exportsMap.at("GameUI");
 if((rd32(ui+0x7c)&0x0fffffff)==1){if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{27}))return false;}
 if((rd32(ui+0x7c)&0x0fffffff)!=0)return false;
 for(int n=0;int(rd32(ui+0x7c)>>28)!=tab&&n<9;++n)if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{7}))return false;
 return int(rd32(ui+0x7c)>>28)==tab;
}
static void codexEnterMenu() {
 if(codexNativeTouchDown) {
  callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniTouch"),{ENV_OBJ,0x10,(u32)codexNativeTouchX,(u32)codexNativeTouchY,1});
  codexNativeTouchDown=false;
 }
 const int slot=rd16(exportsMap.at("Game")+0x4886);if(slot<=9)codex.select(slot);
 codexOpen=true;codexTabSelected=true;codexDetailsOpen=false;codexDetailPage=codexDetailScroll=0;codexSelection=0;
 auto records=codex.entries.find(codex.slot);if(records!=codex.entries.end()){size_t index=0;for(const auto& name:codex.names){if(records->second.count(name.first)){codexSelection=index;break;}++index;}}
 codexPage=codexSelection/destinia::codexEntriesPerPage;codexEnsurePortraits();codexFlush();
}
static bool codexMenuKey(int key,bool down) {
 if(!down)return false; // Always allow native releases to clear held commands.
 if(key>=13&&key<=16){const int directions[]={3,9,5,7};key=directions[key-13];}
 if(codexOpen) {
  if(codexDetailsOpen){
   if(key==10 || key==27)codexDetailsOpen=false;
   else if(key==5)codexChangeDetailPage(-1);
   else if(key==7 || key==17 || key==6)codexChangeDetailPage(1);
   else if(key==3)codexScrollDetails(-1);
   else if(key==9)codexScrollDetails(1);
   return true;
  }
  if(key==17 || key==6){codexEnterDetails();return true;}
  if(key==10 || key==27){codexOpen=false;codexTabSelected=true;codexFlush();}
  else if(key==5)codexMoveSelection(-1);
  else if(key==7)codexMoveSelection(1);
  else if(key==3)codexMoveSelection(-int(destinia::codexGridColumns));
  else if(key==9)codexMoveSelection(int(destinia::codexGridColumns));
  return true;
 }
 if(!codexMenuVisible()){codexTabSelected=false;return false;}
 auto ui=exportsMap.at("GameUI");const auto state=rd32(ui+0x7c);
 if(codexTabSelected) {
  if(key==17 || key==6 || key==9){codexEnterMenu();return true;}
  if(key==5 || key==7){codexTabSelected=false;codexSelectNativeTab(key==5?8:0);return true;}
  if(key==10 || key==27){codexTabSelected=false;return false;}
  return true;
 }
 // System body has no horizontal choice: return through native Cancel before
 // selecting the added tab. Never extend this to deeper settings/popups.
 if((key==5 && state==0) || (key==7 && (state==0x80000000u || state==0x80000001u))){
  if(state==0x80000001u && !codexSelectNativeTab(8))return false;
  codexEnterMenu();codexOpen=false;return true;
 }
 return false;
}
static bool codexMenuTouch(int x,int y,int phase) {
 const auto point=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,preferences.uiScale).inverse(x,y);x=point.first;y=point.second;
 if(codexTouchCaptured && phase!=0){if(phase==1)codexTouchCaptured=false;return true;}
 if(codexOpen&&codexNativeTouchDown&&phase!=0){if(phase==1)codexNativeTouchDown=false;codexNativeTouchX=x;codexNativeTouchY=y;return false;}
 if(phase==0 && (codexOpen || codexMenuVisible())) {const int tab=destinia::nativeCodexTab(x,y,SCREEN_W,SCREEN_H);if(tab>=0){codexTouchCaptured=true;if(tab==9)codexEnterMenu();else{codexOpen=codexDetailsOpen=codexTabSelected=false;codexFlush();codexSelectNativeTab(tab);}return true;}}
 if(phase==0 && !codexOpen && codexTabSelected && codexMenuVisible() && destinia::nativeCodexRect(84,25,232,215,SCREEN_W,SCREEN_H).contains(x,y))codexOpen=true;
 if(codexOpen&&(x<(SCREEN_W-400)/2+84||x>=(SCREEN_W-400)/2+339||y<(SCREEN_H-240)/2||y>=(SCREEN_H-240)/2+240)){
  if(phase==0)codexNativeTouchDown=true;if(phase==1)codexNativeTouchDown=false;
  codexNativeTouchX=x;codexNativeTouchY=y;return false;
 }
 if(codexOpen) {
  if(phase==0) {
   codexTouchCaptured=true;
   if(codexDetailsOpen){
    if(destinia::nativeCodexBack(SCREEN_W,SCREEN_H).contains(x,y))codexDetailsOpen=false;
    else if(destinia::nativeCodexPrevious(SCREEN_W,SCREEN_H).contains(x,y))codexChangeDetailPage(-1);
    else if(destinia::nativeCodexNext(SCREEN_W,SCREEN_H).contains(x,y))codexChangeDetailPage(1);
    else if(destinia::nativeCodexUp(SCREEN_W,SCREEN_H).contains(x,y))codexScrollDetails(-1);
    else if(destinia::nativeCodexDown(SCREEN_W,SCREEN_H).contains(x,y))codexScrollDetails(1);
    return true;
   }
   if(destinia::nativeCodexBack(SCREEN_W,SCREEN_H).contains(x,y)){codexOpen=false;codexTabSelected=true;codexFlush();}
   else if(destinia::nativeCodexPrevious(SCREEN_W,SCREEN_H).contains(x,y))codexChangePage(-1);
   else if(destinia::nativeCodexNext(SCREEN_W,SCREEN_H).contains(x,y))codexChangePage(1);
   else if(destinia::nativeCodexInfo(SCREEN_W,SCREEN_H).contains(x,y))codexEnterDetails();
   else {const int index=destinia::nativeCodexHit(x,y,codex.names.size(),codexPage,SCREEN_W,SCREEN_H);if(index>=0)codexSelection=size_t(index);}
  }
  return true;
 }
 if(phase==0){if(!codexMenuVisible())codexTabSelected=false;codexNativeTouchDown=true;}
 if(phase==1)codexNativeTouchDown=false;
 if(phase==0 || phase==2){codexNativeTouchX=x;codexNativeTouchY=y;}
 return false;
}
static void codexFileRemoved(const std::string& path) {
  const auto name=fs::path(path).filename().string();
  if(codexAvailable && name.size()==9 && name.compare(0,4,"save")==0 && name[4]>='0' && name[4]<='9' && name.substr(5)==".dat") {codex.erase(name[4]-'0');codexFacts.erase(name[4]-'0');codexFlush();}
}
static void codexHook(uc_engine*,uint64_t,uint32_t,void* context) {
  if(!codexAvailable) return;
  try {
    auto kind=reinterpret_cast<uintptr_t>(context);
    if(kind==4) {codex.select(int(R(0)));codex.newMap();return;}
    auto game=exportsMap.at("Game");uint16_t slot=0;
    if(!rd(game+0x4886,&slot,2) || slot>9) return;
    codex.select(slot);auto p=R(0);
    if(kind==3) {codex.newMap();return;}
    if(kind==0) {codex.respawn(p);return;}
    uint16_t species=0,state=0;int32_t hp=0;int16_t x=0,y=0,cx=0,cy=0;
    if(!rd(p+8,&species,2) || !rd(p+0x12,&state,2) || !rd(p+0x20,&hp,4)) return;
    // Normal procMonster notifies quests before it writes death state 2.
    if(kind==2) {if(hp<=0){codexObserveFacts(p,game,species);codex.observe(p,species,true);if(species>=100&&species<=109)destinia::lootEconomyOnBossKilled(saveDir,slot,species);}return;}
    if(!rd(p+0x1a,&x,2) || !rd(p+0x1c,&y,2) || !rd(game+0x4878,&cx,2) || !rd(game+0x487a,&cy,2)) return;
    if(x-cx>=0 && x-cx<SCREEN_W && y-cy>=0 && y-cy<SCREEN_H){codexObserveFacts(p,game,species);codex.observe(p,species,false);}
  }catch(...) {codexAvailable=false;hostReport("[codex] Observador desativado por leitura incompatível.\n");}
}
// ARM callback only defers a command; never call uc_emu_start while it runs.
static void codexNativeMenuHook(uc_engine*,uint64_t address,uint32_t,void*){
 try{
  if(!codexAvailable||codexNativeMenuBypass||codexNativeMenuDraining||!codexMenuVisible())return;
  const int raw=int(R(0)),directions[]={3,9,5,7},key=raw>=13&&raw<=16?directions[raw-13]:raw;const auto state=rd32(exportsMap.at("GameUI")+0x7c);
  if(!(codexOpen||codexTabSelected||(key==5&&state==0)||(key==7&&(state==0x80000000u||state==0x80000001u))))return;
  if(codexNativeMenuPending<0){codexNativeMenuPending=raw;codexNativeMenuTarget=u32(address)|1u;}
  u32 lr=0;uc_reg_read(uc,UC_ARM_REG_LR,&lr);uc_reg_write(uc,UC_ARM_REG_PC,&lr);
 }catch(const std::exception& e){hostReport("[codex] Entrada nativa falhou: %s\n",e.what());fatal=true;uc_emu_stop(uc);}
}
static bool codexDrainNativeMenu(){
 if(codexNativeMenuPending<0||codexNativeMenuDraining)return true;
 const int key=codexNativeMenuPending;const u32 target=codexNativeMenuTarget;codexNativeMenuPending=-1;codexNativeMenuDraining=true;
 bool ok=true;
 try{if(!codexMenuKey(key,true)){CodexNativeMenuGuard bypass;ok=callFn(target,{u32(key)});}}
 catch(...){codexNativeMenuDraining=false;throw;}
 codexNativeMenuDraining=false;return ok;
}
// Suppress only the original top-bar hand. The painter relocates the same sprite.
// Body cursors and off-screen sprite extraction keep their original ARM path.
static void codexTopHandHook(uc_engine*,uint64_t,uint32_t,void*){
 const auto oy=(SCREEN_H-240)/2;u32 lr=0;uc_reg_read(uc,UC_ARM_REG_LR,&lr);
 if(R(3)==exportsMap.at("MMain")+12&&int32_t(R(2))>=oy&&int32_t(R(2))<oy+40&&
    (lr&~1u)>=(exportsMap.at("GameUI_drawMainMenu")&~1u)&&(lr&~1u)<(exportsMap.at("GameUI_drawMainMenu")&~1u)+8400)
  uc_reg_write(uc,UC_ARM_REG_PC,&lr);
}
static void installCodex() {
  std::vector<uint8_t> library,table;
  if(!readFile(soPath,library)) return;
  uint64_t fingerprint=14695981039346656037ull;for(auto b:library) fingerprint=(fingerprint^b)*1099511628211ull;
  if(fingerprint!=0x74b2547839b88777ull || !exportsMap.count("Game")) {hostReport("[codex] Biblioteca diferente da original verificada; observador desativado.\n");return;}
  try {
    {uc_hook hook;const auto at=exportsMap.at("Grp_drawWImage")&~1u;if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(codexTopHandHook),nullptr,at,at)!=UC_ERR_OK)throw std::runtime_error("Falha ao alinhar cursor da barra.");}
    if(!readFile(assetsDir+"/mon_data_en.gdt",table)) throw std::runtime_error("Tabela de monstros ausente.");
    codex.readNames(table);
    try {codex.load(fs::path(saveDir)/"codex-v1.txt");}
    catch(const std::exception& e) {codexWritable=false;hostReport("[codex] AVISO: %s; arquivo original preservado.\n",e.what());}
    const char* symbols[]={"Mon_initMonster","drawMonsterOddEff","checkQuestMonster","Mon_createMonster","GameCanvas_LoadGame"};
    codexAvailable=true;
    for(uintptr_t i=0;i<5;++i) {
      uc_hook hook;auto at=exportsMap.at(symbols[i])&~1u;
      if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(codexHook),reinterpret_cast<void*>(i),at,at)!=UC_ERR_OK) throw std::runtime_error("Falha ao observar monstros.");
    }
    for(const char* name:{"GameCanvas_mKeyPressed","MainMenu_mKeyPressed"}){uc_hook hook;const auto at=exportsMap.at(name)&~1u;
     if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(codexNativeMenuHook),nullptr,at,at)!=UC_ERR_OK)throw std::runtime_error("Falha ao observar entrada do menu.");}
    hostReport("[codex] Bestiario 0.7 ativo: CODEX na barra do menu original; arquivo codex-v1.txt.\n");
  }catch(const std::exception& e) {codexAvailable=false;hostReport("[codex] %s\n",e.what());}
}
