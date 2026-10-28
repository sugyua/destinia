#include "../common/survival_store.h"
#include <cassert>
#include <iostream>
using namespace destinia;
int main(int argc,char** argv){
 assert(argc==2);fs::path dir=argv[1];fs::create_directories(dir);
 std::ofstream(dir/"save0.dat",std::ios::binary)<<"original-save-fixture";
 survival::Ledger l;survival::save(dir,l);auto old=survival::readText(dir);assert(old);
 l.slots[0].bestWave=20;l.slots[0].totalKills=37;l.slots[0].bestTimeSeconds=82;l.slots[0].survivalScore=9123;l.slots[0].titleUnlocked=true;
 bool stale=false;try{survival::save(dir,l);}catch(...){stale=true;}assert(stale&&survival::readText(dir)==old);
 struct Failure:AtomicSaveOps{void replace(const fs::path&,const fs::path&)const override{throw std::runtime_error("injected");}} fail;
 bool failed=false;try{survival::save(dir,l,fail,old);}catch(...){failed=true;}assert(failed&&survival::readText(dir)==old);
 survival::save(dir,l,AtomicSaveOps{},old);auto loaded=survival::load(dir);assert(loaded->slots[0].bestWave==20&&loaded->slots[0].totalKills==37&&loaded->slots[0].bestTimeSeconds==82&&loaded->slots[0].survivalScore==9123);
 std::ifstream in(dir/"save0.dat",std::ios::binary);std::string text((std::istreambuf_iterator<char>(in)),{});assert(text=="original-save-fixture");
 std::cout<<"SURVIVAL_ATOMIC_CAS_OK\n";
}
