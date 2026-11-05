#include "../common/boss_rush_ui.h"
#include <cstdint>
#include <string>
int main(){
 destinia::BossRushUI boss;boss.open=true;boss.gameBase=0x1000;boss.currentTab=3;
 int gold=15113,writes=0,teleports=0;
 boss.getGold=[&](){return gold;};boss.setGold=[&](int g){gold=g;};
 boss.rd16=[](uint32_t){return uint16_t(19);};boss.wr16=[&](uint32_t,uint16_t){++writes;};
 for(int selected:{0,1}){boss.selectedIndex=selected;boss.handleKey(17,true);if(gold!=15113||writes||boss.statusMessage!="RESET INDISPONIVEL")return 1+selected;}
 boss.currentTab=0;boss.selectedIndex=0;boss.teleport=[&](int,int,int){++teleports;};boss.handleKey(17,true);
 if(gold!=14613||teleports!=1)return 3;
 return 0;
}
