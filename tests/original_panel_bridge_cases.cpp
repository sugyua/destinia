#include <map>
#include <stdexcept>
#include <iostream>
#include "../common/original_ui.h"
#include "../common/weapons_catalog_ui.h"
#include "../common/shared_stash_ui.h"
#include "../common/bounties_ui.h"
#include "../common/boss_rush_ui.h"
using namespace destinia;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(){try{
 for(int width:{400,800})for(int percent:{75,100,125}){
  int height=width*3/5;std::vector<uint8_t> pixels(width*height*2);
  WeaponsCatalogUI catalogUI;WeaponsCatalog catalog;SharedStashUI stashUI;SharedStash stash;BountiesUI bountyUI;BountyManager bounty;
  std::vector<UiRect> panels;std::vector<std::string> texts;
  originalPanelPainter=[&](uint8_t*,int,int,int x,int y,int w,int h){panels.push_back({x,y,w,h});return true;};
  originalTextPainter=[&](uint8_t*,int,int,int,int,const std::string& s,uint16_t,int,int){texts.push_back(s);return true;};
  auto verify=[&](auto& ui,auto paint){panels.clear();texts.clear();std::fill(pixels.begin(),pixels.end(),0);paint();auto p=ui.panelRect(width,height);auto r=UiTransform{width,height,percent,UiAnchor::Center,p.w,p.h,p.x,p.y}.rect(p.x,p.y,p.w,p.h);check(!panels.empty(),"panel did not invoke provider");check(panels[0].x==r.x&&panels[0].y==r.y&&panels[0].w==r.w&&panels[0].h==r.h,"panel transform differs");check(panels.size()>3,"buttons/cards bypass provider");check(texts.size()>3,"visible texts bypass provider");check(pixels[(r.y*width+r.x)*2]==0,"fallback overwrote original border");};
  verify(catalogUI,[&]{catalogUI.paint(pixels.data(),width,height,0,catalog,percent);});
  verify(stashUI,[&]{stashUI.paint(pixels.data(),width,height,stash,percent);});
  verify(bountyUI,[&]{bountyUI.paint(pixels.data(),width,height,0,bounty,percent);});
  BossRushUI boss;boss.open=true;boss.survivalRows=[] {return std::vector<std::string>{"Arena original"};};for(int tab=0;tab<6;++tab){boss.currentTab=tab;panels.clear();texts.clear();boss.paint(pixels.data(),width,height);check(panels.size()>=7,"boss frames bypass provider");check(texts.size()>10,"boss text bypass provider");}
  boss.open=false;panels.clear();texts.clear();boss.paint(pixels.data(),width,height);check(panels.empty()&&texts.empty(),"closed boss rendered");
  originalPanelPainter={};originalTextPainter={};std::fill(pixels.begin(),pixels.end(),0);catalogUI.paint(pixels.data(),width,height,0,catalog,percent);check(std::any_of(pixels.begin(),pixels.end(),[](uint8_t v){return v!=0;}),"missing provider lost fallback");auto absent=pixels;std::fill(pixels.begin(),pixels.end(),0);originalPanelPainter=[](uint8_t*,int,int,int,int,int,int){return false;};originalTextPainter=[](uint8_t*,int,int,int,int,const std::string&,uint16_t,int,int){return false;};catalogUI.paint(pixels.data(),width,height,0,catalog,percent);check(pixels==absent,"declined provider changed fallback");
 }
 std::cout<<"ORIGINAL_PANEL_BRIDGE_OK\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
