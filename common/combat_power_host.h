#pragma once
#include "combat_power.h"
static bool fcStatusSeen=false;static destinia::CombatPanel fcPanel;
static std::array<u32,4> fcCandidates{};static size_t fcCandidateCount=0;
static void fcBeginFrame(){fcStatusSeen=false;fcCandidateCount=0;}
static void fcCapture(uc_engine*,uint64_t pc,uint32_t,void*){
 if(pc==(exportsMap.at("_drawMenuStatus")&~1u)){fcStatusSeen=true;fcPanel={int32_t(R(1)),int32_t(R(2)),int(R(3))};}
 else if(R(3)&&fcCandidateCount<fcCandidates.size())fcCandidates[fcCandidateCount++]=R(3);
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
  if(type<3&&type!=int16_t(rd16(game+0x4886)))result.reason="OUTRA CLASSE";
  else if(required>rd16(game+0x48fa))result.reason="NIVEL BAIXO";
  else if(!identified)result.reason="IDENTIFIQUE";
  else if(((header>>8)&7)>4)result.reason="NAO VERIFICADO";
  if(!result.reason.empty()){restore();return result;}
  wr32(game+0x4584+4*(type<3?0:type-2),candidate);
  if(!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))throw std::runtime_error("FC: simulacao nativa falhou");result.after=stats();result.projected=destinia::combatPower(result.after);result.valid=true;restore();return result;
 }catch(...){restore();throw;}
}
static void fcPaint(uint8_t* pixels,int scale){
 if(!fcStatusSeen&&!fcCandidateCount)return;const auto game=exportsMap.at("Game"),ui=exportsMap.at("GameUI");if(!rd16(game+0x48fa)||int16_t(rd16(game+0x4854))!=16)return;
 if(fcStatusSeen){auto result=fcCompareEquipment(0);if(result.valid)destinia::paintCombatPower(pixels,SCREEN_W,SCREEN_H,scale,fcPanel,result,false,portugueseTextTest);}
 for(size_t i=0;i<fcCandidateCount;i++){const auto pointer=fcCandidates[i];bool equipped=false;for(int slot=0;slot<6;slot++)equipped|=pointer==rd32(ui+0x2148+slot*4);if(equipped&&i+1<fcCandidateCount)continue;auto result=fcCompareEquipment(pointer);destinia::paintCombatPower(pixels,SCREEN_W,SCREEN_H,scale,{SCREEN_W/2-116,0,232},result,true,portugueseTextTest);break;}
}
