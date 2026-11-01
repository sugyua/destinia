#pragma once
#include <cstdint>
namespace destinia {
// Repair only a persistent overfill in a live, initialized character.
// Changing values, load/death states, or missing maxima reset the observation.
class ManaGuard {
 uint16_t previousCurrent=0,previousMaximum=0,previousLevel=0,previousSlot=0;
 unsigned stableFrames=0;
public:
 void reset(){stableFrames=0;}
 template<class R,class W> bool update(uint32_t base,R read,W write){
  if(!base)return false;
  const uint16_t state=read(base+0x4854),level=read(base+0x48fa),hp=read(base+0x48f4),slot=read(base+0x4886);
  const uint16_t current=read(base+0x48f2),maximum=read(base+0x48f6);
  if(slot>=10 || state!=0 || int16_t(level)<=0 || int16_t(hp)<=0 || int16_t(maximum)<=0 || int16_t(current)<=0 || current<=maximum){stableFrames=0;return false;}
  if(current!=previousCurrent||maximum!=previousMaximum||level!=previousLevel||slot!=previousSlot){previousCurrent=current;previousMaximum=maximum;previousLevel=level;previousSlot=slot;stableFrames=1;return false;}
  if(++stableFrames<60)return false;
  write(base+0x48f2,maximum);stableFrames=0;return true;
 }
};
}
