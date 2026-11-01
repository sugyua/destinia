#include "../common/auto_potion.h"
#include <cstdint>
#include <functional>
#include <map>
struct Fixture {
  static constexpr uint32_t g=0x100000;
  std::map<uint32_t,uint16_t> memory;
  destinia::AutoPotionController potion;
  int calls[2]={0,0}, writes=0; bool available[2]={false,false};
  Fixture(){memory[g+0x4854]=0;memory[g+0x48f2]=189;memory[g+0x48f4]=500;memory[g+0x48f6]=189;memory[g+0x48f8]=1000;}
  bool run(){return potion.update(g,[&](uint32_t a){return memory[a];},[&](uint32_t a,uint16_t v){++writes;memory[a]=v;},[&](int type){++calls[type];return available[type];},{});}
};
int main(){
 Fixture f; if(f.run() || f.calls[0]!=1 || f.calls[1]!=0 || f.writes || f.memory[f.g+0x48f2]!=189) return 1;
 Fixture h; h.available[0]=true; if(!h.run() || h.calls[0]!=1 || h.calls[1]!=0 || h.writes || h.memory[h.g+0x48f2]!=189) return 2;
 Fixture m; m.memory[m.g+0x48f4]=1000;m.memory[m.g+0x48f2]=10;m.available[1]=true;if(!m.run() || m.calls[0]!=0 || m.calls[1]!=1 || m.writes) return 3;
 Fixture d; d.memory[d.g+0x48f4]=0;d.memory[d.g+0x48f2]=10;d.available[1]=true;if(d.run() || d.calls[0] || d.calls[1] || d.writes) return 4;
 Fixture negative; negative.memory[negative.g+0x48f4]=65535;negative.memory[negative.g+0x48f2]=10;negative.available[1]=true;if(negative.run() || negative.calls[0] || negative.calls[1]) return 6;
 Fixture p; p.memory[p.g+0x4854]=1;p.available[0]=true;if(p.run() || p.calls[0] || p.calls[1]) return 5;
 Fixture above;above.memory[above.g+0x48f4]=501;above.memory[above.g+0x48f8]=1001;above.available[0]=true;if(above.run() || above.calls[0])return 7;
 Fixture below;below.memory[below.g+0x48f4]=500;below.memory[below.g+0x48f8]=1001;below.available[0]=true;if(!below.run() || below.calls[0]!=1)return 8;
 Fixture full;full.memory[full.g+0x48f4]=1000;full.potion.config.hpThresholdPercent=100;full.available[0]=true;if(full.run() || full.calls[0])return 9;
 Fixture missing;missing.memory[missing.g+0x48f8]=0;missing.available[0]=true;if(missing.run() || missing.calls[0])return 10;
 Fixture manaOnly;manaOnly.memory[manaOnly.g+0x48f8]=0;manaOnly.memory[manaOnly.g+0x48f2]=10;manaOnly.available[1]=true;if(!manaOnly.run() || manaOnly.calls[0] || manaOnly.calls[1]!=1)return 11;
 Fixture disabled;disabled.potion.config.enabled=false;disabled.available[0]=true;if(disabled.run() || disabled.calls[0])return 12;
 Fixture cooldown;cooldown.available[0]=true;cooldown.potion.cooldownHpTimer=2;if(cooldown.run() || cooldown.calls[0])return 13;
 Fixture retry;if(retry.run() || retry.calls[0]!=1)return 14;
 for(int i=0;i<9;++i)if(retry.run() || retry.calls[0]!=1)return 15;
 if(retry.run() || retry.calls[0]!=2)return 16;
 Fixture notice;notice.available[0]=true;if(!notice.run() || notice.potion.noticeTimer!=60)return 17;
 std::vector<uint8_t> pixels(400*240*2);
 destinia::originalPanelPainter=[](uint8_t*,int,int,int,int,int,int){return true;};
 destinia::originalTextPainter=[](uint8_t*,int,int,int,int,const std::string&,uint16_t,int,int){return false;};
 notice.potion.paintNotice(pixels.data(),400,240);
 if(!std::any_of(pixels.begin(),pixels.end(),[](uint8_t b){return b;}))return 18;
 Fixture stop;stop.memory[stop.g+0x48f2]=10;
 auto stopRead=[&](uint32_t a){return stop.memory[a];};
 auto stopWrite=[&](uint32_t a,uint16_t v){stop.memory[a]=v;};
 auto stopUse=[&](int type){++stop.calls[type];stop.potion.config.enabled=false;return false;};
 if(stop.potion.update(stop.g,stopRead,stopWrite,stopUse,{}) || stop.calls[0]!=1 || stop.calls[1]!=0)return 19;
 return 0;
}
