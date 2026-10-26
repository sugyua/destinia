#pragma once
#include "codex_ui.h"
namespace destinia {
inline CodexRect nativeCodexRect(int x,int y,int w,int h,int sw,int sh){return {x+(sw-400)/2,y+(sh-240)/2,w,h};}
inline CodexRect nativeCodexBarRect(int w,int h){return {(w-256)/2,(h-240)/2,256,25};}
inline CodexRect nativeCodexTabRect(int i,int w,int h){return {(w-230)/2+23*i,(h-240)/2,23,24};}
inline UiTransform nativeCodexTransform(int w,int h,int percent){return {w,h,percent,UiAnchor::Center,256,240,(w-256)/2,(h-240)/2};}
inline int nativeCodexTab(int x,int y,int w,int h){for(int i=0;i<10;++i)if(nativeCodexTabRect(i,w,h).contains(x,y))return i;return -1;}
inline CodexRect nativeCodexBack(int w,int h){return nativeCodexRect(272,29,34,18,w,h);}
inline CodexRect nativeCodexFilterTab(int filter,int w,int h){
 if(filter==0) return nativeCodexRect(136,31,44,14,w,h);
 if(filter==1) return nativeCodexRect(184,31,52,14,w,h);
 return nativeCodexRect(240,31,50,14,w,h);
}
inline int nativeCodexFilterHit(int x,int y,int w,int h){
 for(int f=0;f<3;++f) if(nativeCodexFilterTab(f,w,h).contains(x,y)) return f;
 return -1;
}
inline CodexRect nativeCodexPrevious(int w,int h){return nativeCodexRect(92,215,62,20,w,h);}
inline CodexRect nativeCodexNext(int w,int h){return nativeCodexRect(249,215,58,20,w,h);}
inline CodexRect nativeCodexInfo(int w,int h){return nativeCodexRect(238,189,69,20,w,h);}
inline CodexRect nativeCodexUp(int w,int h){return nativeCodexRect(92,193,58,18,w,h);}
inline CodexRect nativeCodexDown(int w,int h){return nativeCodexRect(250,193,57,18,w,h);}
inline CodexRect nativeCodexCell(size_t i,int w,int h){return nativeCodexRect(96+int(i%8)*26,52+int(i/8)*26,24,24,w,h);}
inline int nativeCodexHit(int x,int y,size_t count,size_t page,int w,int h){for(size_t i=0;i<32&&page*32+i<count;i++)if(nativeCodexCell(i,w,h).contains(x,y))return int(page*32+i);return -1;}
}
