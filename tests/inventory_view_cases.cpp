#include <cassert>
#include <vector>
#include "../common/inventory_view.h"
int main(){using namespace destinia;
 std::vector<InventoryItem> items={{0,0,7,2,1,8,300},{0,1,2,0,0,4,100},{1,0,9,2,1,8,100}};
 const auto original=items;InventoryQuery q;q.sort=InventorySort::Price;
 auto view=inventoryView(items,q);assert(view.size()==3&&view[0]==1&&view[1]==2&&view[2]==0);assert(items==original);
 q.type=2;q.rank=1;view=inventoryView(items,q);assert((view==std::vector<size_t>{2,0}));
 assert(inventorySelection(items,view,7)==1);assert(!inventorySelection(items,view,2));
 q.minLevel=9;assert(inventoryView(items,q).empty());assert(!inventorySelection(items,{},7));
 q={};q.sort=InventorySort::Level;q.descending=true;view=inventoryView(items,q);assert((view==std::vector<size_t>{0,2,1}));
 // Reordering the input must not silently select another equipment pool.
 std::swap(items[0],items[2]);view=inventoryView(items,q);auto selected=inventorySelection(items,view,7);assert(selected&&items[view[*selected]].pool==7);
 auto duplicate=items;duplicate.push_back(items[0]);bool rejected=false;try{inventoryView(duplicate,q);}catch(...){rejected=true;}assert(rejected);
 q={};q.sort=InventorySort::Type;view=inventoryView(items,q);assert(items[view[0]].type==0);
 q.sort=InventorySort::Rank;view=inventoryView(items,q);assert(items[view[0]].rank==0);
}
