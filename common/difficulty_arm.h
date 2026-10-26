#pragma once
#include "progression.h"
#include <unicorn/unicorn.h>
#include <array>
#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace destinia {
// Verified APK 1.0.6 campaign ABI. No native save, drop table or RNG writes.
class DifficultyArmRuntime {
 using U=uint32_t;
 struct Health {unsigned slot;uint16_t species,kind;int32_t base,scaled;U factor;unsigned difficulty;};
 uc_engine* engine=nullptr;
 U base=0,game=0;
 bool ready=false;
 std::function<unsigned(unsigned)> difficulty;
 std::map<U,Health> health;
 std::map<U,unsigned> deadTier;
 std::map<U,U> xpFrames,goldFrames;
 std::map<U,std::pair<U,U>> abyssalEliteRestore;
 std::vector<uc_hook> hooks;
 template<class T>T read(U address)const{T value{};if(uc_mem_read(engine,address,&value,sizeof value)!=UC_ERR_OK)throw std::runtime_error("difficulty: invalid ARM read");return value;}
 template<class T>void write(U address,T value){if(uc_mem_write(engine,address,&value,sizeof value)!=UC_ERR_OK)throw std::runtime_error("difficulty: invalid ARM write");}
 U reg(int id)const{U value=0;if(uc_reg_read(engine,id,&value)!=UC_ERR_OK)throw std::runtime_error("difficulty: invalid ARM register");return value;}
 void set(int id,U value){if(uc_reg_write(engine,id,&value)!=UC_ERR_OK)throw std::runtime_error("difficulty: invalid ARM register write");}
 unsigned slot()const{return read<uint16_t>(game+0x4886);}
 unsigned activeTier()const{const auto s=slot();if(s>=10)throw std::runtime_error("difficulty: invalid active slot");const auto d=difficulty(s);progression::require(d<5,"difficulty: invalid profile");return d;}
 progression::Multipliers active()const{return progression::multipliers(progression::Difficulty(activeTier()),0);}
 progression::Multipliers reward(U monster){const auto d=deathDifficulty(monster);return d?progression::multipliers(progression::Difficulty(*d),0):active();}
 bool campaign(U monster)const{
  if(monster<game+0x14fc||monster>=game+0x14fc+20*136||(monster-game-0x14fc)%136)return false;
  const U index=(monster-game-0x14fc)/136,count=read<U>(game+0x1f9c);
  return count<=20&&index<count&&read<uint16_t>(monster+8)<=109&&read<uint16_t>(monster+0xa)<200;
 }
 static int32_t scaledHealth(int32_t value,U factor){
  if(value<=0)throw std::runtime_error("difficulty: invalid monster maximum HP");
  const auto result=progression::scale(U(value),factor);
  progression::require(result>0&&result<=INT32_MAX,"difficulty: monster HP exceeds signed ARM32");
  return int32_t(result);
 }
 void spawn(U monster){
  if(!campaign(monster))return;
  const auto raw=read<int32_t>(monster+0x24);if(raw<=0)return;
  const auto s=slot();const auto species=read<uint16_t>(monster+8),kind=read<uint16_t>(monster+0xa);
  auto found=health.find(monster);int32_t original=raw;
  if(found!=health.end()&&found->second.slot==s&&found->second.species==species&&found->second.kind==kind&&raw==found->second.scaled)original=found->second.base;
  const auto factor=active().hp;const auto scaled=scaledHealth(original,factor);
  // Mon_initMonster itself copies max HP (+0x24) to current HP (+0x20).
  // A respawn must reuse the original base, never the previously scaled max.
  if(raw!=scaled)write<int32_t>(monster+0x24,scaled);
  if(activeTier()==4&&read<uint16_t>(monster+8)<=109&&read<uint16_t>(monster+0xa)<200){
   write<uint32_t>(monster+0x4c,1);
  }
  health[monster]={s,species,kind,original,scaled,factor,activeTier()};deadTier.erase(monster);
 }
 static void hook(uc_engine*,uint64_t pc,uint32_t,void* context){
  auto& self=*static_cast<DifficultyArmRuntime*>(context);
  if(!self.ready||!self.error.empty())return;
  try{self.at(U(pc));}catch(const std::exception& ex){self.error=ex.what();uc_emu_stop(self.engine);}
 }
 void at(U pc){
  const U off=pc-base;
  if(off==0x1740c){spawn(reg(UC_ARM_REG_R0));return;}
  if(off==0x1bfc8||off==0x1b9e8){health.clear();deadTier.clear();return;}
  if(off==0x1f51c||off==0x18580){refresh();return;}
  // Final incoming hit, after native defense/crit/block, before display and HP subtraction.
  // Pet/PvP attackers are excluded by the campaign array and kind guards.
  if(off==0x133b2){if(campaign(reg(UC_ARM_REG_R6))){const auto factor=active().attack;if(factor!=progression::Basis){const auto value=progression::scale(reg(UC_ARM_REG_R8),factor);progression::require(value<=INT32_MAX,"difficulty: damage exceeds signed ARM32");set(UC_ARM_REG_R8,U(value));}}return;}
  if(off==0x123f0){if(xpFrames.size()>=64)throw std::runtime_error("difficulty: XP call depth");xpFrames[reg(UC_ARM_REG_SP)]=campaign(reg(UC_ARM_REG_R0))?reward(reg(UC_ARM_REG_R0)).xp:progression::Basis;return;}
  if(off==0x12430){const auto key=reg(UC_ARM_REG_SP)+8;auto frame=xpFrames.find(key);if(frame!=xpFrames.end()){const auto factor=frame->second;xpFrames.erase(frame);if(factor!=progression::Basis)set(UC_ARM_REG_R0,progression::checkedArmXp(progression::scale(reg(UC_ARM_REG_R0),factor)));}return;}
  if(off==0x16b7c){
   if(goldFrames.size()>=64)throw std::runtime_error("difficulty: drop call depth");
   const U m=reg(UC_ARM_REG_R2);
   goldFrames[reg(UC_ARM_REG_SP)]=campaign(m)?reward(m).gold:progression::Basis;
   if(campaign(m)&&reward(m).hp>=60000){
    const U elite=read<U>(m+0x4c);
    if(elite!=0){abyssalEliteRestore[reg(UC_ARM_REG_SP)]={m,elite};write<U>(m+0x4c,0);}
   }
   return;
  }
  if(off==0x16e96){auto frame=goldFrames.find(reg(UC_ARM_REG_SP)+0x38);if(frame!=goldFrames.end()&&frame->second!=progression::Basis){const auto value=progression::checkedU32(progression::scale(reg(UC_ARM_REG_R0),frame->second));set(UC_ARM_REG_R0,progression::checkedArmU16(value));}return;}
  if(off==0x16d6c){
   goldFrames.erase(reg(UC_ARM_REG_SP)+0x38);
   auto it=abyssalEliteRestore.find(reg(UC_ARM_REG_SP)+0x38);
   if(it!=abyssalEliteRestore.end()){write<U>(it->second.first+0x4c,it->second.second);abyssalEliteRestore.erase(it);}
   return;
  }
 }
public:
 std::string error;
 std::optional<unsigned> deathDifficulty(U monster){
  if(!campaign(monster)||read<int32_t>(monster+0x20)>0)return std::nullopt;
  auto found=deadTier.find(monster);if(found!=deadTier.end())return found->second;
  auto h=health.find(monster);const auto d=h!=health.end()?h->second.difficulty:activeTier();deadTier[monster]=d;return d;
 }
 void endCall(){xpFrames.clear();goldFrames.clear();abyssalEliteRestore.clear();}
 // Validate the next choice before committing it to disk. No ARM mutation here.
 void validateProfile(unsigned s,const progression::Multipliers& next)const{
  for(const auto& pair:health){const auto& h=pair.second;if(h.slot==s&&campaign(pair.first)&&read<int32_t>(pair.first+0x24)==h.scaled)scaledHealth(h.base,next.hp);}
 }
 void refresh(){
  if(!ready)return;const auto s=slot();if(s>=10)return;const auto factor=active().hp;
  struct Update{U p;int32_t maximum,current;};std::vector<Update> updates;
  for(auto it=health.begin();it!=health.end();){const U p=it->first;const auto& h=it->second;
   if(h.slot!=s||!campaign(p)||read<uint16_t>(p+8)!=h.species||read<uint16_t>(p+0xa)!=h.kind||read<int32_t>(p+0x24)!=h.scaled){it=health.erase(it);continue;}
   const auto old=read<int32_t>(p+0x20);if(old<=0)deathDifficulty(p);
   if(h.factor!=factor){const auto maximum=scaledHealth(h.base,factor);int32_t current=old;
    if(old>0){progression::require(old<=h.scaled,"difficulty: current HP exceeds maximum");current=int32_t(std::max<uint64_t>(1,uint64_t(old)*U(maximum)/U(h.scaled)));}
    updates.push_back({p,maximum,current});
   }++it;
  }
  // Compute/check the entire batch before touching live monsters.
  for(const auto& update:updates){write<int32_t>(update.p+0x24,update.maximum);write<int32_t>(update.p+0x20,update.current);auto& h=health.at(update.p);h.scaled=update.maximum;h.factor=factor;h.difficulty=activeTier();}
 }
 void install(uc_engine* e,U soBase,const std::map<std::string,U>& exports,const std::string& library,std::function<unsigned(unsigned)> provider){
  std::ifstream file(library,std::ios::binary);uint64_t hash=14695981039346656037ull;char byte;while(file.get(byte))hash=(hash^static_cast<unsigned char>(byte))*1099511628211ull;
  if(!file.eof()||hash!=0x74b2547839b88777ull)throw std::runtime_error("difficulty: incompatible ARM library");
  for(const auto& symbol:std::array<std::pair<const char*,U>,7>{{{"Game",0x77ee0},{"Mon_initMonster",0x1740c},{"Mon_createMonster",0x1bfc8},{"GameCanvas_LoadGame",0x1b9e8},{"GameCanvas_mRun",0x1f51c},{"GameCanvas_getGainExp",0x123f0},{"GameCanvas_dropItem",0x16b7c}}})
   if((exports.at(symbol.first)&~1u)!=soBase+symbol.second)throw std::runtime_error("difficulty: incompatible ARM symbols");
  engine=e;base=soBase;game=exports.at("Game");difficulty=std::move(provider);
  try{for(U offset:{0x1740cu,0x1bfc8u,0x1b9e8u,0x1f51cu,0x18580u,0x133b2u,0x123f0u,0x12430u,0x16b7cu,0x16e96u,0x16d6cu}){uc_hook h;const auto address=base+offset;if(uc_hook_add(engine,&h,UC_HOOK_CODE,reinterpret_cast<void*>(hook),this,address,address)!=UC_ERR_OK)throw std::runtime_error("difficulty: cannot install ARM hook");hooks.push_back(h);}ready=true;}
  catch(...){for(auto h:hooks)uc_hook_del(engine,h);hooks.clear();throw;}
 }
};
}
