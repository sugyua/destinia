#pragma once
#include "ui_transform.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
namespace destinia {
using CombatStats=std::array<int,13>;
// Informational index v1. Derived attributes already include equipment/buffs;
// primary attributes are not counted again. HP/MP are maxima, never current HP.
inline int64_t combatPower(const CombatStats& stats){auto s=[&](int i){return int64_t(std::max(0,stats[i]));};return 4*s(7)+3*s(8)+s(5)/10+s(6)/10+s(9)/10+s(10)/10+s(11)/5+s(12)/10;}
struct CombatPreview {bool valid=false;CombatStats before{},after{};int64_t current=0,projected=0;std::string reason;int64_t delta()const{return projected-current;}};
struct CombatPanel {int x=0,y=0,w=232;};
inline std::string signedPower(int64_t n){return (n>0?"+":"")+std::to_string(n);}
}
