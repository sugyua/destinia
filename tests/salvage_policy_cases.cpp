#include "../common/inventory_view.h"
#include "../common/salvage_policy.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace destinia;
 std::vector<InventoryItem> items={
  {0,0,10,0,1,1,100,false},
  {0,1,11,0,1,1,90,true},
  {0,2,12,0,1,1,80,false},
  {1,3,13,0,1,1,70,false,false,true},
  {1,4,14,0,1,1,60,false}
 };
 InventoryQuery query;auto view=inventoryView(items,query);
 const auto targets=inventorySalvageCandidates(items,view,{12},50);
 assert((targets==std::vector<size_t>{4,0}));
 assert(inventorySalvageCandidates(items,view,{12},2).size()==2);
 bool rejected=false;try{inventorySalvageCandidates(items,{0,0},{},50);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 std::cout<<"SALVAGE_POLICY_OK\n";
}
