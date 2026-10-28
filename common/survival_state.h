#pragma once
#include <algorithm>
#include <cstdint>
namespace destinia { namespace survival {
// Transient timers are advanced only by unpaused gameplay, never by painting.
struct CombatState {
 bool enabled=false,master=false;
 uint32_t immuneMs=0,cooldownMs=0,elixirMs=0;
 void tick(uint32_t ms){auto dec=[ms](uint32_t& n){n=ms>=n?0:n-ms;};dec(immuneMs);dec(cooldownMs);dec(elixirMs);}
 void reset(){immuneMs=cooldownMs=elixirMs=0;master=false;}
 bool activateElixir(){elixirMs=300000;return true;}
 uint32_t hit(uint32_t hp,uint32_t maxHp,uint32_t damage,uint32_t roll){
  if(!hp||!maxHp||!damage||immuneMs)return 0;
  if(master&&roll<1500)return 0; // supplied uniform roll in [0,10000)
  const unsigned resistance=(master?20:0)+(elixirMs?20:0);
  damage=uint32_t(uint64_t(damage)*(100-resistance)/100);
  damage=std::min(damage,std::max(1u,uint32_t(uint64_t(maxHp)*60/100)));
  if(enabled&&!cooldownMs&&damage>=hp){immuneMs=2000;cooldownMs=120000;return hp-1;}
  return damage;
 }
};
}}
