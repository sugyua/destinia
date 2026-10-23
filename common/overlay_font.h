#pragma once
#include <array>
#include <cstdint>
#include <string>
namespace destinia {
// Original overlay 5x7 bitmap, with explicit Portuguese accents above/below it.
// UTF-8 stays in host UI; native ARM text tables use their separate encoding.
inline char32_t overlayCodepoint(const std::string& text,size_t& at) {
 const auto first=static_cast<unsigned char>(text[at++]);
 if(first<128)return first;
 if(first>=0xc2 && first<=0xdf && at<text.size()) {
  const auto next=static_cast<unsigned char>(text[at]);
  if((next&0xc0)==0x80){++at;return ((first&31)<<6)|(next&63);}
 }
 // Unsupported sequences draw one replacement, never read beyond the string.
 while(at<text.size() && (static_cast<unsigned char>(text[at])&0xc0)==0x80)++at;
 return U'?';
}
inline std::array<uint16_t,5> overlayGlyph(char32_t code) {
 char base='?'; int mark=0;
 switch(code) {
  case U'á':case U'Á':base='A';mark=1;break;
  case U'à':case U'À':base='A';mark=2;break;
  case U'â':case U'Â':base='A';mark=3;break;
  case U'ã':case U'Ã':base='A';mark=4;break;
  case U'é':case U'É':base='E';mark=1;break;
  case U'ê':case U'Ê':base='E';mark=3;break;
  case U'í':case U'Í':base='I';mark=1;break;
  case U'ó':case U'Ó':base='O';mark=1;break;
  case U'ô':case U'Ô':base='O';mark=3;break;
  case U'õ':case U'Õ':base='O';mark=4;break;
  case U'ú':case U'Ú':base='U';mark=1;break;
  case U'ü':case U'Ü':base='U';mark=5;break;
  case U'ç':case U'Ç':base='C';mark=6;break;
  default:if(code<128)base=char(code);break;
 }
 auto ascii=[](char c)->std::array<uint8_t,5>{
  static const std::array<uint8_t,5> letters[]={
   {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67}};
  static const std::array<uint8_t,5> digits[]={{62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30}};

  if(c>='a' && c<='z')c=char(c-'a'+'A');
  if(c>='A'&&c<='Z')return letters[c-'A'];if(c>='0'&&c<='9')return digits[c-'0'];
  switch(c){
   case '?':return {2,1,81,9,6};case '.':return {0,96,96,0,0};case ':':return {0,54,54,0,0};
   case '-':return {8,8,8,8,8};case '/':return {32,16,8,4,2};case '_':return {64,64,64,64,64};
   case '<':return {0,8,20,34,0};case '>':return {0,34,20,8,0};case '^':return {4,2,1,2,4};
   case '+':return {8,8,62,8,8};case '(':return {0,28,34,65,0};case ')':return {0,65,34,28,0};
   case ' ':return {0,0,0,0,0};default:return {2,1,81,9,6};
  }
 };
 std::array<uint16_t,5> result{};const auto body=ascii(base);
 for(size_t i=0;i<5;++i)result[i]=uint16_t(body[i])<<2;
 if(mark==1){result[2]|=2;result[3]|=1;}
 if(mark==2){result[1]|=1;result[2]|=2;}
 if(mark==3){result[1]|=2;result[2]|=1;result[3]|=2;}
 if(mark==4){result[0]|=2;result[1]|=1;result[2]|=1;result[3]|=2;result[4]|=1;}
 if(mark==5){result[1]|=1;result[3]|=1;}
 if(mark==6){result[2]|=1<<9;result[1]|=1<<10;result[2]|=1<<10;}
 return result;
}
inline std::string overlayTextClip(const std::string& text,size_t cells) {
 size_t at=0;while(at<text.size() && cells){overlayCodepoint(text,at);--cells;}return text.substr(0,at);
}
inline int overlayTextWidth(const std::string& text,int scale=1) {
 int width=0;for(size_t at=0;at<text.size();){overlayCodepoint(text,at);width+=6*scale;}return width;
}
inline void paintOverlayText(uint8_t* pixels,int width,int height,int x,int y,const std::string& text,uint16_t color,int scale=1) {
 if(scale<1)return;
 for(size_t at=0;at<text.size();x+=6*scale) {
  const auto glyph=overlayGlyph(overlayCodepoint(text,at));
  for(int i=0;i<5;++i)for(int row=0;row<11;++row)if(glyph[i]&(1<<row))
   for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx){
    const int px=x+i*scale+dx,py=y+(row-2)*scale+dy;
    if(px>=0 && px<width && py>=0 && py<height){const auto p=(py*width+px)*2;pixels[p]=color&255;pixels[p+1]=color>>8;}
   }
 }
}
} // namespace destinia
