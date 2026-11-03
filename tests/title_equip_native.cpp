#include <unicorn/unicorn.h>
#include <cstdio>
#include <fstream>
static uc_err keepTitleEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepTitleEngine
#define main originalTitleMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
#define CHECK(e) do{if(!(e)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e);return 1;}}while(false)
static std::map<std::string,std::string> savedFiles(){
 std::map<std::string,std::string> result;
 for(const auto& item:fs::directory_iterator(fs::path(saveDir))){
  if(!item.is_regular_file())continue;
  std::ifstream in(item.path(),std::ios::binary);
  result[item.path().filename().string()]={std::istreambuf_iterator<char>(in),{}};
 }
 return result;
}
int main(int argc,char**argv){
 if(int r=originalTitleMain(argc,argv))return r;
 CHECK(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 const u32 g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");wr16(g+0x4854,16);wr16(g+0x4886,0);wr16(g+0x48fa,20);wr16(g+0x7574,65535);wr16(ui+0x76,16);wr32(ui+0x7c,2);for(int i=0;i<6;i++)wr32(g+0x4584+4*i,0);
 uint16_t stats[4]={20,15,10,10};CHECK(wr(g+0x48ba,stats,sizeof stats));wr16(g+0x48f0,0);CHECK(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 const unsigned critBefore=rd16(g+0x48d0);
 int before[4]={int(rd16(g+0x48c4)),int(rd16(g+0x48c6)),int(rd16(g+0x48c8)),int(rd16(g+0x48ca))};
 // Exercise the real ARM calculation for every added title, not only the
 // introductory one, and confirm repeated recalculation cannot stack buffs.
 auto originalStore=bossRushUI.titlesStore;
 for(const auto& def:destinia::allTitles()){
  if(def.id==destinia::TitleId::Nenhum)continue;
  auto& store=bossRushUI.titlesStore;store.unlock(0,def.id);store.setEquipped(0,def.id);store.setExclusive(0,true);
  wr16(g+0x48f4,32767);wr16(g+0x48f2,32767);
  CHECK(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
  const int pct[]={def.hpBonusPct,def.mpBonusPct,def.atkBonusPct,def.defBonusPct};
  for(int i=0;i<4;i++)CHECK(rd16(g+0x48c4+2*i)==unsigned(std::min(32767,before[i]+before[i]*pct[i]/100)));
  const unsigned expectedCrit=def.id==destinia::TitleId::MestreDaSobrevivencia?std::min(32767u,critBefore*220/100):(def.critChanceTenths?std::min(1000u,critBefore+unsigned(def.critChanceTenths)):critBefore);
  if(rd16(g+0x48d0)!=expectedCrit)std::fprintf(stderr,"CRIT id=%u before=%u got=%u expected=%u\n",unsigned(def.id),critBefore,rd16(g+0x48d0),expectedCrit);
  CHECK(rd16(g+0x48d0)==expectedCrit);
  CHECK(rd16(g+0x48f8)==rd16(g+0x48c4));CHECK(rd16(g+0x48f6)==rd16(g+0x48c6));
  CHECK(rd16(g+0x48f4)<=rd16(g+0x48f8));CHECK(rd16(g+0x48f2)<=rd16(g+0x48f6));
  std::vector<uint8_t> once(26),twice(26);CHECK(rd(g+0x48ba,once.data(),once.size()));
  CHECK(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));CHECK(rd(g+0x48ba,twice.data(),twice.size()));CHECK(once==twice);
 }
 bossRushUI.titlesStore=originalStore;CHECK(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));
 wr16(g+0x48f0,2);CHECK(callFn(exportsMap.at("GameCanvas_SaveGame"),{0}));CHECK(titleSelectExtra(destinia::TitleId::Iniciante));
 CHECK(rd16(g+0x48f0)==0);CHECK(bossRushUI.titlesStore.isExclusive(0));CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Iniciante);
 for(int i=0;i<4;i++){const int pct=i<2?5:2;CHECK(rd16(g+0x48c4+2*i)==unsigned(std::min(32767,before[i]+before[i]*pct/100)));}
 CHECK(rd16(g+0x48f8)==rd16(g+0x48c4));CHECK(rd16(g+0x48f6)==rd16(g+0x48c6));
 std::vector<uint8_t> first(26),second(26);CHECK(rd(g+0x48ba,first.data(),first.size()));CHECK(callFn(exportsMap.at("GameUI_calcHeroStat"),{}));CHECK(rd(g+0x48ba,second.data(),second.size()));CHECK(first==second);
 destinia::TitlesStore loaded;CHECK(loaded.load(fs::path(saveDir)));CHECK(loaded.isExclusive(0)&&loaded.getEquipped(0)==destinia::TitleId::Iniciante);
 CHECK(callFn(exportsMap.at("MMain_LoadData"),{}));CHECK(callFn(exportsMap.at("GameCanvas_LoadGame"),{0,0}));CHECK(rd16(g+0x48f0)==0);CHECK(bossRushUI.titlesStore.isExclusive(0));
 wr32(ui+0x7c,2);wr16(ui+0x56,2);wr32(g+0x7758+8,0);CHECK(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));CHECK(bossRushUI.titlesStore.isExclusive(0));CHECK(rd32(ui+0x7c)==2);
 wr16(ui+0x56,1);wr32(g+0x7758+4,1);CHECK(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));CHECK(rd16(g+0x48f0)==1);CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Nenhum);CHECK(!bossRushUI.titlesStore.isExclusive(0));CHECK(rd32(ui+0x7c)==1);
 destinia::TitlesStore again;CHECK(again.load(fs::path(saveDir)));CHECK(again.getEquipped(0)==destinia::TitleId::Nenhum&&!again.isExclusive(0));
 // The older boss menu must use the same exclusive selection as the native
 // title page, including clearing the original title and its bonuses.
 bossRushUI.currentSlot=0;bossRushUI.currentTab=4;bossRushUI.selectedIndex=1;bossRushUI.open=true;
 CHECK(bossRushUI.handleKey(17,true));CHECK(rd16(g+0x48f0)==0);CHECK(bossRushUI.titlesStore.isExclusive(0));
 CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Iniciante);
 wr16(g+0x48f4,rd16(g+0x48f8));wr16(g+0x48f2,rd16(g+0x48f6));
 bossRushUI.selectedIndex=0;CHECK(bossRushUI.handleKey(17,true));CHECK(!bossRushUI.titlesStore.isExclusive(0));
 CHECK(rd16(g+0x48f8)==rd16(g+0x48c4));CHECK(rd16(g+0x48f6)==rd16(g+0x48c6));
 CHECK(rd16(g+0x48f4)<=rd16(g+0x48f8));CHECK(rd16(g+0x48f2)<=rd16(g+0x48f6));
 CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Nenhum);bossRushUI.open=false;
 wr32(ui+0x7c,2);CHECK(titleSelectExtra(destinia::TitleId::Iniciante));
 const auto path=fs::path(saveDir)/"titles-v1.txt";{std::ofstream out(path,std::ios::binary|std::ios::trunc);out<<"DESTINIA_TITLES_V1\nequipped 1 1\nend\n";}
 const auto diskBefore=savedFiles();
 std::vector<uint8_t> snapshot(32056),after(32056);CHECK(rd(g,snapshot.data(),snapshot.size()));CHECK(!titleSelectExtra(destinia::TitleId::Iniciante));CHECK(rd(g,after.data(),after.size()));CHECK(snapshot==after);CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Iniciante);
 bossRushUI.open=true;bossRushUI.currentTab=4;bossRushUI.selectedIndex=0;CHECK(bossRushUI.handleKey(17,true));
 CHECK(bossRushUI.statusMessage.find("ERRO")!=std::string::npos);CHECK(rd(g,after.data(),after.size()));CHECK(snapshot==after);
 CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Iniciante);bossRushUI.open=false;
 wr32(ui+0x7c,2);wr16(ui+0x56,1);wr32(g+0x7758+4,1);CHECK(rd(g,snapshot.data(),snapshot.size()));
 CHECK(callFn(exportsMap.at("MainMenu_mKeyPressed"),{17}));CHECK(rd(g,after.data(),after.size()));CHECK(snapshot==after);CHECK(rd32(ui+0x7c)==2);CHECK(bossRushUI.titlesStore.getEquipped(0)==destinia::TitleId::Iniciante);CHECK(titleMenuMessage=="ERRO AO SALVAR TITULO");
 CHECK(savedFiles()==diskBefore);
 puts("TITLE_EQUIP_ARM_OK");return 0;
}
