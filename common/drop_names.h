#pragma once
#include <unicorn/unicorn.h>
#include <cstdint>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
namespace destinia {
// APK1.0.6: only broaden the visibility test *inside* drawDropItemName.
// Collection/combat callers of _checkItemNearby retain their native distance.
class DropNamesRenderer {
 static constexpr uint32_t returnPc=0x60900000;
 uc_engine* engine=nullptr;uint32_t base=0,game=0,caller=0;int width=0,height=0;bool drawing=false;
 uint32_t reg(int id){uint32_t v=0;uc_reg_read(engine,id,&v);return v;}
 void set(int id,uint32_t v){uc_reg_write(engine,id,&v);}
 template<class T>T read(uint32_t at){T v{};if(uc_mem_read(engine,at,&v,sizeof v)!=UC_ERR_OK)throw std::runtime_error("Drops: leitura ARM invalida");return v;}
 static void callback(uc_engine*,uint64_t at,uint32_t,void* data){auto& self=*static_cast<DropNamesRenderer*>(data);try{self.at(uint32_t(at));}catch(const std::exception& ex){self.error=ex.what();uc_emu_stop(self.engine);}}
 void at(uint32_t pc){
  if(pc==returnPc){drawing=false;set(UC_ARM_REG_LR,caller);set(UC_ARM_REG_PC,caller);return;}
  if(pc==base+0x166e0){if(enabled&&!drawing){drawing=true;caller=reg(UC_ARM_REG_LR);set(UC_ARM_REG_LR,returnPc|1);}return;}
  if(!drawing)return;
  const uint32_t index=reg(UC_ARM_REG_R0);bool visible=false;
  if(index<20&&read<int8_t>(game+0x3e76+index)>0){const auto entry=game+0x3e90+index*60;const int x=read<int16_t>(entry+40)-read<int16_t>(game+0x4878),y=read<int16_t>(entry+42)-read<int16_t>(game+0x487a);visible=x>=0&&x<width&&y>=0&&y<height;}
  set(UC_ARM_REG_R0,visible?1:0);set(UC_ARM_REG_PC,reg(UC_ARM_REG_LR));
 }
public:
 bool enabled=false;std::string error;
 void install(uc_engine* e,uint32_t soBase,const std::map<std::string,uint32_t>& exports,int w,int h,const std::string& library){
  std::ifstream file(library,std::ios::binary);uint64_t hash=14695981039346656037ull;char byte;while(file.get(byte))hash=(hash^static_cast<unsigned char>(byte))*1099511628211ull;
  if(!file.eof()||hash!=0x74b2547839b88777ull)throw std::runtime_error("Drops: biblioteca ARM incompativel");
  engine=e;base=soBase;game=exports.at("Game");width=w;height=h;enabled=false;drawing=false;error.clear();
  if(uc_mem_map(e,returnPc,4096,UC_PROT_READ|UC_PROT_EXEC)!=UC_ERR_OK)throw std::runtime_error("Drops: retorno indisponivel");
  for(uint32_t pc:{base+0x166e0,base+0x12444,returnPc}){uc_hook hook;if(uc_hook_add(e,&hook,UC_HOOK_CODE,(void*)callback,this,pc,pc)!=UC_ERR_OK)throw std::runtime_error("Drops: falha instalando hook");}
 }
};
}
