#include "../common/titles.h"
#include <cstdio>
using namespace destinia;
#define CHECK(e) do{if(!(e)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e);return 1;}}while(false)
int main(){
 Codex c;c.names[1]="A";c.names[2]="B";c.names[100]="Boss";c.entries[0][1].kills=50;c.entries[0][100].kills=1000;
 CHECK(!titleUnlockedByCodex(TitleId::Cacador,c,0));c.entries[0][2].kills=1;CHECK(titleUnlockedByCodex(TitleId::Cacador,c,0));CHECK(!titleUnlockedByCodex(TitleId::Cacador,c,1));
 CHECK(titleUnlockedByCodex(TitleId::Complecionista,c,0));c.entries[0].erase(100);CHECK(!titleUnlockedByCodex(TitleId::Complecionista,c,0));
 c.names[1]="Fungus";c.names[2]="Magic Fungus";c.entries[0][1].kills=9;c.entries[0][2].kills=100;
 CHECK(!titleUnlockedByCodex(TitleId::CacadorDeFungos,c,0));
 c.entries[0][1].kills=10;CHECK(titleUnlockedByCodex(TitleId::CacadorDeFungos,c,0));CHECK(!titleUnlockedByCodex(TitleId::CacadorDeFungos,c,1));
 CHECK(!titleUnlockedByCodex(TitleId::Veterano,c,0));
}
