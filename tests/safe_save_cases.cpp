#include "../common/safe_save.h"
#include <cassert>
#include <iostream>
using namespace destinia;
static std::string contents(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void write(SaveTransaction& tx,const fs::path&p,const std::string&s,const char*mode="wb"){FILE*f=tx.open(p,mode);assert(f);assert(std::fwrite(s.data(),1,s.size(),f)==s.size());assert(tx.close(f)==0);}
struct FailReplace:AtomicSaveOps{mutable int count=0;int when;bool crash;FailReplace(int n,bool die=false):when(n),crash(die){}void replace(const fs::path&a,const fs::path&b)const override{if(++count==when){if(crash)std::_Exit(73);throw std::runtime_error("replace sem espaco");}AtomicSaveOps::replace(a,b);}};
int main(int argc,char**argv){
 assert(argc>=2);fs::path root=argv[1];fs::create_directories(root);
 if(argc==3){const bool deleting=std::string(argv[2])=="delete-crash";FailReplace ops(deleting?1:2,true);SaveTransaction tx(ops);tx.configure(root);tx.recover();tx.begin();if(deleting){assert(tx.remove(root/"a.dat")==0);}else write(tx,root/"a.dat","novoA");write(tx,root/"b.dat","novoB");tx.finish(true);return 99;}
 std::ofstream(root/"a.dat")<<"antigoA";std::ofstream(root/"b.dat")<<"antigoB";
 SaveTransaction tx;tx.configure(root);tx.recover();tx.begin();write(tx,root/"a.dat","novoA");write(tx,root/"b.dat","novoB");
 assert(contents(root/"a.dat")=="antigoA");assert(contents(root/"b.dat")=="antigoB");
 FILE*r=tx.open(root/"a.dat","rb");char b[6]={};assert(std::fread(b,1,5,r)==5);assert(std::string(b)=="novoA");assert(tx.close(r)==0);
 tx.finish(true);assert(contents(root/"a.dat")=="novoA"&&contents(root/"b.dat")=="novoB");
 tx.begin();write(tx,root/"a.dat","falhou");tx.finish(false);assert(contents(root/"a.dat")=="novoA");
 tx.begin();write(tx,root/"a.dat","+","ab");tx.finish(true);assert(contents(root/"a.dat")=="novoA+");
 tx.begin();FILE*f=tx.open(root/"a.dat","r+b");assert(f);std::fseek(f,0,SEEK_SET);std::fputs("N",f);assert(tx.close(f)==0);tx.finish(true);assert(contents(root/"a.dat")=="NovoA+");
 tx.begin();assert(tx.open(root/"nao-existe.dat","r+b")==nullptr);tx.finish(true);assert(!fs::exists(root/"nao-existe.dat"));
 // Unclosed writer and stream error cannot publish anything.
 tx.begin();f=tx.open(root/"a.dat","wb");assert(f);std::fputs("parcial",f);bool rejected=false;try{tx.finish(true);}catch(...){rejected=true;}assert(rejected&&contents(root/"a.dat")=="NovoA+");
 tx.begin();write(tx,root/"a.dat","outro");f=tx.open(root/"a.dat","rb");assert(f);assert(std::fputc('x',f)==EOF);assert(tx.close(f)!=0);rejected=false;try{tx.finish(true);}catch(...){rejected=true;}assert(rejected&&contents(root/"a.dat")=="NovoA+");
 {FailReplace ops(2);SaveTransaction failing(ops);failing.configure(root);failing.begin();write(failing,root/"a.dat","trocadoA");write(failing,root/"b.dat","trocadoB");rejected=false;try{failing.finish(true);}catch(...){rejected=true;}assert(rejected&&contents(root/"a.dat")=="NovoA+"&&contents(root/"b.dat")=="novoB");}
 tx.begin();rejected=false;try{tx.open(root/"../fora.dat","wb");}catch(...){rejected=true;}assert(rejected);tx.finish(false);
 std::error_code ec;std::ofstream(root.parent_path()/"intocado.dat")<<"fora";fs::create_symlink(root.parent_path()/"intocado.dat",root/"link.dat",ec);
 if(!ec){tx.begin();rejected=false;try{tx.open(root/"link.dat","wb");}catch(...){rejected=true;}assert(rejected);tx.finish(false);assert(contents(root.parent_path()/"intocado.dat")=="fora");fs::remove(root/"link.dat");}
 tx.begin();assert(tx.remove(root/"a.dat")==0);assert(contents(root/"a.dat")=="NovoA+");assert(tx.open(root/"a.dat","rb")==nullptr);tx.finish(false);assert(contents(root/"a.dat")=="NovoA+");
 tx.begin();assert(tx.remove(root/"a.dat")==0);assert(tx.open(root/"a.dat","r+b")==nullptr);write(tx,root/"a.dat","recriado","ab");tx.finish(true);assert(contents(root/"a.dat")=="recriado");
 tx.begin();write(tx,root/"c.dat","efemero");assert(tx.remove(root/"c.dat")==0);tx.finish(true);assert(!fs::exists(root/"c.dat"));
 tx.begin();assert(tx.remove(root/"a.dat")==0);tx.finish(true);assert(!fs::exists(root/"a.dat"));
 std::ofstream(root/"a.dat")<<"checkpoint";
 std::cout<<"SAFE_SAVE_OK\n";
}
