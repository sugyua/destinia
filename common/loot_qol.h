#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <vector>
namespace destinia {
struct LootOptions {bool visual=false,autoGold=true,autoMaterials=false;int categories=31,minRank=0,radius=19;};
struct LootDrop {int type=0,id=0,category=16,rank=0;};
inline void validateLootOptions(const LootOptions& o){if(o.categories<0||o.categories>31||o.minRank<0||o.minRank>4||o.radius<1||o.radius>19)throw std::runtime_error("Preferencias de loot invalidas");}
inline std::map<int,int> lootMaterialTypes(const std::vector<uint8_t>& bytes){
 if(bytes.size()!=158*166)throw std::runtime_error("Tabela de materiais incompativel");std::map<int,int> types;
 for(int i=0;i<158;i++){size_t p=size_t(i)*166;int id=bytes[p]|bytes[p+1]<<8,type=bytes[p+4]|bytes[p+5]<<8;if(id!=i||type>255)throw std::runtime_error("Registro de material invalido");types[id]=type;}return types;
}
inline LootDrop describeDrop(int type,int id,const std::map<int,int>& types){
 LootDrop d{type,id,16,0};if(type==0){d.category=1;d.rank=(id>>8)&7;if(d.rank>4)d.category=16;}
 else if(type==5)d.category=4;else if(type==2)d.category=8;
 else if(type==1){auto it=types.find(id);if(it!=types.end()&&(it->second==14||it->second==15||it->second==16||it->second==17||it->second==22))d.category=2;}
 return d;
}
inline bool visibleDrop(const LootDrop& d,const LootOptions& o){return !o.visual||((o.categories&d.category)!=0&&(d.category!=1||d.rank>=o.minRank));}
inline bool autoEligible(const LootDrop& d,const LootOptions& o,int dx,int dy){return std::abs(dx)<=o.radius&&std::abs(dy)<=o.radius&&((o.autoGold&&d.category==4&&d.id>0)||(o.autoMaterials&&d.category==2));}
inline uint16_t lootColor(const LootDrop& d){constexpr uint16_t ranks[]={0xe71c,0x07e0,0x5dff,0xd01f,0xffe0};if(d.category==1)return ranks[std::clamp(d.rank,0,4)];return d.category==4?0xffe0:d.category==2?0x07ff:d.category==8?0xfd20:0xe71c;}
inline void paintLootMarker(uint8_t* pixels,int w,int h,int x,int y,uint16_t color){
 auto put=[&](int px,int py,uint16_t c){if(px>=0&&py>=0&&px<w&&py<h){size_t at=size_t(py*w+px)*2;pixels[at]=uint8_t(c);pixels[at+1]=uint8_t(c>>8);}};
 for(int dx=-7;dx<=7;dx++){int dy=7-std::abs(dx);for(int sign:{-1,1}){put(x+dx,y+sign*dy,0);if(std::abs(dx)<6)put(x+dx,y+sign*(dy-1),color);}}put(x,y,color);
}
}
