#pragma once
#include "codex_portraits.h"
#include <functional>
#include <optional>
#include <stdexcept>
#include <limits>
namespace destinia {
struct MageSite {int area=0,map=65,x=0,y=0;};
struct BossMageArt {CodexPortrait body,shadow;};
inline void validateMageArt(const CodexPortrait& art){if(art.width<1||art.height<1||art.width>256||art.height>256||art.pixels.size()!=size_t(art.width*art.height))throw std::invalid_argument("Mago: arte invalida");}
inline uint16_t bossMageColor(uint16_t color){
 const int r=(color>>11)*8,g=((color>>5)&63)*4,b=(color&31)*8,luma=(3*r+6*g+b)/10;
 static constexpr uint16_t ramp[]={0x0823,0x1048,0x18ee,0x22b6,0x449d,0x8e7f,0xd7bf,0xffff};
 const int index=std::min(7,luma/32);return ramp[r>g+40&&r>b+40?std::min(2,index):index];
}
inline CodexPortrait bossMagePalette(const CodexPortrait& source){validateMageArt(source);return source;}
inline BossMageArt loadBossMageArt(const std::vector<uint8_t>& body,const std::vector<uint8_t>& animation,const std::vector<uint8_t>& shadow){
 auto pose=codexDecodePortrait(codexUnpackResource(body),codexUnpackResource(animation));validateMageArt(pose);
 std::vector<uint8_t> single(20);single[0]=1;single[2]=1;single[11]=1;
 auto shade=codexDecodePortrait(codexUnpackResource(shadow),single,0);validateMageArt(shade);

 return {bossMagePalette(pose),std::move(shade)};
}
inline bool mageFacing(const MageSite& npc,int x,int y,int facing){
 const int64_t dx=int64_t(npc.x)-x,dy=int64_t(npc.y)-y;if(std::abs(dx)>28||std::abs(dy)>28)return false;
 return facing==0?dx<0:facing==1?dx>0:facing==2?dy<0:facing==3?dy>0:false;
}
// Original MPD: dimensions, resource IDs, four byte planes, objects, events,
// exits, NPCs and spawns. Plane 3 is the original 0=walkable/1=blocked grid.
inline std::optional<MageSite> placeBossMage(int area,int map,const std::vector<uint8_t>& bytes){
 if(area<0||area>2||map<0||map>255||bytes.size()>2*1024*1024)throw std::invalid_argument("Mago: mapa invalido");
 size_t at=0;const auto need=[&](size_t n){if(at>bytes.size()||n>bytes.size()-at)throw std::invalid_argument("Mago: MPD truncado");};
 const auto take=[&](){need(1);return int(bytes[at++]);};const auto skip=[&](size_t n){need(n);at+=n;};
 const int w=take(),h=take(),p=take(),q=take();if(!w||!h)throw std::invalid_argument("Mago: dimensoes invalidas");skip(size_t(p)+q);need(size_t(w)*h*4);const size_t grid=at+size_t(w)*h*3;skip(size_t(w)*h*4);
 std::vector<std::pair<int,int>> occupied;
 const auto records=[&](int stride,int coordinateOffset){const int count=take();for(int i=0;i<count;++i){need(stride);occupied.push_back({bytes[at+coordinateOffset]*16+8,bytes[at+coordinateOffset+1]*16+8});skip(stride);}};
 records(3,1);skip(2);records(7,0);records(5,0);records(6,0);skip(12);const int spawns=take();skip(size_t(spawns)*4);if(at!=bytes.size())throw std::invalid_argument("Mago: dados MPD extras");
 if(spawns)return std::nullopt;
 if(area==0){
  if(map==61)return MageSite{area,map,280,232};
  if(map==62)return MageSite{area,map,504,280};
  if(map==63)return MageSite{area,map,280,296};
  if(map==64)return MageSite{area,map,344,424};
  if(map==65)return MageSite{area,map,328,344};
 }
 std::optional<MageSite> result;int score=std::numeric_limits<int>::max();
 for(int y=4;y<h-2;++y)for(int x=2;x<w-2;++x){bool clear=true;
  for(int yy=y-3;yy<=y+1;++yy)for(int xx=x-1;xx<=x+1;++xx)if(bytes[grid+size_t(yy)*w+xx]!=0)clear=false;
  const int px=x*16+8,py=y*16+8;for(const auto& point:occupied)if(std::abs(px-point.first)<64&&std::abs(py-point.second)<64)clear=false;
  const int distance=(2*x-w)*(2*x-w)+(2*y-h)*(2*y-h);if(clear&&distance<score){score=distance;result=MageSite{area,map,px,py};}
 }return result;
}
inline void paintBossMage(uint8_t* pixels,int width,int height,const CodexPortrait& body,const CodexPortrait& shadow,int x,int y){
 if(!pixels||width<=0||height<=0||width>4096||height>4096)throw std::invalid_argument("Mago: raster invalido");validateMageArt(body);validateMageArt(shadow);
 const auto paint=[&](const CodexPortrait& art,int64_t left,int64_t top){for(int yy=0;yy<art.height;++yy)for(int xx=0;xx<art.width;++xx){const auto c=art.pixels[size_t(yy)*art.width+xx];const int64_t dx=left+xx,dy=top+yy;if((c&0x10000)&&dx>=0&&dy>=0&&dx<width&&dy<height){const auto at=(size_t(dy)*width+size_t(dx))*2;pixels[at]=uint8_t(c);pixels[at+1]=uint8_t(c>>8);}}};
 paint(shadow,int64_t(x)-shadow.width/2,int64_t(y)-shadow.height/2);paint(body,int64_t(x)-body.width/2,int64_t(y)-body.height);
}
// Dispatch outside Unicorn callbacks. UI binds its opener; absence keeps a
// request pending rather than pretending a menu opened. No native state changes.
struct BossRushMenuEvent {
 bool pending=false,dispatching=false;std::function<void()> opener;
 void openBossRushMenu(){pending=true;}
 void dispatch(){if(!pending||!opener||dispatching)return;dispatching=true;try{opener();pending=false;dispatching=false;}catch(...){dispatching=false;throw;}}
};
}
