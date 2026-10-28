#include "../common/survival_state.h"
#include <cassert>
#include <iostream>
using namespace destinia::survival;
int main(){
 CombatState c; c.enabled=true;
 assert(c.hit(100,100,1000,9999)==60); // cap before lethal check
 assert(c.hit(20,100,1000,9999)==19); assert(c.immuneMs==2000&&c.cooldownMs==120000);
 assert(c.hit(1,100,10,9999)==0); c.tick(1999);assert(c.hit(1,100,10,9999)==0);
 c.tick(1); assert(c.hit(1,100,10,9999)==10); // cooldown still active
 c.tick(118000);assert(c.hit(1,100,10,9999)==0&&c.immuneMs==2000);
 c.reset();c.enabled=false;assert(c.hit(1,100,50,9999)==50);
 c.elixirMs=300000;assert(c.hit(100,100,50,9999)==40);
 c.master=true;assert(c.hit(100,100,50,9999)==30); // additive 40% mitigation
 assert(c.hit(100,100,50,1499)==0);assert(c.hit(100,100,50,1500)==30);
 assert(c.activateElixir());assert(c.elixirMs==300000);assert(c.activateElixir());assert(c.elixirMs==300000);
 c.tick(300000);assert(c.hit(100,100,50,9999)==40);
 assert(c.hit(0,100,10,9999)==0);assert(c.hit(100,0,10,9999)==0);
 std::cout<<"SURVIVAL_STATE_OK\n";
}
