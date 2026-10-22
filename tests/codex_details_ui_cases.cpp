#include "../common/codex_ui.h"
#include <cassert>
int main(){
 using namespace destinia;
 CodexInfoView info;info.hp="170 / NV 2";info.notes="ULTIMO ENCONTRO";
 CodexInfoSection drops{"drops","DROPS",{{"gear","EQUIPAMENTOS",{}},{"eggs","OVOS",{{"Rare Monster Egg",0xffff}}}}};
 for(int i=0;i<8;i++)drops.groups[0].rows.push_back({"Sword "+std::to_string(i),0x07e0});
 info.sections={drops,{"spawns","LOCAIS DE SPAWN",{{"spawns","",{{"Dungeon Prison",0}}}}},{"variants","VARIANTES",{{"variants","",{{"Elite",0}}}}}};
 auto lists=codexDetailPages(drops);assert(lists.size()==3 && lists[0].rows.size()==7 && lists[1].rows.size()==1 && lists[2].title=="OVOS");
 for(auto size:std::vector<std::pair<int,int>>{{400,240},{800,480}}){
  Codex book;book.names[1]="Fungus";book.select(0);
  std::vector<uint8_t> hidden(size.first*size.second*2+16,0x35),changed=hidden;
  codexPaint(hidden.data(),size.first,size.second,book,true,0,true,true,true,nullptr,0,true,&info,0,0);
  CodexInfoView secret=info;secret.hp="SECRET";secret.sections[0].groups[0].rows[0].text="SECRET ITEM";
  codexPaint(changed.data(),size.first,size.second,book,true,0,true,true,true,nullptr,0,true,&secret,0,0);assert(hidden==changed);
  book.observe(1,1,true);
  codexPaint(hidden.data(),size.first,size.second,book,true,0,true,true,true,nullptr,0,true,&info,0,0);
  codexPaint(changed.data(),size.first,size.second,book,true,0,true,true,true,nullptr,0,true,&info,0,2);assert(hidden!=changed);
  auto panel=codexPanel(size.first,size.second);bool green=false;for(int y=panel.y+105;y<panel.y+174;y++)for(int x=panel.x+12;x<panel.x+348;x++){auto at=(y*size.first+x)*2;green|=(hidden[at]|hidden[at+1]<<8)==0x07e0;}assert(green);
  for(size_t i=hidden.size()-16;i<hidden.size();i++)assert(hidden[i]==0x35);
  assert(!codexDetailUp(size.first,size.second).contains(codexNext(size.first,size.second).x,codexNext(size.first,size.second).y));
 }
}
