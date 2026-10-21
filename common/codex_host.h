// Included by both hosts after their ARM helpers and ELF exports are declared.
#include "codex.h"
#include "codex_ui.h"
static destinia::Codex codex;
static bool codexAvailable=false,codexWritable=true,codexOpen=false;
static size_t codexPage=0;
static destinia::CodexPortraits codexPortraits;
static void codexEnsurePortraits() {
 auto character=codex.entries.find(codex.slot);if(character==codex.entries.end())return;
 size_t row=0;for(const auto& entry:character->second) {
  if(row++<codexPage*destinia::codexRowsPerPage || row> (codexPage+1)*destinia::codexRowsPerPage)continue;
  const int id=entry.first;if(codexPortraits.count(id))continue;
  destinia::CodexPortrait picture;auto paths=destinia::codexPortraitAssets(id);std::vector<uint8_t> sheet,pose;
  auto bounded=[&](const std::string& name){std::error_code ec;auto size=fs::file_size(assetsDir+"/"+name,ec);return !ec && size>0 && size<=2*1024*1024;};
  if(!paths.first.empty() && bounded(paths.first) && bounded(paths.second) && readFile(assetsDir+"/"+paths.first,sheet) && readFile(assetsDir+"/"+paths.second,pose))picture=destinia::codexDecodePortrait(destinia::codexUnpackResource(sheet),destinia::codexUnpackResource(pose));
  if(picture.pixels.empty())hostReport("[codex] Imagem indisponivel para especie %d; nome e contador preservados.\n",id);
  codexPortraits.emplace(id,std::move(picture));
 }
}
static void codexChangePage(int direction) {
  auto it=codex.entries.find(codex.slot);const size_t pages=it==codex.entries.end()?1:std::max<size_t>(1,(it->second.size()+destinia::codexRowsPerPage-1)/destinia::codexRowsPerPage);
  codexPage=std::min(codexPage,pages-1);if(direction>0 && codexPage+1<pages)++codexPage;else if(direction<0 && codexPage)--codexPage;codexEnsurePortraits();
}
static void codexFlush() {
  if(!codexAvailable || !codexWritable || !codex.dirty) return;
  try {codex.save(fs::path(saveDir)/"codex-v1.txt");}
  catch(const std::exception& e) {codexWritable=false;hostReport("[codex] AVISO: %s; contagem apenas nesta sessao.\n",e.what());}
}
// Host-owned extension of the original root menu. Never writes a new ARM tab ID.
static bool codexTabSelected=false,codexTouchCaptured=false,codexNativeTouchDown=false;
static int codexNativeTouchX=0,codexNativeTouchY=0;
static bool codexMenuVisible() {
 if(!codexAvailable || !exportsMap.count("Game") || !exportsMap.count("GameUI"))return false;
 const auto ui=exportsMap.at("GameUI");const auto state=rd32(ui+0x7c);
 return rd16(exportsMap.at("Game")+0x4854)==16 && rd16(ui+0x76)>=2 && (state&0x0fffffff)==0 && (state>>28)<=8;
}
static void codexEnterMenu() {
 if(codexNativeTouchDown) {
  callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniTouch"),{ENV_OBJ,0x10,(u32)codexNativeTouchX,(u32)codexNativeTouchY,1});
  codexNativeTouchDown=false;
 }
 const int slot=rd16(exportsMap.at("Game")+0x4886);if(slot<=9)codex.select(slot);
 codexOpen=true;codexTabSelected=true;codexPage=0;codexEnsurePortraits();codexFlush();
}
static bool codexMenuKey(int key,bool down) {
 if(!down)return false; // Always allow native releases to clear held commands.
 if(codexOpen) {
  if(key==10 || key==27){codexOpen=false;codexTabSelected=true;codexFlush();}
  else if(key==5 || key==3)codexChangePage(-1);
  else if(key==7 || key==9)codexChangePage(1);
  return true;
 }
 if(!codexMenuVisible()){codexTabSelected=false;return false;}
 auto ui=exportsMap.at("GameUI");const auto state=rd32(ui+0x7c);
 if(codexTabSelected) {
  if(key==17 || key==6 || key==9){codexEnterMenu();return true;}
  if(key==5 || key==7){codexTabSelected=false;wr32(ui+0x7c,key==5?0x80000000u:0u);return true;}
  if(key==10 || key==27){codexTabSelected=false;return false;}
  return true;
 }
 if((key==5 && state==0) || (key==7 && state==0x80000000u)){codexTabSelected=true;return true;}
 return false;
}
static bool codexMenuTouch(int x,int y,int phase) {
 if(codexTouchCaptured && phase!=0){if(phase==1)codexTouchCaptured=false;return true;}
 if(codexOpen) {
  if(phase==0) {
   codexTouchCaptured=true;const auto panel=destinia::codexPanel(SCREEN_W,SCREEN_H);
   if(destinia::CodexRect{panel.x+298,panel.y+4,58,20}.contains(x,y)){codexOpen=false;codexTabSelected=true;codexFlush();}
   else if(destinia::CodexRect{panel.x+4,panel.y+185,90,27}.contains(x,y))codexChangePage(-1);
   else if(destinia::CodexRect{panel.x+280,panel.y+185,76,27}.contains(x,y))codexChangePage(1);
  }
  return true;
 }
 if(phase==0 && codexMenuVisible() && destinia::codexMenuButton(SCREEN_W,SCREEN_H).contains(x,y)) {
  codexEnterMenu();codexTouchCaptured=true;return true;
 }
 if(phase==0){codexTabSelected=false;codexNativeTouchDown=true;}
 if(phase==1)codexNativeTouchDown=false;
 if(phase==0 || phase==2){codexNativeTouchX=x;codexNativeTouchY=y;}
 return false;
}
static void codexFileRemoved(const std::string& path) {
  const auto name=fs::path(path).filename().string();
  if(codexAvailable && name.size()==9 && name.compare(0,4,"save")==0 && name[4]>='0' && name[4]<='9' && name.substr(5)==".dat") {codex.erase(name[4]-'0');codexFlush();}
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
    if(kind==2) {if(state==2 && hp<=0) codex.observe(p,species,true);return;}
    if(!rd(p+0x1a,&x,2) || !rd(p+0x1c,&y,2) || !rd(game+0x4878,&cx,2) || !rd(game+0x487a,&cy,2)) return;
    if(x-cx>=0 && x-cx<SCREEN_W && y-cy>=0 && y-cy<SCREEN_H) codex.observe(p,species,false);
  }catch(...) {codexAvailable=false;hostReport("[codex] Observador desativado por leitura incompatível.\n");}
}
static void installCodex() {
  std::vector<uint8_t> library,table;
  if(!readFile(soPath,library)) return;
  uint64_t fingerprint=14695981039346656037ull;for(auto b:library) fingerprint=(fingerprint^b)*1099511628211ull;
  if(fingerprint!=0x74b2547839b88777ull || !exportsMap.count("Game")) {hostReport("[codex] Biblioteca diferente da original verificada; observador desativado.\n");return;}
  try {
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
    hostReport("[codex] Bestiario 0.7 ativo: CODEX no menu original (F8 no menu); arquivo codex-v1.txt.\n");
  }catch(const std::exception& e) {codexAvailable=false;hostReport("[codex] %s\n",e.what());}
}
