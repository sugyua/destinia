#include "../common/safe_save.h"
#include <cstdlib>
#include <fstream>

// Process death after a completed batch's marker has been removed is a
// different failure point from death while replacing the active files.
struct InterruptedCleanup : destinia::AtomicSaveOps {
 bool owner=false,beforePromotion=false,duringInit=false;
 void afterInitializationDirectoryCreated(const destinia::fs::path&) const override {if(duringInit)std::_Exit(76);}
 void afterCommitMarker(const destinia::fs::path&) const override {if(beforePromotion)std::_Exit(75);}
 void removeJournal(const destinia::fs::path& journal) const override {
  destinia::fs::remove(journal / (owner?"owner":"committed"));
  std::_Exit(74);
 }
};
struct FailedCleanup : destinia::AtomicSaveOps {
 void removeJournal(const destinia::fs::path&) const override {throw std::runtime_error("cleanup unavailable");}
};
int main(int argc,char** argv){
 if(argc!=3)return 1;
 const destinia::fs::path root=argv[1];destinia::fs::create_directories(root);
 if(std::string(argv[2])=="cleanup-error"){
  std::ofstream(root/"save0.dat")<<"old";std::ofstream(root/"titles-v1.txt")<<"old";
  FailedCleanup ops;destinia::SaveTransaction tx(ops);tx.configure(root);tx.begin();
  if(tx.remove(root/"save0.dat"))return 6;
  FILE* f=tx.open(root/"titles-v1.txt","wb");if(!f||std::fputs("new",f)<0||tx.close(f))return 7;
  try{tx.finish(true);return 8;}catch(const std::exception&){}
  if(!tx.wasCommitted()||tx.committedRemovals().size()!=1||destinia::fs::exists(root/"save0.dat"))return 9;
  destinia::SaveTransaction retry;retry.configure(root);retry.recover();
  std::ifstream in(root/"titles-v1.txt");std::string value;in>>value;return value=="new"?0:10;
 }
 if(std::string(argv[2])=="new-after-init"){
  destinia::SaveTransaction tx;tx.configure(root);tx.recover();tx.begin();
  for(const char* name:{"save0.dat","titles-v1.txt"}){
   FILE* f=tx.open(root/name,"wb");if(!f||std::fputs("new",f)<0||tx.close(f))return 11;
  }
  tx.finish(true);return 0;
 }
 if(std::string(argv[2])=="crash"||std::string(argv[2])=="crash-owner"||std::string(argv[2])=="crash-before-promotion"||std::string(argv[2])=="crash-init"){
  for(const char* name:{"save0.dat","titles-v1.txt"})std::ofstream(root/name)<<"old";
  InterruptedCleanup ops;ops.owner=std::string(argv[2])=="crash-owner";ops.beforePromotion=std::string(argv[2])=="crash-before-promotion";ops.duringInit=std::string(argv[2])=="crash-init";destinia::SaveTransaction tx(ops);tx.configure(root);tx.begin();
  for(const char* name:{"save0.dat","titles-v1.txt"}){
   FILE* f=tx.open(root/name,"wb");if(!f||std::fputs("new",f)<0||tx.close(f))return 2;
  }
  tx.finish(true);return 3;
 }
 try {destinia::SaveTransaction tx;tx.configure(root);tx.recover();} catch(...) {return 4;}
 for(const char* name:{"save0.dat","titles-v1.txt"}){
  std::ifstream in(root/name);std::string value;in>>value;if(value!=(std::string(argv[2])=="recover-old"?"old":"new"))return 5;
 }
 return 0;
}
