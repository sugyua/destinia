#include "../common/codex_portraits.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <set>
using namespace destinia;
static std::vector<uint8_t> file(const std::filesystem::path& p) {std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv) {
 assert(argc==2);std::filesystem::path root=argv[1];
 assert(codexPortraitAssets(1).first=="mon1_0_head.wmb");assert(codexPortraitAssets(63).first=="mon21_2_head.wmb");
 assert(codexPortraitAssets(103)==codexPortraitAssets(101));assert(codexPortraitAssets(999).first.empty());
 // Regression confirmed with original drawAGDImageT (APK 1.0.6):
 // Imperial Soldier front idle body and weapon occupy 47x46.
 {auto n=codexPortraitAssets(61);auto p=codexDecodePortrait(codexUnpackResource(file(root/n.first)),codexUnpackResource(file(root/n.second)));assert(p.width==47 && p.height==46);}
 // Original-file references: complete poses, not sheet fragments.
 for(auto item:std::vector<std::array<int,3>>{{1,27,28},{4,54,43},{100,76,81},{102,75,69},{109,108,92}}) {
  auto names=codexPortraitAssets(item[0]);auto p=codexDecodePortrait(codexUnpackResource(file(root/names.first)),codexUnpackResource(file(root/names.second)),item[0]<100?1:0);
  assert(p.width==item[1] && p.height==item[2]);assert(p.pixels.size()==size_t(p.width*p.height));
  assert(std::any_of(p.pixels.begin(),p.pixels.end(),[](uint32_t c){return c&0x10000;}));
 }
 const std::map<int,uint32_t> golden={{1,0xcb852f42},{4,0x033b9dc1},{61,0xba844278},{100,0xac4761fc},{102,0x3301b4b9},{109,0x4e053378}};
 for(auto item:golden){auto n=codexPortraitAssets(item.first);auto p=codexDecodePortrait(codexUnpackResource(file(root/n.first)),codexUnpackResource(file(root/n.second)),item.first<100?1:0);uint32_t hash=2166136261u;for(auto c:p.pixels)for(int shift=0;shift<32;shift+=8)hash=(hash^uint8_t(c>>shift))*16777619u;assert(hash==item.second);}
 // Truncated/missing front actions must never silently display the back pose.
 {auto n=codexPortraitAssets(61);auto w=codexUnpackResource(file(root/n.first)),a=codexUnpackResource(file(root/n.second));
  for(size_t len:{size_t(12),size_t(651),size_t(653),size_t(661),size_t(662),size_t(741)}){auto cut=a;cut.resize(len);assert(codexDecodePortrait(w,cut).pixels.empty());}
  auto bad=a;bad[0]=1;assert(codexDecodePortrait(w,bad).pixels.empty());
  bad=a;bad[652]=0;assert(codexDecodePortrait(w,bad).pixels.empty());
  bad=a;bad[661]=255;assert(codexDecodePortrait(w,bad).pixels.empty());
  assert(codexDecodePortrait(w,a,-1).pixels.empty());assert(codexDecodePortrait(w,a,256).pixels.empty());
 }
 std::set<std::vector<uint32_t>> normal;
 for(int id=1;id<=109;id++){if(id>63 && id<100)continue;auto names=codexPortraitAssets(id);auto w=codexUnpackResource(file(root/names.first)),a=codexUnpackResource(file(root/names.second));auto p=codexDecodePortrait(w,a,id<100?1:0);assert(!p.pixels.empty());if(id<=63)normal.insert(p.pixels);
  for(size_t len:{size_t(0),size_t(4),size_t(11),w.size()-1}){auto shortW=w;shortW.resize(len);assert(codexDecodePortrait(shortW,a).pixels.empty());}
  a[11]=255;assert(codexDecodePortrait(w,a).pixels.empty());
 }
 assert(normal.size()==63);
 auto zip=file(root/"mon1_0_head.wmb");zip[8]=255;zip[9]=255;zip[10]=255;zip[11]=127;assert(codexUnpackResource(zip).empty());
 zip=file(root/"mon1_0_head.wmb");zip.back()^=1;assert(codexUnpackResource(zip).empty());
 std::puts("CODEX_PORTRAITS_73_OK");
}
