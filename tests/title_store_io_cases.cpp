#include "../common/titles.h"
#include <cassert>
#include <fstream>
#undef assert
#define assert(expr) do { if(!(expr)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#expr);return 1;} } while(false)
using namespace destinia;
static std::string read(const fs::path& p){std::ifstream in(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(in),{});}
int main(){
 auto dir=fs::temp_directory_path()/("destinia-title-io-"+uniqueId());fs::create_directories(dir);
 auto path=dir/"titles-v1.txt",legacy=dir/"titles-v1.txt.tmp",sentinel=dir/"sentinel";
 {std::ofstream f(path);f<<"DESTINIA_TITLES_V1\nequipped 0 7\n";}
 const auto bad=read(path);TitlesStore store;assert(!store.load(dir));store.unlock(0,TitleId::MestreDaSobrevivencia);assert(!store.save(dir));assert(read(path)==bad);
 {std::ofstream f(path);f<<"DESTINIA_TITLES_V1\nunlocked 0 7\nend\n";}
 assert(store.load(dir));{std::ofstream f(sentinel);f<<"SENTINELA";}
 std::error_code ec;fs::create_hard_link(sentinel,legacy,ec);if(ec)return 3;
 store.setEquipped(0,TitleId::SenhorDoAbismo);assert(store.save(dir));assert(read(sentinel)=="SENTINELA");assert(fs::exists(legacy));
 TitlesStore reloaded;assert(reloaded.load(dir));assert(reloaded.getEquipped(0)==TitleId::SenhorDoAbismo);assert(reloaded.isUnlocked(0,TitleId::SenhorDoAbismo));
 // A concurrent deletion owns the title lock; no title update may publish
 // until that transaction finishes.
 FileLock held;assert(held.acquire(dir/"titles-v1.lock"));
 reloaded.setEquipped(0,TitleId::Iniciante);const auto lockedSnapshot=read(path);
 assert(!reloaded.save(dir));assert(read(path)==lockedSnapshot);held.release();
 assert(held.acquire(dir/"titles-v1.lock"));
 assert(!persistTitleUnlock(reloaded,0,TitleId::MestreDaSobrevivencia,dir));
 assert(!reloaded.isUnlocked(0,TitleId::MestreDaSobrevivencia));assert(read(path)==lockedSnapshot);
 held.release();assert(persistTitleUnlock(reloaded,0,TitleId::MestreDaSobrevivencia,dir));
 assert(reloaded.isUnlocked(0,TitleId::MestreDaSobrevivencia));
 {std::ofstream f(path);f<<"DESTINIA_TITLES_V1\nequipped 1 1\nend\n";}
 const auto external=read(path);store.setEquipped(0,TitleId::Iniciante);assert(!store.save(dir));assert(read(path)==external);
 TitlesStore untracked;assert(!untracked.save(dir));assert(read(path)==external);
 fs::remove(legacy);fs::remove(sentinel);fs::remove(path);fs::remove(dir/"titles-v1.lock");fs::remove(dir);
}
