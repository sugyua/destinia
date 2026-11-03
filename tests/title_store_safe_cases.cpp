#include "../common/titles.h"
#include <cassert>
using namespace destinia;
int main(){
 TitlesStore s;s.unlock(0,TitleId::SenhorDoAbismo);s.setEquipped(0,TitleId::SenhorDoAbismo);const auto original=s.encode();
 for(const std::string& bad:{"", "DESTINIA_TITLES_V1\nequipped 0 7\n", "DESTINIA_TITLES_V1\nequipped 22 1\nend\n", "DESTINIA_TITLES_V1\nequipped 0 99\nend\n", "DESTINIA_TITLES_V1\nunlocked 0 7\nunlocked 0 7\nend\n", "DESTINIA_TITLES_V1\nequipped 0 7\nequipped 0 8\nend\n", "DESTINIA_TITLES_V1\nunknown 0 7\nend\n", "DESTINIA_TITLES_V1\nend\nextra\n"}){assert(!s.decode(bad));assert(s.encode()==original);}
 TitlesStore copy;assert(copy.decode(original));assert(copy.encode()==original);
 assert(copy.decode("DESTINIA_TITLES_V1\r\nequipped 1 1\r\nend\r\n"));assert(copy.getEquipped(1)==TitleId::Iniciante);
 assert(!copy.isExclusive(1));copy.setExclusive(1,true);auto modern=copy.encode();
 TitlesStore again;assert(again.decode(modern));assert(again.isExclusive(1));assert(again.getEquipped(1)==TitleId::Iniciante);
 auto snapshot=again.encode();assert(!again.decode("DESTINIA_TITLES_V1\nexclusive 0 1\nexclusive 0 1\nend\n"));assert(again.encode()==snapshot);
}
