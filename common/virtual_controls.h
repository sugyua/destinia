#pragma once
#include <unicorn/unicorn.h>
#include <cstdint>
#include <fstream>
#include <string>

namespace destinia {
// APK 1.0.6 Thumb code. States 2/3 are normal/boss gameplay; 4/5 and
// numeric/name-entry states belong to menus and must retain their controls.
inline bool gameplayTouchState(uint32_t state) {return state==2 || state==3;}
inline void virtualControlsDrawHook(uc_engine* engine,uint64_t,uint32_t,void* data) {
  uint32_t state=0;uc_reg_read(engine,UC_ARM_REG_R10,&state);
  if(!gameplayTouchState(state)) return;
  // After the original touch-transition update; execute its state-write and
  // stack-unwind epilogue. The omitted block also advances animation frames
  // for these hidden touch buttons; physical input does not use those frames.
  uint32_t pc=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data))+0x1031f;
  uc_reg_write(engine,UC_ARM_REG_PC,&pc);
}
inline void virtualControlsHitHook(uc_engine* engine,uint64_t,uint32_t,void* data) {
  uint32_t state=0;uc_reg_read(engine,UC_ARM_REG_R0,&state);
  if(!gameplayTouchState(state)) return;
  // getKeyPadIdx has already allocated its stack and returned from
  // _getTouchState. Return native key 0 (no hit), using its real epilogue.
  uint32_t noHit=0,pc=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data))+0xfb0b;
  uc_reg_write(engine,UC_ARM_REG_R0,&noHit);
  uc_reg_write(engine,UC_ARM_REG_PC,&pc);
}
inline bool installVirtualControls(uc_engine* engine,uint32_t base,const std::string& library,bool visible) {
  if(visible) return true; // Existing installations add no hooks or file reads.
  std::ifstream file(library,std::ios::binary);
  if(!file) return false;
  uint64_t fingerprint=14695981039346656037ull;
  char byte;
  while(file.get(byte)) fingerprint=(fingerprint^static_cast<unsigned char>(byte))*1099511628211ull;
  if(!file.eof() || fingerprint!=0x74b2547839b88777ull) return false;
  uc_hook drawHook,hitHook;
  void* data=reinterpret_cast<void*>(static_cast<uintptr_t>(base));
  if(uc_hook_add(engine,&drawHook,UC_HOOK_CODE,reinterpret_cast<void*>(virtualControlsDrawHook),data,base+0xff7c,base+0xff7c)!=UC_ERR_OK) return false;
  if(uc_hook_add(engine,&hitHook,UC_HOOK_CODE,reinterpret_cast<void*>(virtualControlsHitHook),data,base+0xfa3e,base+0xfa3e)!=UC_ERR_OK) {
    uc_hook_del(engine,drawHook);return false;
  }
  return true;
}
}
