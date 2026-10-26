#include "../common/boss_mage.h"
#include <cassert>
#include <fstream>
using namespace destinia;
int main(int argc,char** argv){
 CodexPortrait source{2,2,{0,0x10000u|0xffffu,0x10000u|0xf800u,0x10000u|0x8410u}};auto copy=source;
 auto blue=bossMagePalette(source);assert(source.pixels==copy.pixels&&blue.pixels[0]==0&&blue.width==2&&blue.height==2);assert(blue.pixels[2]!=source.pixels[2]);
 for(size_t i=0;i<4;i++)assert(bool(blue.pixels[i]&0x10000)==bool(source.pixels[i]&0x10000));
 MageSite site{0,65,200,200};assert(mageFacing(site,200,216,2));assert(!mageFacing(site,200,216,3));assert(!mageFacing(site,200,217,2));
 assert(mageFacing(site,184,200,1)&&mageFacing(site,216,200,0)&&mageFacing(site,200,184,3));
 std::vector<uint8_t> guarded(400*240*2+32,0x35);paintBossMage(guarded.data()+16,400,240,blue,source,-1,1);for(int i=0;i<16;i++)assert(guarded[i]==0x35&&guarded[guarded.size()-1-i]==0x35);
 if(argc>1){const auto root=std::string(argv[1]);auto read=[&](const std::string& name){std::ifstream in(root+"/"+name,std::ios::binary);return std::vector<uint8_t>{std::istreambuf_iterator<char>(in),{}};};
  auto raw=read("mon6_0_head.wmb"),before=raw;auto art=loadBossMageArt(raw,read("mon6.agd"),read("shadow.wmb"));assert(art.body.width==25&&art.body.height==42&&raw==before&&!art.shadow.pixels.empty());
  for(int map:{61,62,63,64,65}){auto bytes=codexUnpackResource(read("m0_"+std::to_string(map)+".mpd"));auto p=placeBossMage(0,map,bytes);assert(p);if(p){assert(p->map==map);std::printf("SITE %d %d %d\n",map,p->x,p->y);}}
 }
 BossRushMenuEvent event;event.openBossRushMenu();assert(event.pending);int opens=0;event.opener=[&](){++opens;};event.dispatch();assert(opens==1&&!event.pending);event.dispatch();assert(opens==1);
}
