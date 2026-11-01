#include "../common/auto_potion_native.h"
#include <map>
#include <cstdint>
#include <vector>
#include <stdexcept>
int main(){
 std::vector<uint8_t> table(158*166,0);table[5*166]=5;table[5*166+4]=1;table[5*166+6]=0;
 if(!destinia::validPotionData(table,5,1) || destinia::validPotionData(table,5,0)) return 6;
 table[5*166+4]=2;if(destinia::validPotionData(table,5,1)) return 7;
 constexpr uint32_t g=0x100000;std::map<uint32_t,uint16_t> m;
 m[g+0x4a8c]=0; m[g+0x4a90]=0x6123; m[g+0x433c]=2;
 m[g+0x433e]=1;m[g+0x4340]=9;m[g+0x4342]=4;
 m[g+0x4344]=1;m[g+0x4346]=0;m[g+0x4348]=3;
 auto read=[&](uint32_t a){return m[a];};auto write=[&](uint32_t a,uint16_t v){m[a]=v;};
 int calls=0;auto invoke=[&](){++calls;if(m[g+0x4a90]!=0x4000)return false;--m[g+0x4348];return true;};
 auto valid=[](uint16_t id,int type){return type==0&&id==0;};
 if(!destinia::useNativePotion(g,0,read,write,invoke,valid) || calls!=1 || m[g+0x4348]!=2 || m[g+0x4a90]!=0x6123) return 1;
 m[g+0x4348]=0;if(destinia::useNativePotion(g,0,read,write,invoke,valid) || calls!=1 || m[g+0x4a90]!=0x6123) return 2;
 m[g+0x4a8c]=65535;if(destinia::useNativePotion(g,0,read,write,invoke,valid) || calls!=1) return 3;
 m[g+0x4a8c]=0;m[g+0x433c]=25;if(destinia::useNativePotion(g,0,read,write,invoke,valid) || calls!=1) return 4;
 m[g+0x433c]=2;m[g+0x4348]=3;
 auto reject=[](uint16_t,int){return false;};
 if(destinia::useNativePotion(g,0,read,write,invoke,reject) || calls!=1 || m[g+0x4348]!=3) return 8;
 try {destinia::useNativePotion(g,0,read,write,[]()->bool{throw std::runtime_error("native error");},valid);return 9;}
 catch(const std::runtime_error&) {if(m[g+0x4a90]!=0x6123)return 10;}
 // A failed write may have modified memory before reporting failure: restore it.
 int writes=0;
 auto partialWrite=[&](uint32_t a,uint16_t v){m[a]=v;if(++writes==1)throw std::runtime_error("partial write");};
 try {destinia::useNativePotion(g,0,read,partialWrite,invoke,valid);return 11;}
 catch(const std::runtime_error&) {if(m[g+0x4a90]!=0x6123)return 12;}
 // An unverified restore must be distinguished from an ordinary read error.
 writes=0;
 auto badRestore=[&](uint32_t a,uint16_t v){if(++writes==2)throw std::runtime_error("restore failed");m[a]=v;};
 try {destinia::useNativePotion(g,0,read,badRestore,invoke,valid);return 13;}
 catch(const destinia::AutoPotionRestoreError&) {}
 catch(...) {return 14;}
 m[g+0x4a90]=0x6123;writes=0;
 auto silentBadRestore=[&](uint32_t a,uint16_t v){if(++writes!=2)m[a]=v;};
 try {destinia::useNativePotion(g,0,read,silentBadRestore,invoke,valid);return 15;}
 catch(const destinia::AutoPotionRestoreError&) {}
 catch(...) {return 16;}
 return 0;
}
