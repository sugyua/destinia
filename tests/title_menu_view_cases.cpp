#include "../common/title_menu_view.h"
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
using namespace destinia;
int main(){
 TitlesStore titles;TitleMenuModel menu;assert(menu.enter(16,2,49,0,7));
 for(auto spec: {std::array<int,3>{400,240,100},std::array<int,3>{800,480,125}}){
  int w=spec[0],h=spec[1],scale=spec[2],panels=0,cursors=0;std::vector<std::string> labels;std::vector<uint8_t> frame(size_t(w)*h*2);
  originalPanelPainter=[&](uint8_t*,int,int,int,int,int,int){++panels;return true;};
  originalTextPainter=[&](uint8_t*,int,int,int,int,const std::string&s,uint16_t,int,int){labels.push_back(s);return true;};
  originalCursorPainter=[&](uint8_t*,int,int,int,int){++cursors;return true;};
  assert(paintTitleExtra(frame.data(),w,h,scale,menu,titles));assert(panels>=2&&cursors==1);
  assert(std::find(labels.begin(),labels.end(),"8 / 9")!=labels.end());assert(std::find(labels.begin(),labels.end(),"Iniciante")!=labels.end());
  auto t=nativeCodexTransform(w,h,scale);auto row=titleExtraRow(1,w,h);auto point=t.point(row.x+3,row.y+3);auto hit=titleExtraHit(point.first,point.second,w,h,scale);assert(hit.kind==TitleHitKind::Row&&hit.row==1);
  menu.selected=8;labels.clear();assert(paintTitleExtra(frame.data(),w,h,scale,menu,titles));
  assert(std::find(labels.begin(),labels.end(),"9 / 9")!=labels.end());
  assert(std::find(labels.begin(),labels.end(),"Cacador de Fungos")!=labels.end());
  assert(std::find(labels.begin(),labels.end(),"Derrote 10 Fungos. +10% critico.")!=labels.end());
  menu.selected=0;
  auto prev=titleExtraPrevious(w,h);point=t.point(prev.x+2,prev.y+2);hit=titleExtraHit(point.first,point.second,w,h,scale);assert(hit.kind==TitleHitKind::Previous);
 }
}
