#pragma once
#include "title_menu_model.h"
#include "codex_native_layout.h"
#include "original_ui.h"
#include "overlay_font.h"

namespace destinia {
enum class TitleHitKind { None, Previous, Next, Row };
struct TitleHit { TitleHitKind kind=TitleHitKind::None; int row=-1; };
inline CodexRect titleExtraRow(int row,int w,int h){return nativeCodexRect(96,58+13*row,208,13,w,h);}
inline CodexRect titleExtraPrevious(int w,int h){return nativeCodexRect(166,170,30,19,w,h);}
inline CodexRect titleExtraNextOriginal(int w,int h){return nativeCodexRect(209,170,30,19,w,h);}
inline TitleHit titleExtraHit(int x,int y,int w,int h,int percent){
 const auto p=nativeCodexTransform(w,h,percent).inverse(x,y);
 if(titleExtraPrevious(w,h).contains(p.first,p.second))return {TitleHitKind::Previous,-1};
 if(titleExtraNextOriginal(w,h).contains(p.first,p.second))return {TitleHitKind::Next,-1};
 for(int i=0;i<8;++i)if(titleExtraRow(i,w,h).contains(p.first,p.second))return {TitleHitKind::Row,i};
 return {};
}
inline const char* extraTitleRequirement(TitleId id){
 switch(id){
  case TitleId::CacadorDeFungos:return "Derrote 10 Fungos. +10% critico.";
  case TitleId::Iniciante:return "Disponivel desde o inicio.";
  case TitleId::Cacador:return "Derrote 51 inimigos comuns.";
  case TitleId::Complecionista:return "Descubra todos no Codex.";
  case TitleId::MestreDaSobrevivencia:return "Conclua a onda 20.";
  case TitleId::SenhorDoAbismo:return "Derrote Diablo no modo Abissal.";
  default:return "Desbloqueio ainda nao implementado.";
 }
}
inline bool paintTitleExtra(uint8_t* pixels,int w,int h,int percent,const TitleMenuModel& menu,const TitlesStore& titles,const std::string& message={}){
 if(!pixels||!menu.open||menu.slot<0||menu.selected>=menu.count())return false;
 const auto tr=nativeCodexTransform(w,h,percent);const auto at=[&](int x,int y){return nativeCodexRect(x,y,0,0,w,h);};
 // Render once at native resolution, then scale the complete panel. Scaling
 // positions alone leaves the original bitmap font unreadably small at 125%.
 std::vector<uint8_t> raster(size_t(w)*h*2);
 const auto rect=[&](int x,int y,int rw,int rh,uint16_t c){const auto base=at(x,y);for(int yy=std::max(0,base.y);yy<std::min(h,base.y+rh);++yy)for(int xx=std::max(0,base.x);xx<std::min(w,base.x+rw);++xx){const auto k=(size_t(yy)*w+xx)*2;raster[k]=uint8_t(c);raster[k+1]=uint8_t(c>>8);}};
 const auto panel=[&](int x,int y,int rw,int rh){const auto base=at(x,y);if(!paintOriginalPanel(raster.data(),w,h,base.x,base.y,rw,rh))rect(x,y,rw,rh,0x0841);};
 const auto text=[&](int x,int y,const std::string& s,uint16_t color,int limit=208){const auto base=at(x,y);if(!paintOriginalText(raster.data(),w,h,base.x,base.y,s,color,limit,1))paintOverlayText(raster.data(),w,h,base.x,base.y,s,color);};
 panel(84,25,232,215);panel(91,48,218,122);panel(95,190,210,32);
 text(95,32,"Selecione um titulo.",0xffff);
 const size_t first=menu.selected/8*8;
 for(size_t row=0;row<8&&first+row<menu.count();++row){const size_t i=first+row;const auto& def=allTitles().at(i+1);const bool sel=i==menu.selected,unlocked=titles.isUnlocked(menu.slot,def.id);if(sel)rect(96,58+int(row)*13,208,13,0xfff0);text(100,59+int(row)*13,def.nome,sel?0x0000:(unlocked?0xffff:0x8c51));}
 text(173,174,"<",0xffff);text(190,174,std::to_string(8+first/8)+" / "+std::to_string(7+(menu.count()+7)/8),0xffff);if(first+8<menu.count())text(220,174,">",0xffff);
 const auto& def=allTitles().at(menu.selected+1);text(100,196,extraTitleRequirement(def.id),0xffff,198);
 if(!message.empty())text(100,208,message,0xf800,198);
 const auto cursor=at(288,59+int(menu.selected%8)*13);paintOriginalCursor(raster.data(),w,h,cursor.x,cursor.y);
 const auto origin=at(84,25);const auto area=tr.rect(origin.x,origin.y,232,215);
 for(int y=std::max(0,area.y);y<std::min(h,area.y+area.h);++y)for(int x=std::max(0,area.x);x<std::min(w,area.x+area.w);++x){
  const auto source=tr.inverse(x,y);if(source.first<origin.x||source.first>=origin.x+232||source.second<origin.y||source.second>=origin.y+215)continue;
  const auto src=(size_t(source.second)*w+source.first)*2,dst=(size_t(y)*w+x)*2;pixels[dst]=raster[src];pixels[dst+1]=raster[src+1];
 }
 return true;
}
}
