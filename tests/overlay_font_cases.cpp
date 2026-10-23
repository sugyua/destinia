#include "../common/overlay_font.h"
#include <cassert>
#include <array>
int main() {
 using namespace destinia;
 assert(overlayTextWidth("AÇÃO")==24);
 assert(overlayTextClip("AÇÃO",3)=="AÇÃ");
 assert(overlayTextClip("AÇÃO",0).empty());
 assert(overlayTextWidth("HABILIDADE: Ç Á À Â Ã É Ê Í Ó Ô Õ Ú Ü")==222);
 std::array<uint8_t,64*16*2> plain{},accent{};
 paintOverlayText(plain.data(),64,16,0,0,"ACAO",0xffff);
 paintOverlayText(accent.data(),64,16,0,0,"AÇÃO",0xffff);
 assert(plain!=accent);
 // A composed UTF-8 glyph occupies one cell; no phantom extra glyph per byte.
 auto bits=overlayGlyph(U'Ã');assert(bits!=overlayGlyph(U'A'));
 bool hasAccent=false;for(auto column:bits)hasAccent|=(column&3)!=0;assert(hasAccent);
 for(const auto letter:U"ÁÀÂÃÉÊÍÓÔÕÚÜÇáàâãéêíóôõúüç")if(letter)assert(overlayGlyph(letter)!=overlayGlyph(U'?'));
 // Clipping is required when the independent UI scale reaches an edge.
 std::array<uint8_t,4*4*2+2> clipped{};clipped.front()=17;clipped.back()=29;
 paintOverlayText(clipped.data()+1,4,4,-3,-2,"AÇÃO",0xffff,2);
 assert(clipped.front()==17 && clipped.back()==29);
}
