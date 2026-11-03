#include "../common/title_menu_model.h"
#include <cassert>
using namespace destinia;
int main(){
 TitlesStore store; TitleMenuModel menu;
 assert(!menu.enter(16,1,49,0,7));
 assert(!menu.enter(16,2,47,0,7));
 assert(!menu.enter(16,2,49,10,7));
 assert(!menu.enter(16,2,49,0,5));
 assert(menu.enter(16,2,49,0,7));assert(menu.open&&menu.slot==0&&menu.selected==0);
 assert(menu.count()==9);assert(menu.selectedTitle()==TitleId::Iniciante);
 assert(menu.key(17,store)==TitleMenuAction::Equip);
 assert(menu.key(9,store)==TitleMenuAction::Consumed);assert(menu.selectedTitle()==TitleId::Cacador);
 assert(menu.key(17,store)==TitleMenuAction::Locked);assert(menu.open);
 store.unlock(0,TitleId::Cacador);assert(menu.key(17,store)==TitleMenuAction::Equip);
 for(int i=0;i<15;i++)menu.key(9,store);assert(menu.selected==8);
 assert(menu.key(7,store)==TitleMenuAction::Consumed&&menu.selected==8);
 assert(menu.key(5,store)==TitleMenuAction::Consumed && menu.open && menu.selected==0);
 assert(menu.key(5,store)==TitleMenuAction::Back&& !menu.open);
 assert(menu.enter(16,2,48,1,7));assert(menu.slot==1);
 assert(menu.key(27,store)==TitleMenuAction::Cancel&&!menu.open);
 assert(menu.enter(16,2,48,1,7));menu.validate(16,2,0);assert(!menu.open);
 assert(menu.enter(16,2,48,1,7));menu.validate(0,2,1);assert(!menu.open);
}
