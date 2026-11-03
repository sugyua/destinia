#include "../common/boss_rush_ui.h"
#include <cstdio>
#include <fstream>
using namespace destinia;
#define CHECK(e) do{if(!(e)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e);return 1;}}while(false)
int main(){
 auto dir=fs::temp_directory_path()/("destinia-title-ui-"+uniqueId());fs::create_directories(dir);auto path=dir/"titles-v1.txt";{std::ofstream f(path);f<<"DESTINIA_TITLES_V1\nequipped 0 7\n";}
 BossRushUI ui;ui.saveDir=dir;CHECK(!ui.titlesStore.load(dir));ui.open=true;ui.currentTab=4;ui.currentSlot=0;ui.selectedIndex=1;int calls=0;ui.calcHeroStat=[&](){++calls;return true;};
 CHECK(ui.handleKey(17,true));CHECK(ui.titlesStore.getEquipped(0)==TitleId::Nenhum);CHECK(calls==0);CHECK(ui.statusMessage.find("ERRO")!=std::string::npos);
 {std::ifstream f(path);std::string s((std::istreambuf_iterator<char>(f)),{});CHECK(s=="DESTINIA_TITLES_V1\nequipped 0 7\n");}
 fs::remove(path);fs::remove(dir);return 0;
}
