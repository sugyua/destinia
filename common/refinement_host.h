#pragma once
#include "refinement.h"
#include "safe_save.h"
#include <unicorn/unicorn.h>
#include <functional>
#include <optional>
#include <fstream>
#include <vector>

namespace destinia {
// No ARM calls or disk writes inside callbacks. The host stages pending metadata
// in the SAME SaveTransaction as the unmodified native GameCanvas_SaveGame path.
class RefinementHost {
 using U=uint32_t;
 uc_engine* engine=nullptr;U base=0,game=0;fs::path directory;
 std::map<std::string,U> symbols;
 std::function<bool(U,const std::vector<U>&,U*)> invoke;
 std::array<int64_t,13> bonus{};
 std::optional<std::string> previous,pending;
 bool staged=false;
 std::function<void(SaveTransaction&)> stageExtra;
 template<class T>T read(U at)const{T v{};if(uc_mem_read(engine,at,&v,sizeof v)!=UC_ERR_OK)throw std::runtime_error("Refino: leitura ARM invalida");return v;}
 template<class T>void write(U at,T v){if(uc_mem_write(engine,at,&v,sizeof v)!=UC_ERR_OK)throw std::runtime_error("Refino: escrita ARM invalida");}
 U reg(int id)const{U v=0;if(uc_reg_read(engine,id,&v)!=UC_ERR_OK)throw std::runtime_error("Refino: registro ARM invalido");return v;}
 U call(const char* name,const std::vector<U>& args={}){U result=0;if(!invoke(symbols.at(name),args,&result))throw std::runtime_error(std::string("Refino: rotina falhou ")+name);return result;}
 std::optional<std::string> disk()const{
  const auto path=directory/"refinement-v1.txt";
  if(isLink(path))throw std::runtime_error("Refino: sidecar vinculado recusado");
  if(!fs::exists(path))return std::nullopt;
  if(!fs::is_regular_file(path)||fs::file_size(path)>120000)throw std::runtime_error("Refino: sidecar invalido");
  std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Refino: falha na leitura");
  return std::string(std::istreambuf_iterator<char>(in),{});
 }
 static void hook(uc_engine*,uint64_t pc,uint32_t,void* context){auto& self=*static_cast<RefinementHost*>(context);try{self.at(U(pc));}catch(const std::exception& e){self.error=e.what();uc_emu_stop(self.engine);}}
 void at(U pc){
  if(!ready)return;
  if(pc==base+0x24fb0){bonus.fill(0);return;}
  if(pc==base+0x25064){
   const U p=reg(UC_ARM_REG_R5);if(!owned(p))return;
   const int level=itemLevel(p),type=(read<uint16_t>(p)>>5)&7;
   const U sp=reg(UC_ARM_REG_SP),index=read<uint16_t>(read<U>(sp));
   if(index>=13||!level)return;
   const auto value=read<uint16_t>(reg(UC_ARM_REG_R11));
   if(type<=2&&index==7)bonus[index]+=refinement_policy::weaponAttack(value,level)-value;
   if(type>=3&&type<=6&&index==8)bonus[index]+=refinement_policy::armorDefense(value,level)-value;
   if(type>=3&&type<=6&&index==5)bonus[index]+=refinement_policy::armorHealth(value,level)-value;
  }else if(pc==base+0x25392){
   for(unsigned index:{5u,7u,8u}){const U at=game+0x48ba+2*index;write<uint16_t>(at,uint16_t(std::min<int64_t>(32767,read<uint16_t>(at)+bonus[index])));}
  }
 }
public:
 refinement::Store store;std::string error,message;bool ready=false;
 unsigned selectedEquipment=0;
 bool owned(U p)const{return p>=game+0x2ed6&&p<game+0x2ed6+200*20&&(p-game-0x2ed6)%20==0&&read<uint8_t>(game+0x2e0c+(p-game-0x2ed6)/20)!=0;}
 refinement::Bytes bytes(U p)const{refinement::Bytes b{};if(uc_mem_read(engine,p,b.data(),b.size())!=UC_ERR_OK)throw std::runtime_error("Refino: equipamento invalido");return b;}
 int itemLevel(U p)const{return owned(p)?store.level(read<uint16_t>(game+0x4886),int((p-game-0x2ed6)/20),bytes(p)):0;}
 std::string suffix(U p)const{return owned(p)?refinement::tag(itemLevel(p)):"";}
 void install(uc_engine* e,U soBase,const std::map<std::string,U>& exports,const fs::path& dir,std::function<bool(U,const std::vector<U>&,U*)> caller){
  engine=e;base=soBase;symbols=exports;game=exports.at("Game");directory=dir;invoke=std::move(caller);
  if((exports.at("GameUI_calcHeroStat")&~1u)!=base+0x24fb0)throw std::runtime_error("Refino: ABI incompativel");
  previous=disk();if(previous)store.decode(*previous);
  for(U offset:{0x24fb0u,0x25064u,0x25392u}){uc_hook h;const auto at=base+offset;if(uc_hook_add(engine,&h,UC_HOOK_CODE,reinterpret_cast<void*>(hook),this,at,at)!=UC_ERR_OK)throw std::runtime_error("Refino: falha no hook");}
  ready=true;
 }
 // Called by callFn immediately before its SaveCall commits.
 void stage(SaveTransaction& transaction){
  if(!pending)return;
  if(disk()!=previous)throw std::runtime_error("Refino: sidecar mudou externamente");
  FILE* out=transaction.open(directory/"refinement-v1.txt","wb");
  if(!out)throw std::runtime_error("Refino: falha ao preparar sidecar");
  const bool okay=std::fwrite(pending->data(),1,pending->size(),out)==pending->size();
  const int closed=transaction.close(out);
  if(!okay||closed!=0)throw std::runtime_error("Refino: sidecar incompleto");
  if(stageExtra)stageExtra(transaction);
  staged=true;
 }
 bool commit(const refinement::Store& next,std::function<void(SaveTransaction&)> extra={}){
  if(!ready||!error.empty())return false;
  try{
   pending=next.encode();staged=false;stageExtra=std::move(extra);
   call("GameCanvas_SaveGame",{read<uint16_t>(game+0x4886)});
   if(!staged)throw std::runtime_error("Refino: transacao nao integrada");
   previous=pending;pending.reset();stageExtra={};store=next;return true;
  }catch(const std::exception& e){pending.reset();stageExtra={};message=e.what();return false;}
 }
 U selected()const{return read<U>(game+0x4584+4*selectedEquipment);}
 void cycle(){selectedEquipment=(selectedEquipment+1)%6;message="EQUIPAMENTO "+std::to_string(selectedEquipment+1)+" "+suffix(selected());}
 bool upgrade(U pointer=0){
  if(!ready){message="REFINO INDISPONIVEL";return false;}
  if(!pointer)pointer=selected();
  if(!owned(pointer)){message="EQUIPE UMA ARMA OU ARMADURA";return false;}
  const int type=(read<uint16_t>(pointer)>>5)&7;const auto kind=type<=2?refinement::Kind::Weapon:type<=6?refinement::Kind::Armor:refinement::Kind::Other;
  const unsigned slot=read<uint16_t>(game+0x4886);if(slot>=10)return false;
  const int64_t oldGold=call("getMyGold");int64_t gold=oldGold;auto next=store;
  const auto result=next.upgrade(int(slot),int((pointer-game-0x2ed6)/20),bytes(pointer),kind,gold);
  if(!result){message="REFINO: OURO/PEDRAS INSUFICIENTES OU LIMITE +15";return false;}
  call("setMyGold",{U(gold)});
  if(!commit(next)){call("setMyGold",{U(oldGold)});return false;}
  call("GameUI_calcHeroStat");message="REFINADO "+refinement::tag(result.level)+" - SUCESSO 100%";return true;
 }
 std::string summary()const{
  if(!ready)return message.empty()?"REFINO INDISPONIVEL":message;
  const auto s=read<uint16_t>(game+0x4886);
  return "PEDRAS C/P/A "+std::to_string(store.crystals(s,0))+"/"+std::to_string(store.crystals(s,1))+"/"+std::to_string(store.crystals(s,2));
 }
};
}
