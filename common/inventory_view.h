#pragma once
#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
namespace destinia {
// Session-only references to original equipment records. Never persist a bag
// position as identity. Revalidate pool ownership/fingerprint before actions.
struct InventoryItem {
 int bag,slot,pool,type,rank,level,price;
 bool operator==(const InventoryItem& o)const{return bag==o.bag&&slot==o.slot&&pool==o.pool&&type==o.type&&rank==o.rank&&level==o.level&&price==o.price;}
};
enum class InventorySort {Original,Type,Rank,Level,Price};
struct InventoryQuery {int type=-1,rank=-1,minLevel=0,maxLevel=65535;InventorySort sort=InventorySort::Original;bool descending=false;int minPrice=0,maxPrice=2147483647;};
inline std::vector<size_t> inventoryView(const std::vector<InventoryItem>& items,const InventoryQuery& query){
 std::vector<size_t> result;std::set<int> pools;std::set<std::pair<int,int>> slots;
 for(size_t i=0;i<items.size();i++){
  const auto& item=items[i];
  if(item.pool<0||item.pool>=200||item.bag<0||item.bag>=4||item.slot<0||item.type<0||item.type>7||item.rank<0||item.rank>4||item.level<0||item.price<0||!pools.insert(item.pool).second||!slots.emplace(item.bag,item.slot).second)throw std::runtime_error("Inventario inconsistente; nao projetar alvos ambiguos.");
  if((query.type<0||query.type==item.type)&&(query.rank<0||query.rank==item.rank)&&item.level>=query.minLevel&&item.level<=query.maxLevel&&item.price>=query.minPrice&&item.price<=query.maxPrice)result.push_back(i);
 }
 auto value=[&](const InventoryItem& i){switch(query.sort){case InventorySort::Type:return i.type;case InventorySort::Rank:return i.rank;case InventorySort::Level:return i.level;case InventorySort::Price:return i.price;default:return 0;}};
 std::sort(result.begin(),result.end(),[&](size_t a,size_t b){const auto& left=items[a];const auto& right=items[b];const int x=value(left),y=value(right);if(x!=y)return query.descending?x>y:x<y;return std::pair<int,int>{left.bag,left.slot}<std::pair<int,int>{right.bag,right.slot};});
 return result;
}
// No silent fallback if the selected item was filtered out or disappeared.
inline std::optional<size_t> inventorySelection(const std::vector<InventoryItem>& items,const std::vector<size_t>& view,int pool){
 for(size_t at=0;at<view.size();at++)if(view[at]<items.size()&&items[view[at]].pool==pool)return at;
 return std::nullopt;
}
}
