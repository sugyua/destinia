#pragma once
#include "codex_ui.h"
namespace destinia {
inline CodexRect nativeCodexRect(int x,int y,int w,int h,int sw,int sh){return {x+(sw-400)/2,y+(sh-240)/2,w,h};}
inline UiTransform nativeCodexTransform(int w,int h,int percent){return {w,h,percent,UiAnchor::Center,232,240};}
inline int nativeCodexTab(int x,int y,int w,int h){x-=(w-400)/2;y-=(h-240)/2;return x>=86&&x<314&&y>=0&&y<24?std::min(9,(x-86)/22):-1;}
inline CodexRect nativeCodexBack(int w,int h){return nativeCodexRect(272,29,34,18,w,h);}
inline CodexRect nativeCodexPrevious(int w,int h){return nativeCodexRect(92,215,62,20,w,h);}
inline CodexRect nativeCodexNext(int w,int h){return nativeCodexRect(249,215,58,20,w,h);}
inline CodexRect nativeCodexInfo(int w,int h){return nativeCodexRect(238,189,69,20,w,h);}
inline CodexRect nativeCodexUp(int w,int h){return nativeCodexRect(92,193,58,18,w,h);}
inline CodexRect nativeCodexDown(int w,int h){return nativeCodexRect(250,193,57,18,w,h);}
inline CodexRect nativeCodexCell(size_t i,int w,int h){return nativeCodexRect(96+int(i%8)*26,52+int(i/8)*26,24,24,w,h);}
inline int nativeCodexHit(int x,int y,size_t count,size_t page,int w,int h){for(size_t i=0;i<32&&page*32+i<count;i++)if(nativeCodexCell(i,w,h).contains(x,y))return int(page*32+i);return -1;}
}
