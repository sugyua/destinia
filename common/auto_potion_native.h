#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <stdexcept>

namespace destinia {
class AutoPotionAccessError : public std::runtime_error {
public:
  explicit AutoPotionAccessError(const char* message) : std::runtime_error(message) {}
};
class AutoPotionRestoreError : public std::runtime_error {
public:
  AutoPotionRestoreError() : std::runtime_error("Auto-pocao: restauracao do atalho nao confirmada") {}
};
inline bool validPotionData(const std::vector<uint8_t>& table, uint16_t id, int type) {
  if (table.size()!=158u*166u || (type!=0 && type!=1) || id<type*5 || id>=type*5+5) return false;
  const size_t at=size_t(id)*166;
  auto u16=[&](size_t offset){return uint16_t(table[at+offset] | (uint16_t(table[at+offset+1])<<8));};
  return u16(0)==id && u16(4)==type && u16(6)==id%5;
}
inline bool useNativePotion(uint32_t game, int type,
    const std::function<uint16_t(uint32_t)>& read,
    const std::function<void(uint32_t,uint16_t)>& write,
    const std::function<bool()>& invoke,
    const std::function<bool(uint16_t,int)>& valid) {
  if (!game || !read || !write || !invoke || !valid || (type != 0 && type != 1)) return false;
  const uint16_t group=read(game+0x4a8c);
  if (group > 4) return false;
  int chosen=-1;
  for (unsigned bag=0;bag<4;++bag) {
    const uint32_t base=game+0x433c+bag*0x92;
    const uint16_t count=read(base);
    if (count>24) return false;
    for (unsigned slot=0;slot<count;++slot) {
      const uint32_t record=base+2+6*slot;
      const uint16_t kind=read(record), id=read(record+2), qty=read(record+4);
      if (kind==1 && qty && id>=type*5 && id<type*5+5 && valid(id,type) && chosen<0) chosen=id;
    }
  }
  if (chosen<0) return false;
  auto total=[&]() {
    uint32_t n=0;
    for (unsigned bag=0;bag<4;++bag) {
      const uint32_t base=game+0x433c+bag*0x92;
      const uint16_t count=read(base);
      if (count>24) return UINT32_MAX;
      for (unsigned slot=0;slot<count;++slot) {
        const uint32_t rec=base+2+6*slot;
        if (read(rec)==1 && read(rec+2)==chosen) n+=read(rec+4);
      }
    }
    return n;
  };
  const uint32_t before=total(), slot=game+0x4a90+10*group;
  const uint16_t old=read(slot);
  auto restore=[&]() {
    try { write(slot,old); if(read(slot)!=old) throw AutoPotionRestoreError(); }
    catch(...) { throw AutoPotionRestoreError(); }
  };
  try { write(slot,uint16_t(0x4000|chosen)); }
  catch(...) { restore(); throw; }
  bool ok=false;
  try {ok=invoke();} catch (...) {restore();throw;}
  restore();
  const uint32_t after=total();
  return ok && before!=UINT32_MAX && after!=UINT32_MAX && before==after+1;
}
}
