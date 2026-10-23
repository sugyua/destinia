#pragma once
#include "codex.h"
#include "overlay_font.h"
#include "codex_portraits.h"
#include "codex_details.h"
#include <array>
#include <cctype>
namespace destinia {
struct CodexRect { int x,y,w,h; bool contains(int px,int py) const { return px>=x && py>=y && px<x+w && py<y+h; } };
// The original menu is 232x240, centered in both hosts. Append a book tab
// beside its nine icons; never replace an existing native tab/hitbox.
inline CodexRect codexMenuButton(int width,int height) { return {width/2+120,height/2-120,76,24}; }
inline CodexRect codexPanel(int width,int height) { return {(width-360)/2,(height-240)/2+25,360,215}; }
constexpr size_t codexGridColumns=8,codexGridRows=4,codexEntriesPerPage=codexGridColumns*codexGridRows;
inline CodexRect codexGrid(int width,int height){auto p=codexPanel(width,height);return {p.x+24,p.y+28,312,134};}
inline CodexRect codexCell(size_t index,int width,int height){auto g=codexGrid(width,height);return {g.x+int(index%codexGridColumns)*40,g.y+int(index/codexGridColumns)*34,32,32};}
inline CodexRect codexDetails(int width,int height){auto p=codexPanel(width,height);return {p.x+8,p.y+167,344,23};}
inline CodexRect codexPrevious(int width,int height){auto p=codexPanel(width,height);return {p.x+4,p.y+192,90,20};}
inline CodexRect codexNext(int width,int height){auto p=codexPanel(width,height);return {p.x+280,p.y+192,76,20};}
inline CodexRect codexBack(int width,int height){auto p=codexPanel(width,height);return {p.x+298,p.y+4,58,20};}
constexpr size_t codexDetailRowsPerPage=7;
inline CodexRect codexDetailUp(int width,int height){auto p=codexPanel(width,height);return {p.x+8,p.y+174,70,16};}
inline CodexRect codexDetailDown(int width,int height){auto p=codexPanel(width,height);return {p.x+282,p.y+174,70,16};}
inline std::vector<CodexInfoGroup> codexDetailPages(const CodexInfoSection& section){
 std::vector<CodexInfoGroup> pages;
 for(const auto& group:section.groups)for(size_t at=0;at<std::max<size_t>(1,group.rows.size());at+=codexDetailRowsPerPage){
  const auto end=std::min(at+codexDetailRowsPerPage,group.rows.size());
  pages.push_back({group.id,group.title,{group.rows.begin()+at,group.rows.begin()+end}});
 }
 if(pages.empty())pages.push_back({"","",{}});
 return pages;
}
inline int codexHitTest(int x,int y,size_t count,size_t page,int width,int height){
 for(size_t i=0;i<codexEntriesPerPage && page*codexEntriesPerPage+i<count;++i)if(codexCell(i,width,height).contains(x,y))return int(page*codexEntriesPerPage+i);
 return -1;
}
inline void codexPaint(uint8_t* pixels,int width,int height,const Codex& book,bool open,size_t page,bool available,bool inMenu=false,bool selected=false,const CodexPortraits* portraits=nullptr,size_t selectedIndex=0,bool detailsOpen=false,const CodexInfoView* info=nullptr,size_t detailPage=0,size_t detailScroll=0) {
 if(!open && !inMenu) return;
 auto pixel=[&](int x,int y,uint16_t c){if(x>=0&&x<width&&y>=0&&y<height){pixels[(y*width+x)*2]=c&255;pixels[(y*width+x)*2+1]=c>>8;}};
 auto rect=[&](int x,int y,int w,int h,uint16_t c){for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx)pixel(x+dx,y+dy,c);};
 auto text=[&](int x,int y,const std::string& value,uint16_t color,int scale=1){paintOverlayText(pixels,width,height,x,y,value,color,scale);};
 const uint16_t dark=0x1946,gold=0xe5b2,white=0xe71c,muted=0x8c92,border=0x6b6b;
 auto button=codexMenuButton(width,height);
 if(selected || open) {
  // Dim only the old toolbar highlight while the added tab has focus.
  for(int yy=height/2-120;yy<height/2-95;++yy)for(int xx=width/2-116;xx<width/2+116;++xx) {
   if(xx<0 || yy<0 || xx>=width || yy>=height)continue;
   const int at=(yy*width+xx)*2;uint16_t c=pixels[at]|(pixels[at+1]<<8);pixel(xx,yy,(c>>1)&0x7bef);
  }
 }
 rect(button.x,button.y,button.w,button.h,selected||open?gold:border);
 rect(button.x+1,button.y+1,button.w-2,button.h-2,dark);
 // Book icon and the new entry, entirely inside the game framebuffer.
 rect(button.x+6,button.y+6,13,12,gold);rect(button.x+7,button.y+7,5,9,dark);rect(button.x+13,button.y+7,5,9,dark);
 text(button.x+24,button.y+9,"CODEX",selected||open?gold:white);
 if(!open) return;
 auto panel=codexPanel(width,height);const int x=panel.x,y=panel.y;
 rect(x,y,panel.w,panel.h,border);rect(x+2,y+2,panel.w-4,panel.h-4,white);
 rect(x+5,y+5,panel.w-10,17,0);text(x+12,y+10,"CODEX - MONSTROS",gold);text(x+305,y+10,"VOLTAR",white);
 // Catalog order is independent of discovered records; an unseen record stays masked.
 std::vector<std::pair<int,std::string>> rows(book.names.begin(),book.names.end());
 const auto character=book.entries.find(book.slot);
 auto entry=[&](int id)->const CodexEntry*{if(!available || character==book.entries.end())return nullptr;auto found=character->second.find(id);return found==character->second.end()?nullptr:&found->second;};
 page=std::min(page,rows.empty()?size_t(0):(rows.size()-1)/codexEntriesPerPage);
 const size_t start=page*codexEntriesPerPage,end=std::min(start+codexEntriesPerPage,rows.size());
 selectedIndex=rows.empty()?0:std::max(start,std::min(selectedIndex,end-1));
 if(detailsOpen){
  const auto selectedEntry=rows.empty()?nullptr:entry(rows[selectedIndex].first);
  if(!selectedEntry)info=nullptr;
  rect(x+5,y+5,panel.w-10,17,0);text(x+12,y+10,"CÓDEX - INFORMAÇÕES",gold);text(x+305,y+10,"VOLTAR",white);
  rect(x+12,y+28,60,60,border);rect(x+13,y+29,58,58,0xbdd7);
  const CodexPortrait* art=nullptr;if(selectedEntry && portraits){auto found=portraits->find(rows[selectedIndex].first);if(found!=portraits->end())art=&found->second;}
  if(art && art->width>0 && art->height>0 && art->pixels.size()==size_t(art->width*art->height)){
   const int dw=std::max(1,56*art->width/std::max(art->width,art->height)),dh=std::max(1,56*art->height/std::max(art->width,art->height));
   for(int yy=0;yy<dh;++yy)for(int xx=0;xx<dw;++xx){const auto c=art->pixels[(yy*art->height/dh)*art->width+xx*art->width/dw];if(c&0x10000)pixel(x+14+(56-dw)/2+xx,y+30+(56-dh)/2+yy,uint16_t(c));}
  }else text(x+32,y+46,"?",0x4208,2);
  text(x+82,y+30,selectedEntry?overlayTextClip(rows[selectedIndex].second,43):"??????",0);
  text(x+82,y+44,"HP MAX: "+(info?info->hp:std::string("??????")),0);
  text(x+82,y+58,"MORTOS: "+std::to_string(selectedEntry?selectedEntry->kills:0),0);
  text(x+82,y+74,overlayTextClip(info?info->notes:std::string("???????"),43),border);
  CodexInfoSection fallback{"info","INFORMACOES",{{"","",{{selectedEntry?"DADOS ORIGINAIS INDISPONÍVEIS":"MONSTRO AINDA NÃO ENCONTRADO",0xffff}}}}};
  const size_t count=info && !info->sections.empty()?info->sections.size():1;detailPage=std::min(detailPage,count-1);
  const auto& section=info && !info->sections.empty()?info->sections[detailPage]:fallback;
  const auto pages=codexDetailPages(section);detailScroll=std::min(detailScroll,pages.size()-1);const auto& list=pages[detailScroll];
  text(x+12,y+92,overlayTextClip(std::to_string(detailPage+1)+" - "+section.title+(list.title.empty()?"":" / "+list.title),55),0);
  rect(x+12,y+101,panel.w-24,1,border);rect(x+12,y+103,panel.w-24,70,dark);
  for(size_t i=0;i<list.rows.size();++i)text(x+12,y+105+int(i)*10,overlayTextClip(list.rows[i].text,55),list.rows[i].color);
  if(pages.size()>1){
   const auto up=codexDetailUp(width,height),down=codexDetailDown(width,height);
   for(const auto b:{up,down}){rect(b.x,b.y,b.w,b.h,border);rect(b.x+1,b.y+1,b.w-2,b.h-2,white);}
   text(up.x+19,up.y+5,"SUBIR",detailScroll?0:muted);text(down.x+17,down.y+5,"DESCER",detailScroll+1<pages.size()?0:muted);
   text(x+145,y+179,(section.id=="drops"?"DROP ":"LISTA ")+std::to_string(detailScroll+1)+"/"+std::to_string(pages.size()),border);
  }
  text(x+12,y+198,"ANTERIOR",detailPage?0:muted);text(x+145,y+198,"PAG "+std::to_string(detailPage+1)+"/"+std::to_string(count),border);text(x+298,y+198,"PROXIMA",detailPage+1<count?0:muted);return;
 }
 text(x+180,y+10,"ACAO: INFO",gold);
 for(size_t i=start;i<end;++i) {
  auto cell=codexCell(i-start,width,height);rect(cell.x,cell.y,32,32,i==selectedIndex?gold:border);rect(cell.x+1,cell.y+1,30,30,0xbdd7);
  const CodexPortrait* art=nullptr;if(entry(rows[i].first) && portraits){auto found=portraits->find(rows[i].first);if(found!=portraits->end())art=&found->second;}
  if(art && art->width>0 && art->height>0 && art->pixels.size()==size_t(art->width*art->height)) {
   const int dw=std::max(1,30*art->width/std::max(art->width,art->height)),dh=std::max(1,30*art->height/std::max(art->width,art->height));
   for(int yy=0;yy<dh;++yy)for(int xx=0;xx<dw;++xx){const auto c=art->pixels[(yy*art->height/dh)*art->width+xx*art->width/dw];if(c&0x10000)pixel(cell.x+1+(30-dw)/2+xx,cell.y+1+(30-dh)/2+yy,uint16_t(c));}
  }else text(cell.x+11,cell.y+9,"?",0x4208,2);
 }
 auto detail=codexDetails(width,height);const CodexEntry* selectedEntry=rows.empty()?nullptr:entry(rows[selectedIndex].first);
 rect(detail.x,detail.y,detail.w,detail.h,0xffff);rect(x+156,detail.y,1,detail.h,border);rect(x+254,detail.y,1,detail.h,border);
 text(x+12,y+169,"MONSTRO",border);text(x+164,y+169,"ESTADO",border);text(x+270,y+169,"MORTOS",border);
 text(x+12,y+180,selectedEntry?overlayTextClip(rows[selectedIndex].second,23):"??????",0);text(x+164,y+180,selectedEntry?"ENCONTRADO":"???????",0);text(x+270,y+180,std::to_string(selectedEntry?selectedEntry->kills:0),0);
 text(x+12,y+198,"ANTERIOR",page?0:muted);text(x+145,y+198,"PAG "+std::to_string(page+1)+"/"+std::to_string(std::max<size_t>(1,(rows.size()+codexEntriesPerPage-1)/codexEntriesPerPage)),border);text(x+298,y+198,"PROXIMA",end<rows.size()?0:muted);
}
} // namespace destinia
