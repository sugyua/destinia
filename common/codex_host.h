// Included by both hosts after their ARM helpers and ELF exports are declared.
#include "codex.h"
static destinia::Codex codex;
static bool codexAvailable=false,codexWritable=true,codexOpen=false;
static size_t codexPage=0;
static void codexChangePage(int direction) {
  auto it=codex.entries.find(codex.slot);const size_t pages=it==codex.entries.end()?1:std::max<size_t>(1,(it->second.size()+7)/8);
  codexPage=std::min(codexPage,pages-1);if(direction>0 && codexPage+1<pages)++codexPage;else if(direction<0 && codexPage)--codexPage;
}
static void codexFlush() {
  if(!codexAvailable || !codexWritable || !codex.dirty) return;
  try {codex.save(fs::path(saveDir)/"codex-v1.txt");}
  catch(const std::exception& e) {codexWritable=false;hostReport("[codex] AVISO: %s; contagem apenas nesta sessao.\n",e.what());}
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
    hostReport("[codex] Bestiario 0.7 ativo: F8 ou botao CODEX; arquivo codex-v1.txt.\n");
  }catch(const std::exception& e) {codexAvailable=false;hostReport("[codex] %s\n",e.what());}
}
