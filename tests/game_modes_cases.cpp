#include "../common/game_modes.h"
#include <cassert>
#include <iostream>
using namespace destinia;
static bool rejected(const std::function<void()>& f){try{f();return false;}catch(const std::exception&){return true;}}
int main(int argc,char** argv){assert(argc==2);fs::path root=argv[1],classic=root/"save",expanded=modeSaveDir(classic,"expandido");fs::create_directories(classic);std::ofstream(classic/"save0.dat")<<"progress";std::ofstream(classic/"msave.dat")<<"slots";std::ofstream(classic/"private.txt")<<"not progress";
 assert(modeSaveDir(classic,"classico")==classic);assert(expanded!=classic);assert(modeSaveDir(fs::path(classic.string()+"/"),"expandido")==expanded);assert(rejected([&]{modeSaveDir(classic,"invalid");}));
 assert(rejected([&]{verifySaveMode(classic,"expandido");}));verifySaveMode(classic,"classico");assert(rejected([&]{verifySaveMode(classic,"expandido");}));
 importClassicSave(classic,expanded);assert(sameFileContents(classic/"save0.dat",expanded/"save0.dat"));assert(!fs::exists(expanded/"private.txt"));verifySaveMode(expanded,"expandido");assert(rejected([&]{verifySaveMode(expanded,"classico");}));assert(rejected([&]{importClassicSave(classic,expanded);}));std::ofstream(expanded/"save0.dat")<<"expanded";assert(!sameFileContents(classic/"save0.dat",expanded/"save0.dat"));
 FileLock lock;assert(lock.acquire(classic/".destinia.lock"));assert(rejected([&]{importClassicSave(classic,root/"locked");}));lock.release();

#ifndef _WIN32
 fs::create_directory_symlink(classic,root/"link");assert(rejected([&]{importClassicSave(root/"link",root/"bad");}));
#endif
 fs::create_directories(root/"empty");assert(rejected([&]{importClassicSave(root/"empty",root/"bad");}));
 std::cout<<"SAVE_MODES_OK\n";
}
