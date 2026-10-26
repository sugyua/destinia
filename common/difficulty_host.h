#pragma once
// Included after native menu adapters. All disk writes happen outside ARM hooks.
#include "difficulty.h"
#include "titles.h"
static destinia::DifficultyProgress difficultyProgress;
static bool difficultyReady=false,difficultyOpen=false,difficultyButton=false,difficultyTouchHeld=false;
static int difficultySelected=0,difficultyRequest=-1,difficultyRequestSlot=-1;
static std::set<std::pair<unsigned,unsigned>> difficultyKills;
static std::unique_ptr<destinia::FileLock> difficultyRemovalLock;
static int difficultyRetry=0;
static std::set<unsigned> difficultyRemovingSlots;
static std::string difficultyStatus;
static u32 difficultyString=0;
static std::set<std::pair<unsigned,u32>> difficultyDeaths;
static void difficultyNewMap(){difficultyDeaths.clear();}
static void difficultyRespawn(u32 p){for(auto i=difficultyDeaths.begin();i!=difficultyDeaths.end();)if(i->second==p)i=difficultyDeaths.erase(i);else ++i;}
static bool difficultySystem(){return difficultyReady&&!codexOpen&&!codexTabSelected&&codexMenuVisible()&&(rd32(exportsMap.at("GameUI")+0x7c)>>28)==8;}
static unsigned difficultySlot(){return rd16(exportsMap.at("Game")+0x4886);}
static void difficultyBossDefeated(unsigned slot,int species,u32 p){
 if(difficultyReady&&slot<10&&species==108&&difficultyDeaths.emplace(slot,p).second){
  const auto tier=difficultyArm.deathDifficulty(p);
  const unsigned deadTier=tier?*tier:difficultyProgress.current(slot);
  difficultyKills.emplace(slot,deadTier);
  if(deadTier>=4){
   try{
    destinia::TitlesStore ts;
    ts.load(fs::path(saveDir));
    ts.unlock(slot,destinia::TitleId::SenhorDoAbismo);
    ts.save(fs::path(saveDir));
   }catch(...){}
  }
 }
}
static int difficultySaveSlot(const std::string& path){const auto n=fs::path(path).filename().string();return n.size()==9&&n.substr(0,4)=="save"&&n[4]>='0'&&n[4]<='9'&&n.substr(5)==".dat"?n[4]-'0':-1;}
// Reset extended progress in the very transaction deleting the original slot.
static void difficultyPrepareRemoval(const std::string& path){
 const int slot=difficultySaveSlot(path);if(slot<0)return;
 if(!difficultyReady)throw std::runtime_error("difficulty unavailable; deletion refused to preserve slot identity");
 if(!difficultyRemovalLock){difficultyRemovalLock=std::make_unique<destinia::FileLock>();if(!difficultyRemovalLock->acquire(fs::path(saveDir)/"progression-v1.lock")){difficultyRemovalLock.reset();throw std::runtime_error("progression busy");}}
 auto next=destinia::loadDifficulty(fs::path(saveDir),difficultyProgress.book.mode);difficultyRemovingSlots.insert(unsigned(slot));for(auto removed:difficultyRemovingSlots)next.erase(removed);destinia::stageDifficulty(saveTransaction,fs::path(saveDir),next);
}
static void difficultyEndCall(){difficultyRemovalLock.reset();difficultyRemovingSlots.clear();}
static void difficultyFileRemoved(const std::string& path){const int slot=difficultySaveSlot(path);if(slot<0||!difficultyReady)return;difficultyProgress=destinia::loadDifficulty(fs::path(saveDir),difficultyProgress.book.mode);for(auto i=difficultyKills.begin();i!=difficultyKills.end();)if(i->first==unsigned(slot))i=difficultyKills.erase(i);else ++i;difficultyRemovalLock.reset();}
static void difficultyFlush(){
 if(!difficultyReady)return;if(difficultyRetry>0){--difficultyRetry;return;}
 if(difficultyKills.empty()&&difficultyRequest<0)return;
 try{auto next=destinia::loadDifficulty(fs::path(saveDir),difficultyProgress.book.mode);bool changed=false;
  for(const auto& kill:difficultyKills)changed=next.defeatedAt(kill.first,kill.second)||changed;
  if(difficultyRequest>=0){const auto slot=difficultySlot();if(int(slot)!=difficultyRequestSlot||slot>=10||!next.change(slot,unsigned(difficultyRequest)))throw std::runtime_error("DIFICULDADE BLOQUEADA");changed=true;}
  if(changed){const auto slot=difficultySlot();if(slot<10)difficultyArm.validateProfile(slot,destinia::progression::multipliers(destinia::progression::Difficulty(next.current(slot)),0));destinia::saveDifficulty(fs::path(saveDir),next);}
  difficultyProgress=std::move(next);difficultyArm.refresh();difficultyStatus="DIFICULDADE SALVA";difficultyKills.clear();
 }catch(const std::exception& e){difficultyStatus="FALHA AO SALVAR - TENTE NOVAMENTE";difficultyRetry=60;hostReport("[dificuldade] %s\n",e.what());}
 difficultyRequest=-1;
}
static void difficultyEnter(){if(!difficultySystem()||difficultySlot()>=10)return;difficultyOpen=true;difficultyButton=true;difficultySelected=int(difficultyProgress.current(difficultySlot()));difficultyStatus.clear();}
static bool difficultyMenuKey(int key,bool down){
 if(!difficultySystem()){difficultyOpen=difficultyButton=false;return false;}
 if(key>=13&&key<=16){const int keys[]={3,9,5,7};key=keys[key-13];}
 if(!down)return difficultyOpen||difficultyButton;
 if(difficultyOpen){
  if(key==10||key==27){difficultyOpen=false;difficultyStatus.clear();}
  else if(key==3||key==9){difficultySelected=(difficultySelected+(key==3?4:1))%5;difficultyStatus.clear();}
  else if(key==6||key==17){if(difficultyProgress.allowed(difficultySlot(),unsigned(difficultySelected))){difficultyRequest=difficultySelected;difficultyRequestSlot=int(difficultySlot());difficultyStatus="SALVANDO...";}else difficultyStatus="BLOQUEADA: DERROTE DIABLO";}
  return true;
 }
 const auto ui=exportsMap.at("GameUI");const auto state=rd32(ui+0x7c);
 if(difficultyButton){
  if(key==6||key==17||key==9){difficultyEnter();return true;}
  difficultyButton=false;if(key==3)return true; // Native selection remains on row 4.
  return false;
 }
 if(state==0x80000001u&&rd16(ui+0x54)==4&&key==9){difficultyButton=true;return true;}
 return false;
}
static bool difficultyMenuTouch(int x,int y,int phase){
 if(difficultyTouchHeld&&phase!=0){if(phase==1)difficultyTouchHeld=false;return true;}
 if(!difficultySystem()){difficultyOpen=difficultyButton=false;return false;}
 const auto p=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,preferences.uiScale).inverse(x,y);x=p.first;y=p.second;
 auto box=[&](int a,int b,int w,int h){return destinia::nativeCodexRect(a,b,w,h,SCREEN_W,SCREEN_H).contains(x,y);};
 if(difficultyOpen){if(phase==0){difficultyTouchHeld=true;if(box(272,49,34,14)){difficultyOpen=false;return true;}for(int i=0;i<5;++i)if(box(94,62+i*25,212,22)){difficultySelected=i;difficultyMenuKey(17,true);return true;}}return true;}
 if(phase==0&&box(94,177,212,20)){difficultyTouchHeld=true;difficultyEnter();return true;}return false;
}
static void difficultyClassHook(uc_engine*,uint64_t,uint32_t,void*){
 try{if(!difficultyReady||(regLR()&~1u)!=SO_BASE+0x21c7a)return;
 const auto menu=exportsMap.at("Menu");const unsigned slot=rd16(menu+0xbe);
 if(slot>=10||!fs::is_regular_file(fs::path(saveDir)/("save"+std::to_string(slot)+".dat")))return;
 auto label=cstr(R(1),160);if(label.empty())return;
 label+=" - "+destinia::difficultyLabel(difficultyProgress.current(slot));
 // The original byte font has no accented Latin glyph; use its readable ASCII spelling.
 const auto accent=label.find(u8"\u00ed");if(accent!=std::string::npos)label.replace(accent,2,"i");
 wr(difficultyString,label.c_str(),label.size()+1);W(1,difficultyString);
 // The class font/color is restored by the original caller (white).
 }catch(const std::exception& e){hostReport("[dificuldade] Indicador recusado: %s\n",e.what());}
}
static void installDifficulty(){
 if(!codexAvailable)return; // Same verified ARM fingerprint as the native menu.
 try{difficultyProgress=destinia::loadDifficulty(fs::path(saveDir),preferences.gameMode=="expandido"?destinia::progression::Mode::Expanded:destinia::progression::Mode::Classic);difficultyString=hmalloc(512);uc_hook hook;auto at=exportsMap.at("Font_drawStringChar")&~1u;
 if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(difficultyClassHook),nullptr,at,at)!=UC_ERR_OK)throw std::runtime_error("cannot install difficulty label");difficultyReady=true;
 }catch(const std::exception& e){difficultyReady=false;hostReport("[dificuldade] Dados preservados; recurso desativado: %s\n",e.what());}
}
class DifficultyPainter {
 u32 descriptor=0,pixels=0,string=0;
 void call(const char* n,std::initializer_list<u32> a){if(!callFn(exportsMap.at(n),a))throw std::runtime_error("difficulty native drawing failed");}
 void fill(int x,int y,int w,int h,uint16_t c){call("Grp_setDrawOpVal",{0,0xffffffff});call("Grp_setColorPixel",{c});call("Grp_fillRect",{4,u32(x),u32(y),u32(w),u32(h)});}
 void text(int x,int y,std::string value,uint16_t color=0xffff){const auto accent=value.find(u8"\u00ed");if(accent!=std::string::npos)value.replace(accent,2,"i");wr(string,value.c_str(),value.size()+1);call("Font_setColorPixel",{color});call("Font_drawStringChar",{4,string,u32(x),u32(y),208,12,1,0x11});}
public:
 void paint(uint8_t* output){
  if(!difficultySystem()){difficultyOpen=difficultyButton=false;return;}const int ox=(SCREEN_W-400)/2,oy=(SCREEN_H-240)/2;
  if(!descriptor){descriptor=hmalloc(12);pixels=hmalloc(FB_BYTES);string=hmalloc(512);wr32(descriptor,SCREEN_W);wr32(descriptor+4,SCREEN_H);wr32(descriptor+8,pixels);}
  const auto table=exportsMap.at("frameBuffer")+16,old=rd32(table),grp=exportsMap.at("Grp"),font=rd32(exportsMap.at("Font"));std::array<uint8_t,124> graphics{};rd(grp,graphics.data(),graphics.size());wr32(table,descriptor);
  auto restore=[&](){wr32(table,old);wr(grp,graphics.data(),graphics.size());wr32(exportsMap.at("Font"),font);};
  auto button=[&](int y,std::string label,bool selected,bool enabled){fill(ox+94,oy+y,212,20,selected?0x3186:0x0841);call("Grp_setColorPixel",{selected?0xffc0u:0x528au});call("Grp_drawRoundRect",{4,u32(ox+94),u32(oy+y),212,20,2});text(ox+99,oy+y+4,label,enabled?0xffff:0x738e);};
  int top=difficultyOpen?49:177,bottom=difficultyOpen?229:197;
  try{std::vector<uint8_t> blank(FB_BYTES);wr(pixels,blank.data(),blank.size());
   if(difficultyOpen){fill(ox+90,oy+49,220,180,0x0841);text(ox+98,oy+49,"MUDAR DIFICULDADE");text(ox+272,oy+49,"SAIR");for(int i=0;i<5;++i){auto label=destinia::difficultyLabel(i);if(unsigned(i)==difficultyProgress.current(difficultySlot()))label+=" [ATUAL]";button(62+i*25,label,i==difficultySelected,difficultyProgress.allowed(difficultySlot(),i));}text(ox+98,oy+195,"DIABLO LIBERA A PROXIMA",0xffc0);if(!difficultyStatus.empty())text(ox+98,oy+211,difficultyStatus.substr(0,34),0xce59);}
   else button(177,"Mudar Dificuldade",difficultyButton,true);
   std::vector<uint8_t> raster(FB_BYTES);rd(pixels,raster.data(),raster.size());restore();
   const auto transform=destinia::nativeCodexTransform(SCREEN_W,SCREEN_H,preferences.uiScale);
   for(int y=oy+top;y<oy+bottom;++y)for(int x=ox+90;x<ox+310;++x){auto b=transform.rect(x,y,1,1);auto src=(y*SCREEN_W+x)*2;for(int yy=std::max(0,b.y);yy<std::min(SCREEN_H,b.y+b.h);++yy)for(int xx=std::max(0,b.x);xx<std::min(SCREEN_W,b.x+b.w);++xx){auto dst=(yy*SCREEN_W+xx)*2;output[dst]=raster[src];output[dst+1]=raster[src+1];}}
  }catch(...){restore();throw;}
 }
};
static DifficultyPainter difficultyPainter;
