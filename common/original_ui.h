#pragma once
#include <cstdint>
#include <functional>
#include <string>
namespace destinia {
// Runtime adapters use original ARM resources; tests without a runtime retain their raster fallback.
using OriginalPanelPainter=std::function<bool(uint8_t*,int,int,int,int,int,int)>;
using OriginalTextPainter=std::function<bool(uint8_t*,int,int,int,int,const std::string&,uint16_t,int,int)>;
inline std::function<bool(uint8_t*,int,int,int,int)> originalCursorPainter;
inline bool paintOriginalCursor(uint8_t* p,int w,int h,int x,int y){return originalCursorPainter&&originalCursorPainter(p,w,h,x,y);}
inline std::function<bool(uint8_t*,int,int,int,int,int,uint64_t,int)> originalCompanionPainter;
inline bool paintOriginalCompanion(uint8_t* p,int w,int h,int id,int x,int y,uint64_t frame,int level){return originalCompanionPainter&&originalCompanionPainter(p,w,h,id,x,y,frame,level);}
inline OriginalPanelPainter originalPanelPainter;
inline OriginalTextPainter originalTextPainter;
inline bool paintOriginalPanel(uint8_t* pixels,int width,int height,int x,int y,int w,int h){return originalPanelPainter&&originalPanelPainter(pixels,width,height,x,y,w,h);}
inline bool paintOriginalText(uint8_t* pixels,int width,int height,int x,int y,const std::string& value,uint16_t color,int limit=0,int scale=1){return originalTextPainter&&originalTextPainter(pixels,width,height,x,y,value,color,limit?limit:width-x,scale);}
}
