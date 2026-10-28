#include "../common/refinement_system.h"
#include <cassert>
#include <limits>
#include <iostream>
using namespace destinia::refinement_policy;
int main() {
  for (int level=0; level<=15; ++level) {
    assert(weaponAttack(100,level)==100+8*level);
    assert(armorDefense(100,level)==100+6*level);
    assert(armorHealth(100,level)==100+4*level);
    assert(weaponAttack(0,level)==0);
    assert(checkpoint(level)==(level>=10?10:level>=5?5:0));
  }
  assert(weaponAttack(101,1)==109);
  assert(armorDefense(101,15)==191);
  assert(armorHealth(101,15)==161);
  assert(stoneForTarget(1)==Stone::Common && stoneForTarget(5)==Stone::Common);
  assert(stoneForTarget(6)==Stone::Pure && stoneForTarget(10)==Stone::Pure);
  assert(stoneForTarget(11)==Stone::Abyssal && stoneForTarget(15)==Stone::Abyssal);
  assert(quote(0).gold==500 && quote(14).gold==112500);
  assert(quote(4).stone==Stone::Common && quote(5).stone==Stone::Pure);
  assert(quote(10).stone==Stone::Abyssal && quote(14).successPercent==100);
  assert(weaponAttack(std::numeric_limits<int64_t>::max(),0)==std::numeric_limits<int64_t>::max());
  bool overflow=false;try{weaponAttack(std::numeric_limits<int64_t>::max(),15);}catch(const std::overflow_error&){overflow=true;}assert(overflow);
  for(int bad:{-1,16}){bool rejected=false;try{armorHealth(100,bad);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);}
  bool max=false;try{quote(15);}catch(const std::invalid_argument&){max=true;}assert(max);
  std::cout<<"REFINEMENT_POLICY_0130_OK\n";
}
