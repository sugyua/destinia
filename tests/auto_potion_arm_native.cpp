#include <unicorn/unicorn.h>
#include <cstdio>
static uc_err keepPotionEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepPotionEngine
#define main potionOriginalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
int main(int argc,char** argv){
 if(int result=potionOriginalMain(argc,argv))return result;
 const u32 g=exportsMap.at("Game");
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 1;
 std::vector<uint8_t> table;if(!readFile(assetsDir+"/item_en.gdt",table))return 2;
 for(int type=0;type<2;++type){
  const int id=type?5:0;
  wr16(g+0x4a8c,0);wr16(g+0x4a90,0x6123);wr16(g+0x433c,1);wr16(g+0x433e,1);wr16(g+0x4340,id);wr16(g+0x4342,3);
  for(unsigned b=1;b<4;++b)wr16(g+0x433c+b*0x92,0);
  wr16(g+0x4854,0);wr16(g+0x48f2,type?10:189);wr16(g+0x48f4,type?1000:500);wr16(g+0x48f6,189);wr16(g+0x48f8,1000);wr32(g+0x7cd4,1000);
  const auto hp=rd16(g+0x48f4),mp=rd16(g+0x48f2);
  destinia::AutoPotionController controller;
  auto read=[](u32 a){return uint16_t(rd16(a));};auto write=[](u32 a,uint16_t v){wr16(a,v);};
  auto use=[&](int wanted){return destinia::useNativePotion(g,wanted,read,write,[&](){return callFn(exportsMap.at("_pressedQuickSlot"),{0});},[&](uint16_t item,int t){return destinia::validPotionData(table,item,t);});};
  if(!controller.update(g,read,write,use,{}) || rd16(g+0x4342)!=2 || rd16(g+0x4a90)!=0x6123 || rd16(g+0x48f4)!=hp || rd16(g+0x48f2)!=mp || rd16(g+(type?0x7cea:0x7ce8))!=(type?5:50))return 3+type;
  if(controller.update(g,read,write,use,{}) || rd16(g+0x4342)!=2)return 5;
 }
 std::puts("AUTO_POTION_ARM_OK");return 0;
}
