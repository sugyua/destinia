#include <cassert>
#include <fstream>
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
int main(int argc,char** argv){
 assert(argc==2);saveDir=argv[1];fs::create_directories(saveDir);
 const auto file=fs::path(saveDir)/"codex-v1.txt";
 {std::ofstream f(file);f<<"sentinela de dados";}
 const auto dir=fs::absolute(saveDir).lexically_normal();
 const auto journal=dir.parent_path()/(dir.filename().string()+".transaction");
 fs::create_directories(journal);{std::ofstream f(journal/"manifest");f<<"recovery pending";}
 codexAvailable=true;codexWritable=true;codex.dirty=true;codex.entries[0][4].kills=42;
 char program[]="fixture", missing[]="--save";char* args[]={program,missing};
 assert(originalMain(2,args)!=0);
 std::ifstream f(file);std::string text((std::istreambuf_iterator<char>(f)),{});
 if(text!="sentinela de dados")return 19;
 std::ifstream marker(journal/"manifest");std::string journalText((std::istreambuf_iterator<char>(marker)),{});
 if(journalText!="recovery pending")return 20;
 std::puts("FATAL_SHUTDOWN_PRESERVES_CODEX");
}
