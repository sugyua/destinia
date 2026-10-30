#include "../common/auto_potion.h"
#include <unordered_map>
#include <iostream>
#include <stdexcept>
struct Fixture {
 std::unordered_map<uint32_t,uint16_t> mem;
 destinia::AutoPotionController potion;
 static constexpr uint32_t g=0x10000,bag=g+0x433c+0x92;
 Fixture(uint16_t hp=500,uint16_t maximum=1000){mem[g+0x48f2]=hp;mem[g+0x48c4]=maximum;mem[g+0x48f6]=1000;mem[g+0x48c6]=1000;mem[bag]=1;mem[bag+6]=5;}
 bool update(){return potion.update(g,[&](uint32_t p){return mem[p];},[&](uint32_t p,uint16_t v){mem[p]=v;});}
};
static void require(bool yes,const char* what){if(!yes)throw std::runtime_error(what);}
int main(){try{
 Fixture defaults;require(defaults.potion.config.hpThresholdPercent==50,"default HP threshold must be 50 percent");
 require(defaults.update(),"exact 50 percent must trigger");require(defaults.mem[Fixture::g+0x48f2]==900,"expected threshold healing");require(defaults.mem[Fixture::bag+6]==4,"one fixture unit consumed");
 Fixture full(1000);full.potion.config.hpThresholdPercent=100;require(!full.update(),"full HP must not trigger");require(full.mem[Fixture::bag+6]==5,"full HP must preserve fixture quantity");
 Fixture missing(500,0);require(!missing.update(),"missing maximum must not trigger");require(missing.mem[Fixture::bag+6]==5,"missing maximum preserves quantity");
 Fixture mana(500,0);mana.mem[Fixture::g+0x48f6]=100;require(mana.update(),"missing HP maximum must not block mana");require(mana.mem[Fixture::g+0x48f6]==450,"mana must recover independently");require(mana.mem[Fixture::g+0x48f2]==500,"invalid HP maximum must preserve HP");require(mana.mem[Fixture::bag+6]==5,"mana must not enter HP inventory path");
 Fixture above(501,1001);require(!above.update(),"fraction above 50 percent must not be rounded down");
 Fixture below(500,1001);require(below.update(),"fraction below 50 percent must trigger");
 Fixture dead(0);require(!dead.update(),"dead hero must not trigger");
 Fixture disabled;disabled.potion.config.enabled=false;require(!disabled.update(),"disabled assistance must not trigger");
 Fixture paused;paused.mem[Fixture::g+0x4854]=1;require(!paused.update(),"non-gameplay state must not trigger");
 Fixture cooling;cooling.potion.cooldownHpTimer=2;require(!cooling.update(),"active cooldown must block");
 std::cout<<"AUTO_POTION_THRESHOLD_CASES_OK\n";return 0;
}catch(const std::exception& e){std::cout<<e.what()<<'\n';return 1;}}
