#include "../common/codex_native_layout.h"
#include <cassert>
int main(){for(const auto size:{std::pair<int,int>{400,240},{800,480}})for(int scale:{75,100,125}){
 auto first=destinia::nativeCodexTabRect(0,size.first,size.second),last=destinia::nativeCodexTabRect(9,size.first,size.second),bar=destinia::nativeCodexBarRect(size.first,size.second);
 assert(bar.x*2+bar.w==size.first);assert(first.x+last.x+last.w==size.first);
 auto t=destinia::nativeCodexTransform(size.first,size.second,scale);auto bounds=t.rect(first.x,first.y,last.x+last.w-first.x,24);assert(std::abs(bounds.x*2+bounds.w-size.first)<=1);
 for(int i=0;i<10;i++){auto b=destinia::nativeCodexTabRect(i,size.first,size.second);auto raster=t.rect(b.x,b.y,b.w,b.h);for(int x:{raster.x,raster.x+raster.w-1}){auto q=t.inverse(x,raster.y+raster.h/2);assert(destinia::nativeCodexTab(q.first,q.second,size.first,size.second)==i);}}
}}
