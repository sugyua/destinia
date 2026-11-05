#pragma once
#include "combat_power.h"
#include <sstream>
static bool fcStatusSeen=false;static destinia::CombatPanel fcPanel;
static std::array<destinia::CombatPanel,4> fcCandidatePanels{};
static bool fcComparisonPage=false,fcTouchCaptured=false;
static std::array<u32,4> fcCandidates{};static size_t fcCandidateCount=0;
static void fcBeginFrame(){fcStatusSeen=false;fcCandidateCount=0;}
static void fcCapture(uc_engine*,uint64_t pc,uint32_t,void*){
 if(pc==(exportsMap.at("_drawMenuStatus")&~1u)){fcStatusSeen=true;fcPanel={int32_t(R(1)),int32_t(R(2)),int(R(3))};}
 else if(R(3)&&fcCandidateCount<fcCandidates.size()){fcCandidatePanels[fcCandidateCount]={int32_t(R(1)),int32_t(R(2)),218};fcCandidates[fcCandidateCount++]=R(3);}
}
static void fcInstall(){for(const char* name:{"_drawMenuStatus","_drawEquipDesc"}){const auto at=exportsMap.at(name)&~1u;uc_hook hook;if(uc_hook_add(uc,&hook,UC_HOOK_CODE,(void*)fcCapture,nullptr,at,at)!=UC_ERR_OK)throw std::runtime_error("FC: falha no observador");}}
static destinia::CombatPreview fcCompareEquipment(u32 candidate,bool menuBackup=true){
 destinia::CombatPreview result;const auto game=exportsMap.at("Game"),ui=exportsMap.at("GameUI");std::vector<uint8_t> snapshot(32056);if(!rd(game,snapshot.data(),snapshot.size()))throw std::runtime_error("FC: leitura de estado falhou");
 auto owned=[&](u32 pointer){if(pointer<game+0x2ed6 || pointer>=game+0x2ed6+20*200 || (pointer-game-0x2ed6)%20)return false;uint8_t active=0;return rd(game+0x2e0c+(pointer-game-0x2ed6)/20,&active,1)&&active!=0;};
 auto stats=[&](){destinia::CombatStats values{};for(int i=0;i<13;i++)values[i]=int16_t(rd16(game+0x48ba+2*i));return values;};
 auto restore=[&](){if(!wr(game,snapshot.data(),snapshot.size()))throw std::runtime_error("FC: nao foi possivel restaurar o estado");};
 try{
  for(int i=0;i<6;i++){const auto pointer=rd32((menuBackup?ui+0x2148:game+0x4584)+4*i);if(pointer&&!owned(pointer)){result.reason="NAO VERIFICADO";restore();return result;}wr32(game+0x4584+4*i,pointer);}
  if(!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))throw std::runtime_error("FC: calculo nativo falhou");result.before=stats();result.current=destinia::combatPower(result.before);result.after=result.before;result.projected=result.current;
  if(!candidate){result.valid=true;restore();return result;}
  if(!owned(candidate)){result.reason="NAO VERIFICADO";restore();return result;}
  const auto header=rd16(candidate);const int type=(header>>5)&7;u32 required=0,identified=0;
  if(!callFn(exportsMap.at("GameUI_getEquipMinLv"),{header,1},&required)||!callFn(exportsMap.at("GameUI_equipDistinguish"),{candidate},&identified))throw std::runtime_error("FC: requisitos indisponiveis");
  if(type<3&&type!=destinia::weaponTypeForJob(int16_t(rd16(game+0x4886))))result.reason="OUTRA CLASSE";
  else if(required>rd16(game+0x48fa))result.reason="NIVEL BAIXO";
  else if(!identified)result.reason="IDENTIFIQUE";
  else if(((header>>8)&7)>4)result.reason="NAO VERIFICADO";
  if(!result.reason.empty()){restore();return result;}
  wr32(game+0x4584+4*(type<3?0:type-2),candidate);
  if(!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))throw std::runtime_error("FC: simulacao nativa falhou");result.after=stats();result.projected=destinia::combatPower(result.after);result.valid=true;restore();return result;
 }catch(...){restore();throw;}
}
static size_t fcSelection(){const auto u=exportsMap.at("GameUI");for(size_t i=0;i<fcCandidateCount;i++){bool equipped=false;for(int n=0;n<6;n++)equipped|=fcCandidates[i]==rd32(u+0x2148+4*n);if(!equipped||i+1==fcCandidateCount)return i;}return 0;}
static bool fcComparisonVisible(){const auto g=exportsMap.at("Game"),u=exportsMap.at("GameUI"),state=rd32(u+0x7c);return !codexOpen&&fcCandidateCount&&rd16(g+0x48fa)>0&&int16_t(rd16(g+0x4854))==16&&rd16(u+0x76)>=2&&(state==0x10000002u||state==0x20000002u||state==0x91040004u);}
static bool fcMenuKey(int key,bool down){if(key==10015){if(down&&fcComparisonVisible())fcComparisonPage=!fcComparisonPage;return true;}if(!down||!fcComparisonVisible()||(key!=2&&key!=11))return false;fcComparisonPage=key==11;return true;}
static bool fcMenuTouch(int x,int y,int phase){if(phase==0)fcTouchCaptured=false;if(fcTouchCaptured&&phase!=0){if(phase==1)fcTouchCaptured=false;return true;}if(phase!=0||!fcComparisonVisible())return false;const auto p=destinia::UiTransform{SCREEN_W,SCREEN_H,preferences.uiScale,destinia::UiAnchor::Center,232,240}.inverse(x,y);const auto b=fcCandidatePanels[fcSelection()];if(p.first<b.x||p.first>=b.x+218||p.second<b.y+68||p.second>=b.y+82)return false;fcComparisonPage=p.first>=b.x+109;fcTouchCaptured=true;return true;}
class FcNativePainter {
 struct Raster{destinia::UiRect area;std::vector<uint8_t> data;bool transparent=false;};
 u32 descriptor=0,pixels=0,string=0;std::map<std::string,Raster> cache;
 static uint16_t color(int64_t n){return n>0?0x07e0:n<0?0xf800:0x8c51;}
 void call(const char* name,std::initializer_list<u32> args){if(!callFn(exportsMap.at(name),args))throw std::runtime_error(std::string("FC nativo: ")+name);}
 void text(int x,int y,std::string value,uint16_t c=0xffff,int width=208){if(value.size()>160)value.resize(160);wr(string,value.c_str(),value.size()+1);call("Grp_setDrawOpVal",{0,0xffffffff});call("Font_setColorPixel",{c});call("Font_drawStringChar",{4,string,u32(x),u32(y),u32(width),12,1,0x11});}
public:
 void paint(uint8_t* output,int percent,destinia::CombatPanel panel,const destinia::CombatPreview& info,int kind,u32 candidate,bool pt){
  // kind0 = transparent Status label; kind1 = Item footer; kind2 = comparison.
  destinia::UiRect area=kind==0?destinia::UiRect{panel.x+98,panel.y+20,114,14}:kind==1?destinia::UiRect{panel.x,panel.y+68,218,14}:destinia::UiRect{panel.x,panel.y,218,82};
  if(area.x<0||area.y<0||area.x+area.w>SCREEN_W||area.y+area.h>SCREEN_H)return;
  const auto g=exportsMap.at("Game");const bool owned=candidate>=g+0x2ed6&&candidate<g+0x2ed6+4000&&(candidate-g-0x2ed6)%20==0&&(rd16(g+0x2e0c+(candidate-g-0x2ed6)/20)&255)!=0;
  std::ostringstream stamp;stamp<<kind<<':'<<panel.x<<':'<<panel.y<<':'<<panel.w<<':'<<pt<<':'<<preferences.keys.count(SDLK_F6);
  if(kind!=1){stamp<<':'<<candidate<<':'<<info.valid<<':'<<info.current<<':'<<info.projected<<':'<<info.reason;for(auto n:info.before)stamp<<':'<<n;for(auto n:info.after)stamp<<':'<<n;if(owned&&kind==2){std::array<uint8_t,20> record{};rd(candidate,record.data(),record.size());for(auto b:record)stamp<<':'<<unsigned(b);}}
  const auto key=stamp.str();auto found=cache.find(key);
  if(found==cache.end()){
   if(!descriptor){descriptor=hmalloc(12);pixels=hmalloc(FB_BYTES);string=hmalloc(1024);wr32(descriptor,SCREEN_W);wr32(descriptor+4,SCREEN_H);wr32(descriptor+8,pixels);}
   const auto table=exportsMap.at("frameBuffer")+16,slot=rd32(table),grp=exportsMap.at("Grp"),font=rd32(exportsMap.at("Font"));std::array<uint8_t,124> graphics{};rd(grp,graphics.data(),graphics.size());wr32(table,descriptor);
   auto restore=[&](){wr(grp,graphics.data(),graphics.size());wr32(exportsMap.at("Font"),font);wr32(table,slot);};Raster raster{area,{},kind==0};
   try{std::vector<uint8_t> blank(FB_BYTES);if(kind==0)for(size_t i=0;i<blank.size();i+=2){blank[i]=0x1f;blank[i+1]=0xf8;}wr(pixels,blank.data(),blank.size());
    if(kind==0){text(panel.x+101,panel.y+22,"FC",0xffc0,22);text(panel.x+125,panel.y+22,std::to_string(info.current),0xffff,86);}
    else{call("_drawDescBox",{4,u32(panel.x),u32(panel.y),218,82});
     if(kind==2){std::string name="ITEM";uint16_t titleColor=0xffff;const int rank=owned?(rd16(candidate)>>8)&7:5;
      if(owned&&rank<=4&&info.reason!="NAO VERIFICADO"){call("GameUI_getEquipName",{string,candidate});name=cstr(string,512)+refinementHost.suffix(candidate);u32 c=0;if(!callFn(exportsMap.at("GameUI_getEquipColor"),{u32(rank)},&c))throw std::runtime_error("FC: cor nativa indisponivel");titleColor=uint16_t(c);}text(panel.x+4,panel.y+4,name,titleColor);
      if(!info.valid){text(panel.x+4,panel.y+16,"FC "+std::to_string(info.current));std::string reason=info.reason;if(!pt){if(reason=="OUTRA CLASSE")reason="OTHER CLASS";else if(reason=="NIVEL BAIXO")reason="LOW LEVEL";else if(reason=="IDENTIFIQUE")reason="IDENTIFY";else reason="UNVERIFIED";}text(panel.x+4,panel.y+30,reason,0xf800);}
      else{text(panel.x+4,panel.y+16,"FC "+std::to_string(info.current)+" > "+std::to_string(info.projected),0xffff,150);text(panel.x+158,panel.y+16,destinia::signedPower(info.delta()),color(info.delta()),56);
       int row=0;for(const auto& label:std::array<std::pair<int,const char*>,5>{{{7,"ATK"},{8,"DEF"},{5,"PV"},{6,"PM"},{11,"CRI"}}}){const int n=info.after[label.first]-info.before[label.first];const std::string value=label.first==11?std::string(n<0?"-":n>0?"+":"")+std::to_string(std::abs(n)/10)+"."+std::to_string(std::abs(n)%10)+"%":destinia::signedPower(n);text(panel.x+4+row%2*108,panel.y+28+row/2*12,std::string(!pt&&label.first==5?"HP":!pt&&label.first==6?"MP":label.second)+" "+value,color(n),104);++row;}}
     }
     text(panel.x+10,panel.y+71,"ITEM",kind==1?0xffc0:0x8c51,60);text(panel.x+88,panel.y+71,"RS",0x8c51,40);text(panel.x+154,panel.y+71,pt?"COMPARAR":"COMPARE",kind==2?0xffc0:0x8c51,60);
    }
    std::vector<uint8_t> full(FB_BYTES);rd(pixels,full.data(),full.size());raster.data.resize(size_t(area.w)*area.h*2);for(int y=0;y<area.h;y++)std::memcpy(raster.data.data()+y*area.w*2,full.data()+((area.y+y)*SCREEN_W+area.x)*2,area.w*2);restore();
   }catch(...){restore();throw;}
   if(cache.size()>=8)cache.clear();found=cache.emplace(key,std::move(raster)).first;
  }
  const auto& raster=found->second;const destinia::UiTransform transform{SCREEN_W,SCREEN_H,percent,destinia::UiAnchor::Center,232,240};
  if(percent==100&&!raster.transparent){for(int y=0;y<area.h;y++)std::memcpy(output+((area.y+y)*SCREEN_W+area.x)*2,raster.data.data()+y*area.w*2,area.w*2);return;}
  for(int y=0;y<area.h;y++)for(int x=0;x<area.w;x++){const auto at=(y*area.w+x)*2;if(raster.transparent&&(uint16_t(raster.data[at])|(uint16_t(raster.data[at+1])<<8))==0xf81f)continue;const auto b=transform.rect(area.x+x,area.y+y,1,1);for(int yy=std::max(0,b.y);yy<std::min(SCREEN_H,b.y+b.h);yy++)for(int xx=std::max(0,b.x);xx<std::min(SCREEN_W,b.x+b.w);xx++){auto dst=(yy*SCREEN_W+xx)*2;output[dst]=raster.data[at];output[dst+1]=raster.data[at+1];}}
 }
};
static FcNativePainter fcNativePainter;
static void fcPaint(uint8_t* pixels,int scale){
 if(!fcStatusSeen&&!fcCandidateCount)return;const auto g=exportsMap.at("Game"),u=exportsMap.at("GameUI");if(!rd16(g+0x48fa)||int16_t(rd16(g+0x4854))!=16||rd16(u+0x76)<2)return;
 if(fcStatusSeen&&fcPanel.w>=220&&(rd32(u+0x7c)>>28)==0){const auto info=fcCompareEquipment(0);if(info.valid)fcNativePainter.paint(pixels,scale,fcPanel,info,0,0,portugueseTextTest);}
 if(fcComparisonVisible()){const auto i=fcSelection();fcNativePainter.paint(pixels,scale,fcCandidatePanels[i],fcComparisonPage?fcCompareEquipment(fcCandidates[i]):destinia::CombatPreview{},fcComparisonPage?2:1,fcCandidates[i],portugueseTextTest);}
}
