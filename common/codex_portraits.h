#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <zlib.h>
namespace destinia {
constexpr size_t codexRowsPerPage=4;
struct CodexPortrait { int width=0,height=0;std::vector<uint32_t> pixels; }; // bit 16 = opaque, low 16 = RGB565
using CodexPortraits=std::map<int,CodexPortrait>;
// Same resource selection as Monster_loadMonster / Monster_loadBoss in APK 1.0.6.
inline std::pair<std::string,std::string> codexPortraitAssets(int id) {
 if(id>=1 && id<=63) {const auto group=std::to_string((id-1)/3+1);return {"mon"+group+"_"+std::to_string((id-1)%3)+"_head.wmb","mon"+group+".agd"};}
 if(id>=100 && id<=109) {const auto stem="boss"+std::to_string(id==103?1:id-100);return {stem+"_head.wmb",stem+".agd"};}
 return {};
}
inline std::vector<uint8_t> codexUnpackResource(const std::vector<uint8_t>& bytes) {
 constexpr size_t limit=2*1024*1024;
 if(bytes.empty() || bytes.size()>limit)return {};
 if(bytes.size()<4 || bytes[0]!='Z' || bytes[1]!='C' || bytes[2]!='P' || bytes[3]!=0)return bytes;
 if(bytes.size()<12)return {};
 auto u32=[&](int p){return uint32_t(bytes[p])|(uint32_t(bytes[p+1])<<8)|(uint32_t(bytes[p+2])<<16)|(uint32_t(bytes[p+3])<<24);};
 const size_t packed=u32(4),size=u32(8);if(!size || size>limit || packed!=bytes.size()-12)return {};
 std::vector<uint8_t> out(size);uLongf length=size;
 if(uncompress(out.data(),&length,bytes.data()+12,packed)!=Z_OK || length!=size)return {};
 return out;
}
// Decode WBI indexed sheets and compose only body group 0 of the first AGD pose.
// Shadows/effects are deliberately excluded. No ARM memory or game state is used.
inline CodexPortrait codexDecodePortrait(const std::vector<uint8_t>& w,const std::vector<uint8_t>& a) {
 if(w.size()<8 || w.size()>2*1024*1024 || w[0]!='W' || w[1]!='B' || w[2]!='I' || w[3]!=0 || a.size()<12 || !a[0] || !a[2] || !a[11] || a[11]>64 || 12+size_t(a[11])*8>a.size())return {};
 auto u16=[](const std::vector<uint8_t>& b,size_t p){return int(b[p])|(int(b[p+1])<<8);};
 struct Frame{int x,y,w,h;std::vector<uint32_t> pixels;};std::vector<Frame> frames;
 size_t pos=8,total=0;const int count=u16(w,4);if(count<1 || count>256)return {};
 std::vector<uint32_t> palette;
 for(int i=0;i<count;++i) {
  if(pos+7>w.size())return {};
  Frame f{int(int8_t(w[pos])),int(int8_t(w[pos+1])),u16(w,pos+2),u16(w,pos+4),{}};int colors=w[pos+6];pos+=7;
  if(f.w<1 || f.h<1 || f.w>256 || f.h>256 || (total+=f.w*f.h)>2*1024*1024)return {};
  if(colors) {++colors;if(pos+colors*3>w.size())return {};palette.clear();for(int j=0;j<colors;++j){const auto b=w[pos++],g=w[pos++],r=w[pos++];palette.push_back(j?0x10000u|((r>>3)<<11)|((g>>2)<<5)|(b>>3):0);}}
  if(palette.empty() || pos+f.w*f.h>w.size())return {};
  f.pixels.reserve(f.w*f.h);for(int j=0;j<f.w*f.h;++j){const int index=w[pos++];if(index>=int(palette.size()))return {};f.pixels.push_back(palette[index]);}frames.push_back(std::move(f));
 }
 if(pos!=w.size())return {};
 constexpr int side=512,origin=256;std::vector<uint32_t> canvas(side*side);
 for(int i=0;i<a[11];++i) {
  const size_t at=12+i*8;const int flags=a[at+4];if(flags&63)continue; // Other image groups are effects/shadows.
  const int index=a[at+5];if(index>=int(frames.size()) || a[at+7]!=0)return {};
  const auto& f=frames[index];const bool flipX=flags&64,flipY=flags&128;
  const int x=int(int16_t(u16(a,at)))+(flipX?f.x-f.w+1:-f.x)+origin,y=int(int16_t(u16(a,at+2)))+(flipY?f.y-f.h+1:-f.y)+origin;
  if(x<0 || y<0 || x+f.w>side || y+f.h>side)return {};
  for(int yy=0;yy<f.h;++yy)for(int xx=0;xx<f.w;++xx){auto c=f.pixels[(flipY?f.h-1-yy:yy)*f.w+(flipX?f.w-1-xx:xx)];if(c)canvas[(y+yy)*side+x+xx]=c;}
 }
 int left=side,top=side,right=-1,bottom=-1;for(int y=0;y<side;++y)for(int x=0;x<side;++x)if(canvas[y*side+x]){left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);}
 if(right<left)return {};CodexPortrait result;result.width=right-left+1;result.height=bottom-top+1;result.pixels.reserve(result.width*result.height);
 for(int y=top;y<=bottom;++y)for(int x=left;x<=right;++x)result.pixels.push_back(canvas[y*side+x]);return result;
}
} // namespace destinia
