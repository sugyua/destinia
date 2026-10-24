#pragma once
#include "overlay_font.h"
#include "ui_transform.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
namespace destinia {
using CombatStats=std::array<int,13>;
// Informational index v1. Derived attributes already include equipment/buffs;
// primary attributes are not counted again. HP/MP are maxima, never current HP.
inline int64_t combatPower(const CombatStats& stats){auto s=[&](int i){return int64_t(std::max(0,stats[i]));};return 4*s(7)+3*s(8)+s(5)/10+s(6)/10+s(9)/10+s(10)/10+s(11)/5+s(12)/10;}
struct CombatPreview {bool valid=false;CombatStats before{},after{};int64_t current=0,projected=0;std::string reason;int64_t delta()const{return projected-current;}};
struct CombatPanel {int x=0,y=0,w=232;};
inline std::string signedPower(int64_t n){return (n>0?"+":"")+std::to_string(n);}
inline void paintCombatPower(uint8_t* pixels,int width,int height,int percent,CombatPanel panel,const CombatPreview& info,bool comparison,bool portuguese=true){
 UiTransform transform{width,height,percent,UiAnchor::Center,232,240};
 auto rect=[&](int x,int y,int w,int h,uint16_t color){auto area=transform.rect(x,y,w,h);for(int yy=std::max(0,area.y);yy<std::min(height,area.y+area.h);++yy)for(int xx=std::max(0,area.x);xx<std::min(width,area.x+area.w);++xx){const size_t at=size_t(yy*width+xx)*2;pixels[at]=color&255;pixels[at+1]=color>>8;}};
 auto text=[&](int x,int y,const std::string& str,uint16_t c){for(size_t at=0;at<str.size();x+=6){auto glyph=overlayGlyph(overlayCodepoint(str,at));for(int i=0;i<5;i++)for(int j=0;j<11;j++)if(glyph[i]&(1<<j))rect(x+i,y+j-2,1,1,c);}};
 if(!comparison){rect(panel.x+98,panel.y+22,114,13,0x1946);text(panel.x+101,panel.y+25,"FC "+std::to_string(info.current),0xffff);return;}
 const int x=panel.x-82,y=height/2-66;rect(x,y,80,96,0x1946);text(x+4,y+5,portuguese?"COMPARAR":"COMPARE",0xe5b2);text(x+4,y+17,"FC "+std::to_string(info.current),0xffff);
 if(!info.valid){std::string reason=info.reason;if(!portuguese){if(reason=="OUTRA CLASSE")reason="OTHER CLASS";else if(reason=="NIVEL BAIXO")reason="LOW LEVEL";else if(reason=="IDENTIFIQUE")reason="IDENTIFY";else reason="UNVERIFIED";}text(x+4,y+31,overlayTextClip(reason,12),0xf800);return;}
 text(x+4,y+29,std::string(portuguese?"NOVO ":"NEW ")+std::to_string(info.projected),0xffff);text(x+4,y+41,signedPower(info.delta()),info.delta()>0?0x07e0:info.delta()<0?0xf800:0x8c92);
 int row=0;for(const auto& label:std::array<std::pair<int,const char*>,5>{{{7,"ATK"},{8,"DEF"},{5,"PV"},{6,"PM"},{11,"CRI"}}}){const int difference=info.after[label.first]-info.before[label.first];text(x+4,y+51+9*row++,std::string(!portuguese&&label.first==5?"HP":!portuguese&&label.first==6?"MP":label.second)+" "+(label.first==11?std::string(difference<0?"-":difference>0?"+":"")+std::to_string(std::abs(difference)/10)+"."+std::to_string(std::abs(difference)%10):signedPower(difference)),difference>0?0x07e0:difference<0?0xf800:0x8c92);}
}
}
