#include "../common/loot_qol.h"
#include <cassert>
int main(){using namespace destinia;LootOptions o;assert(!o.visual&&o.autoGold&&!o.autoMaterials);std::map<int,int> types{{79,14},{0,0},{119,22}};
 auto gold=describeDrop(5,50,types),material=describeDrop(1,79,types),potion=describeDrop(1,0,types),gear=describeDrop(0,3<<8,types);
 assert(gold.category==4&&material.category==2&&potion.category==16&&gear.rank==3);assert(autoEligible(gold,o,0,0));o.autoGold=false;assert(!autoEligible(gold,o,0,0));o.autoGold=o.autoMaterials=true;
 assert(autoEligible(gold,o,19,-19)&&!autoEligible(gold,o,20,0));assert(autoEligible(material,o,0,0)&&!autoEligible(potion,o,0,0)&&!autoEligible(gear,o,0,0));
 o.visual=true;o.categories=2;assert(!visibleDrop(gold,o)&&visibleDrop(material,o));o.categories=31;o.minRank=4;assert(!visibleDrop(gear,o));
 bool rejected=false;o.radius=20;try{validateLootOptions(o);}catch(...){rejected=true;}assert(rejected);
 for(int width:{400,800}){int height=width==400?240:480;std::vector<uint8_t> pixels(width*height*2+32,0x34);for(int x:{-10,0,width-1,width+10})for(int y:{-10,0,height-1,height+10})paintLootMarker(pixels.data()+16,width,height,x,y,lootColor(gold));for(int i=0;i<16;i++)assert(pixels[i]==0x34&&pixels[pixels.size()-1-i]==0x34);}
}
