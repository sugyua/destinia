#pragma once
#include "loot_qol.h"
#include <unicorn/unicorn.h>
#include <fstream>
#include <functional>
#include <array>
#include <string>
namespace destinia {
// Verified APK1.0.6 only. Hooks never emulate recursively or alter drop records.
class LootQolRuntime {
 static constexpr uint32_t returnPc=0x60902000;
 uc_engine* engine=nullptr;uint32_t base=0,game=0,canvas=0,press=0,caller=0;int width=0,height=0,lastTarget=-1;bool names=false;
 std::map<int,int> types;std::array<uint64_t,20> retryAfter{};
 template<class T>T read(uint32_t at)const{T value{};if(uc_mem_read(engine,at,&value,sizeof value)!=UC_ERR_OK)throw std::runtime_error("Loot: leitura ARM invalida");return value;}
 uint32_t reg(int id)const{uint32_t value=0;uc_reg_read(engine,id,&value);return value;}
 void set(int id,uint32_t value){uc_reg_write(engine,id,&value);}
 static void hook(uc_engine*,uint64_t pc,uint32_t,void* data){auto& self=*static_cast<LootQolRuntime*>(data);try{self.at(uint32_t(pc));}catch(const std::exception& e){self.error=e.what();uc_emu_stop(self.engine);}}
 void at(uint32_t pc){
  if(pc==returnPc){names=false;set(UC_ARM_REG_LR,caller);set(UC_ARM_REG_PC,caller);return;}
  if(pc==base+0x166e0){if(options.visual&&!names){names=true;caller=reg(UC_ARM_REG_LR);set(UC_ARM_REG_LR,returnPc|1);}return;}
  if(pc==base+0x1a0ca){int index=int(reg(UC_ARM_REG_R2));if(options.visual&&index>=0&&index<20&&!visibleDrop(drop(index),options))set(UC_ARM_REG_PC,(base+0x19a64)|1);return;}
  if(pc==base+0x12444){int index=int(reg(UC_ARM_REG_R0));if((autoTarget>=0&&(reg(UC_ARM_REG_LR)&~1u)==base+0x208f4&&index!=autoTarget)||(names&&index>=0&&index<20&&!visibleDrop(drop(index),options))){set(UC_ARM_REG_R0,0);set(UC_ARM_REG_PC,reg(UC_ARM_REG_LR));}return;}
  if(autoTarget<0)return;
  if(pc==base+0x209e4)set(UC_ARM_REG_PC,(base+0x2019c)|1); // No NPC or attack tail.
  if(pc==base+0x208d2)set(UC_ARM_REG_PC,(base+0x208e2)|1); // No auto-target side effect.
 }
public:
 LootOptions options;int autoTarget=-1;std::string error;
 LootDrop drop(int index)const{const auto p=game+0x3e90+uint32_t(index)*60;return describeDrop(read<uint16_t>(p+48),read<uint16_t>(p+50),types);}
 bool gameplay()const{return engine&&read<uint32_t>(read<uint32_t>(base+0x44240))==canvas&&read<uint16_t>(game+0x4854)==0;}
 void install(uc_engine* e,uint32_t soBase,const std::map<std::string,uint32_t>& exports,int w,int h,const std::string& library,const std::vector<uint8_t>& itemTable){
  std::ifstream file(library,std::ios::binary);uint64_t hash=14695981039346656037ull;char b;while(file.get(b))hash=(hash^uint8_t(b))*1099511628211ull;if(!file.eof()||hash!=0x74b2547839b88777ull)throw std::runtime_error("Loot: biblioteca incompativel");
  types=lootMaterialTypes(itemTable);engine=e;base=soBase;game=exports.at("Game");canvas=exports.at("GameCanvas");press=exports.at("GameCanvas_mKeyPressed");width=w;height=h;
  if(uc_mem_map(e,returnPc,4096,UC_PROT_READ|UC_PROT_EXEC)!=UC_ERR_OK)throw std::runtime_error("Loot: retorno indisponivel");
  for(uint32_t pc:{base+0x166e0,base+0x12444,base+0x1a0ca,base+0x209e4,base+0x208d2,returnPc}){uc_hook h;if(uc_hook_add(e,&h,UC_HOOK_CODE,(void*)hook,this,pc,pc)!=UC_ERR_OK)throw std::runtime_error("Loot: falha instalando guarda");}
 }
 bool collect(uint64_t frame,bool allowed,const std::function<bool(uint32_t)>& invoke){
  if(!allowed||frame%4||(!options.autoGold&&!options.autoMaterials)||!gameplay()||read<uint16_t>(game+0x4866)!=0)return true;
  const int x=read<int16_t>(game+0x4890),y=read<int16_t>(game+0x4892);
  for(int n=1;n<=20;n++){int i=(lastTarget+n)%20;if(frame<retryAfter[size_t(i)]||read<int8_t>(game+0x3e76+i)<=0)continue;const auto p=game+0x3e90+uint32_t(i)*60;
   if(!autoEligible(drop(i),options,read<int16_t>(p+40)-x,read<int16_t>(p+42)-y))continue;
   lastTarget=i;autoTarget=i;bool okay=false;try{okay=invoke(press);}catch(...){autoTarget=-1;throw;}autoTarget=-1;retryAfter[size_t(i)]=read<int8_t>(game+0x3e76+i)>0?frame+32:0;return okay&&error.empty();
  }return true;
 }
 void paint(uint8_t* pixels)const{
  if(!options.visual||!gameplay())return;
  const int cx=read<int16_t>(game+0x4878),cy=read<int16_t>(game+0x487a);
  for(int i=0;i<20;i++)if(read<int8_t>(game+0x3e76+i)>0){const auto p=game+0x3e90+uint32_t(i)*60;auto d=drop(i);if(visibleDrop(d,options))paintLootMarker(pixels,width,height,read<int16_t>(p+40)-cx,read<int16_t>(p+42)-cy,lootColor(d));}
 }
};
}
