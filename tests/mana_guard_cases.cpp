#include "../common/mana_guard.h"
#include <cstdint>
#include <map>
int main(){
 constexpr uint32_t base=0x10000;std::map<uint32_t,uint16_t> m;
 auto read=[&](uint32_t a){return m[a];};int writes=0;auto write=[&](uint32_t a,uint16_t v){++writes;m[a]=v;};
 m[base+0x4854]=0;m[base+0x48fa]=19;m[base+0x48f4]=100;m[base+0x48f2]=769;m[base+0x48f6]=189;
 destinia::ManaGuard guard;for(int i=0;i<90;++i)guard.update(base,read,write);
 if(m[base+0x48f2]!=189||writes!=1)return 1;
 for(int i=0;i<90;++i)guard.update(base,read,write);if(writes!=1)return 2;
 m[base+0x48f2]=769;m[base+0x48f6]=0;for(int i=0;i<90;++i)guard.update(base,read,write);if(writes!=1)return 3;
 m[base+0x48f6]=189;m[base+0x4854]=1;for(int i=0;i<90;++i)guard.update(base,read,write);if(writes!=1)return 4;
 m[base+0x4854]=0;m[base+0x48f4]=0;for(int i=0;i<90;++i)guard.update(base,read,write);if(writes!=1)return 5;
 m[base+0x48f4]=100;m[base+0x48fa]=0;for(int i=0;i<90;++i)guard.update(base,read,write);if(writes!=1)return 6;
 m[base+0x48fa]=19;for(int i=0;i<20;++i)guard.update(base,read,write);m[base+0x48f6]=200;for(int i=0;i<20;++i)guard.update(base,read,write);if(writes!=1)return 7;
 destinia::ManaGuard switched;m[base+0x48f2]=769;m[base+0x48f6]=189;m[base+0x4886]=0;
 for(int i=0;i<59;++i)switched.update(base,read,write);
 m[base+0x4886]=1;switched.update(base,read,write);if(writes!=1)return 8;
 for(int i=0;i<59;++i)switched.update(base,read,write);if(writes!=2||m[base+0x48f2]!=189)return 9;
 return 0;
}
