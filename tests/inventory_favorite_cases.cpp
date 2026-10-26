#include "../common/inventory_panel.h"
#include "../common/inventory_protection_store.h"
#include <cassert>
int main(){
 using namespace destinia;InventoryProtection protection;std::array<uint8_t,20> a{};a.fill(3);auto b=a;
 assert(protection.toggle(7,a));a[0]=9;assert(protection.has(7,b));assert(!protection.has(8,b));
 assert(!protection.toggle(7,b));assert(!protection.has(7,b));assert(protection.toggle(7,b));
 assert(!protection.has(7,a));assert(!protection.has(7,b));protection.toggle(7,b);protection.clear();assert(!protection.has(7,b));
 protection.select(2);assert(protection.toggle(7,b));const auto encoded=protection.encode();InventoryProtection reopened;reopened.decode(encoded);reopened.select(2);assert(reopened.has(7,b));reopened.select(1);assert(!reopened.has(7,b));reopened.select(2);
 for(const auto& bad:std::vector<std::string>{encoded.substr(0,encoded.size()-1),encoded+"2 7 "+std::string(40,'0')+"\n","UNKNOWN\n"}){bool threw=false;try{reopened.decode(bad);}catch(...){threw=true;}assert(threw&&reopened.has(7,b));}
 const auto root=fs::temp_directory_path()/("destinia-protection-"+uniqueId());fs::create_directory(root);
 const auto original=root/"save0.dat";{std::ofstream f(original,std::ios::binary);f<<"ORIGINAL-SAVE-SENTINEL";}
 saveInventoryProtection(root,reopened);InventoryProtection loaded;loadInventoryProtection(root,loaded);loaded.select(2);assert(loaded.has(7,b));assert(loaded.encode()==encoded);
 struct FailReplace:AtomicSaveOps{void replace(const fs::path&,const fs::path&)const override{throw std::runtime_error("disco indisponivel");}} failure;
 auto next=loaded;next.toggle(7,b);bool failed=false;try{saveInventoryProtection(root,next,failure);}catch(...){failed=true;}assert(failed);InventoryProtection retained;loadInventoryProtection(root,retained);assert(retained.encode()==encoded);
 {std::ofstream f(inventoryProtectionFile(root));f<<"CORRUPTED";}
 failed=false;try{loadInventoryProtection(root,loaded);}catch(...){failed=true;}assert(failed&&loaded.encode()==encoded);
 failed=false;try{saveInventoryProtection(root,next);}catch(...){failed=true;}assert(failed);
 {std::ifstream f(inventoryProtectionFile(root));std::string value{std::istreambuf_iterator<char>(f),{}};assert(value=="CORRUPTED");}
 {std::ifstream f(original);std::string value{std::istreambuf_iterator<char>(f),{}};assert(value=="ORIGINAL-SAVE-SENTINEL");}
 for(const auto& entry:fs::directory_iterator(root))assert(entry.path().extension()!=".tmp");
 fs::remove_all(root);
 InventoryPanel p;p.items={{0,0,7,0,0,1,10,true}};p.selectedPool=7;p.focus=7;p.sale=true;p.rebuild();
 assert(!p.canChoose());assert(p.key(6,20)==InventoryEvent::None);assert(p.hit(12,20)==InventoryEvent::Protect);
 assert(p.key(12,20)==InventoryEvent::Protect);p.sale=false;assert(p.canChoose());p.page=1;assert(p.hit(12,20)==InventoryEvent::None);
 for(int width:{400,800})for(int scale:{75,100,125}){int height=width==400?240:480;auto t=inventoryTransform(width,height,scale,true);auto r=inventoryPanelRect(width,height);auto point=t.point(r.x+280,r.y+197);assert(inventoryHit(point.first,point.second,width,height,scale,true)==12);}

}
