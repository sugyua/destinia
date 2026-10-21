#pragma once
#include "codex.h"
#include <array>
#include <cctype>
namespace destinia {
inline void codexPaint(uint8_t* pixels,int width,int height,const Codex& book,bool open,size_t page,bool available) {
 const int scale=std::max(1,width/400);auto pixel=[&](int x,int y,uint16_t c){if(x>=0&&x<width&&y>=0&&y<height){pixels[(y*width+x)*2]=c&255;pixels[(y*width+x)*2+1]=c>>8;}};
 auto rect=[&](int x,int y,int w,int h,uint16_t c){for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx)pixel(x+dx,y+dy,c);};
 auto glyph=[](char c)->std::array<uint8_t,5>{
  static const std::array<uint8_t,5> letters[]={
   {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67}};
  static const std::array<uint8_t,5> digits[]={{62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30}};
  c=char(std::toupper(static_cast<unsigned char>(c)));if(c>='A'&&c<='Z')return letters[c-'A'];if(c>='0'&&c<='9')return digits[c-'0'];
  if(c==':')return {0,54,54,0,0};if(c=='-')return {8,8,8,8,8};if(c=='/')return {32,16,8,4,2};return {0,0,0,0,0};
 };
 auto text=[&](int x,int y,const std::string& s,uint16_t color){for(char c:s){auto bits=glyph(c);for(int i=0;i<5;++i)for(int j=0;j<7;++j)if(bits[i]&(1<<j))rect(x+i*scale,y+j*scale,scale,scale,color);x+=6*scale;}};
 const uint16_t dark=0x10a3,gold=0xe5b2,white=0xe71c,muted=0x8c92;
 if(!open){rect(width-85*scale,4*scale,81*scale,15*scale,dark);text(width-80*scale,8*scale,"CODEX F8",gold);return;}
 rect(0,0,width,height,dark);const int x=(width-380*scale)/2,y=(height-220*scale)/2;
 text(x+8*scale,y+5*scale,"CODEX - MONSTROS",gold);text(x+298*scale,y+5*scale,"F8 FECHAR",white);
 text(x+8*scale,y+23*scale,"MORTES REGISTRADAS A PARTIR DA 0.7",muted);
 std::vector<std::pair<int,CodexEntry>> rows;auto character=book.entries.find(book.slot);if(character!=book.entries.end())for(auto entry:character->second)rows.push_back(entry);
 if(!available)text(x+8*scale,y+60*scale,"OBSERVADOR INDISPONIVEL",white);
 else if(rows.empty()){text(x+8*scale,y+60*scale,"NENHUM MONSTRO DESCOBERTO AINDA",white);text(x+8*scale,y+78*scale,"ENTRE NA PARTIDA E EXPLORE O MAPA",muted);}
 else {
  page=std::min(page,(rows.size()-1)/8);text(x+8*scale,y+43*scale,"MONSTRO",muted);text(x+210*scale,y+43*scale,"ESTADO",muted);text(x+310*scale,y+43*scale,"MORTOS",muted);
  for(size_t i=page*8;i<rows.size()&&i<page*8+8;++i){const auto& row=rows[i];int rowY=y+(61+int(i-page*8)*16)*scale;text(x+8*scale,rowY,book.names.at(row.first),white);text(x+210*scale,rowY,row.second.kills?"DERROTADO":"VISTO",row.second.kills?gold:muted);text(x+310*scale,rowY,std::to_string(row.second.kills),white);}
 }
 text(x+8*scale,y+201*scale,"ANTERIOR",gold);text(x+148*scale,y+201*scale,"PAG "+std::to_string(page+1)+"/"+std::to_string(std::max<size_t>(1,(rows.size()+7)/8)),muted);text(x+310*scale,y+201*scale,"PROXIMA",gold);
}
} // namespace destinia
