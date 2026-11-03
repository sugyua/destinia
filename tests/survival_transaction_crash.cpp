#include "../common/safe_save.h"
#include <cstdlib>
#include <fstream>
#include <string>
struct Crash : destinia::AtomicSaveOps {
 mutable int count=0; int stop;
 explicit Crash(int n):stop(n){}
 void replace(const destinia::fs::path& a,const destinia::fs::path& b) const override {
  destinia::AtomicSaveOps::replace(a,b);if(++count==stop)std::_Exit(71);
 }
 void afterCommitMarker(const destinia::fs::path&)const override {if(stop==6)std::_Exit(72);}
};
int main(int argc,char**argv){
 if(argc!=3)return 1; namespace fs=destinia::fs;fs::path root=argv[1];fs::create_directories(root);
 const char* files[]={"save0.dat","refinement-v1.txt","titles-v1.txt","codex-v1.txt","survival-v1.txt"};
 std::string mode=argv[2];
 if(mode=="recover-old"||mode=="recover-old-missing-title"||mode=="recover-new"){
  destinia::SaveTransaction tx;tx.configure(root);tx.recover();
  for(auto n:files){if(mode=="recover-old-missing-title"&&std::string(n)=="titles-v1.txt"){if(fs::exists(root/n))return 5;continue;}std::ifstream f(root/n,std::ios::binary);std::string v((std::istreambuf_iterator<char>(f)),{});if(v!=(mode=="recover-new"?"new":"old"))return 4;}
  return 0;
 }
 int point=std::stoi(mode);const bool missing=point>10;if(missing)point-=10;for(auto n:files){if(missing&&std::string(n)=="titles-v1.txt")continue;std::ofstream f(root/n,std::ios::binary);f<<"old";}
 Crash ops(point);destinia::SaveTransaction tx(ops);tx.configure(root);tx.begin();
 for(auto n:files){FILE*f=tx.open(root/n,"wb");if(!f||std::fwrite("new",1,3,f)!=3||tx.close(f))return 2;}
 tx.finish(true);return 3;
}
