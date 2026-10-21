#pragma once
#include "codex.h"
#include "codex_portraits.h"
#include <array>
#include <cctype>
namespace destinia {
struct CodexRect { int x,y,w,h; bool contains(int px,int py) const { return px>=x && py>=y && px<x+w && py<y+h; } };
// The original menu is 232x240, centered in both hosts. Append a book tab
// beside its nine icons; never replace an existing native tab/hitbox.
inline CodexRect codexMenuButton(int width,int height) { return {width/2+120,height/2-120,76,24}; }
inline CodexRect codexPanel(int width,int height) { return {(width-360)/2,(height-240)/2+25,360,215}; }
inline void codexPaint(uint8_t* pixels,int width,int height,const Codex& book,bool open,size_t page,bool available,bool inMenu=false,bool selected=false,const CodexPortraits* portraits=nullptr) {
 if(!open && !inMenu) return;
 const int scale=1;auto pixel=[&](int x,int y,uint16_t c){if(x>=0&&x<width&&y>=0&&y<height){pixels[(y*width+x)*2]=c&255;pixels[(y*width+x)*2+1]=c>>8;}};
 auto rect=[&](int x,int y,int w,int h,uint16_t c){for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx)pixel(x+dx,y+dy,c);};
 auto glyph=[](char c)->std::array<uint8_t,5>{
  static const std::array<uint8_t,5> letters[]={
   {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67}};
  static const std::array<uint8_t,5> digits[]={{62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30}};
  c=char(std::toupper(static_cast<unsigned char>(c)));if(c>='A'&&c<='Z')return letters[c-'A'];if(c>='0'&&c<='9')return digits[c-'0'];
  if(c=='?')return {2,1,81,9,6};if(c=='.')return {0,96,96,0,0};if(c==':')return {0,54,54,0,0};if(c=='-')return {8,8,8,8,8};if(c=='/')return {32,16,8,4,2};return {0,0,0,0,0};
 };
 auto text=[&](int x,int y,const std::string& s,uint16_t color){for(char c:s){auto bits=glyph(c);for(int i=0;i<5;++i)for(int j=0;j<7;++j)if(bits[i]&(1<<j))rect(x+i*scale,y+j*scale,scale,scale,color);x+=6*scale;}};
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
 rect(x,y,panel.w,panel.h,border);rect(x+2,y+2,panel.w-4,panel.h-4,dark);
 rect(x+5,y+5,panel.w-10,17,0x0000);text(x+12,y+10,"CODEX - MONSTROS",gold);text(x+305,y+10,"VOLTAR",white);
 text(x+12,y+31,"MORTES REGISTRADAS DESDE A 0.7",muted);
 std::vector<std::pair<int,CodexEntry>> rows;auto character=book.entries.find(book.slot);if(character!=book.entries.end())for(auto entry:character->second)rows.push_back(entry);
 if(!available)text(x+12,y+72,"OBSERVADOR INDISPONIVEL",white);
 else if(rows.empty()){text(x+12,y+72,"NENHUM MONSTRO DESCOBERTO AINDA",white);text(x+12,y+90,"EXPLORE O MAPA PARA REGISTRAR MONSTROS",muted);}
 else {
  page=std::min(page,(rows.size()-1)/codexRowsPerPage);text(x+12,y+50,"MONSTRO",muted);text(x+170,y+50,"ESTADO",muted);text(x+250,y+50,"MORTOS",muted);
  for(size_t i=page*codexRowsPerPage;i<rows.size()&&i<page*codexRowsPerPage+codexRowsPerPage;++i) {
   const auto& row=rows[i];const int rowY=y+66+int(i-page*codexRowsPerPage)*32;
   rect(x+11,rowY,30,30,border);rect(x+12,rowY+1,28,28,0x10a3);
   const CodexPortrait* art=nullptr;if(portraits){auto found=portraits->find(row.first);if(found!=portraits->end()&&!found->second.pixels.empty())art=&found->second;}
   if(art && art->width>0 && art->height>0 && art->pixels.size()==size_t(art->width*art->height)) {
    const int dw=std::max(1,28*art->width/std::max(art->width,art->height)),dh=std::max(1,28*art->height/std::max(art->width,art->height));
    for(int yy=0;yy<dh;++yy)for(int xx=0;xx<dw;++xx){const auto c=art->pixels[(yy*art->height/dh)*art->width+xx*art->width/dw];if(c&0x10000)pixel(x+12+(28-dw)/2+xx,rowY+1+(28-dh)/2+yy,uint16_t(c));}
   }else text(x+23,rowY+12,"?",muted);
   text(x+47,rowY+12,book.names.at(row.first),white);text(x+170,rowY+12,row.second.kills?"DERROTADO":"VISTO",row.second.kills?gold:muted);text(x+250,rowY+12,std::to_string(row.second.kills),white);
  }
 }
 text(x+12,y+196,"ANTERIOR",gold);text(x+145,y+196,"PAG "+std::to_string(page+1)+"/"+std::to_string(std::max<size_t>(1,(rows.size()+codexRowsPerPage-1)/codexRowsPerPage)),muted);text(x+298,y+196,"PROXIMA",gold);
}
} // namespace destinia
