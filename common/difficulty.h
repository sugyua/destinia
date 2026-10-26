#pragma once
#include "progression_store.h"
namespace destinia {
inline std::string difficultyLabel(unsigned d){static const char* labels[]={"Normal",u8"Dif\u00edcil","Pesadelo","Inferno","Abissal"};if(d>=5)throw std::invalid_argument("invalid difficulty");return labels[d];}
struct DifficultyProgress {
 progression::Ledger book;std::array<unsigned,10> unlocked{};
 std::optional<std::string> previousBook,previousUnlocks;
 unsigned current(unsigned slot)const{if(slot>=10)throw std::invalid_argument("invalid slot");auto i=book.characters.find(slot);return i==book.characters.end()?0:unsigned(i->second.difficulty);}
 bool allowed(unsigned slot,unsigned d)const{return slot<10&&d<5&&(d<=unlocked[slot]||d==current(slot));}
 bool change(unsigned slot,unsigned d){if(!allowed(slot,d))return false;book.characters[slot].difficulty=progression::Difficulty(d);return true;}
 bool defeated(unsigned slot,int species){if(slot>=10||species!=108)return false;const auto d=current(slot);if(d>=4||d!=unlocked[slot])return false;unlocked[slot]=d+1;return true;}
 void erase(unsigned slot){if(slot>=10)throw std::invalid_argument("invalid slot");book.characters.erase(slot);unlocked[slot]=0;}
};
inline std::optional<std::string> readDifficultyUnlocks(const fs::path& directory){const auto path=directory/"difficulty-unlocks-v1.txt";if(isLink(directory)||isLink(path))throw std::runtime_error("linked difficulty file");if(!fs::exists(path))return std::nullopt;if(!fs::is_regular_file(path)||fs::file_size(path)>1024)throw std::runtime_error("invalid difficulty file");std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("cannot read difficulty");std::string text{std::istreambuf_iterator<char>(in),{}};if(in.bad())throw std::runtime_error("cannot read difficulty");return text;}
inline DifficultyProgress loadDifficulty(const fs::path& directory,progression::Mode mode){DifficultyProgress p;p.book.mode=mode;p.previousBook=progression::readText(directory);if(p.previousBook)p.book=progression::decode(*p.previousBook,mode);p.previousUnlocks=readDifficultyUnlocks(directory);if(p.previousUnlocks){std::istringstream in(*p.previousUnlocks);std::string magic;in>>magic;if(magic!="DESTINIA_DIFFICULTY_UNLOCKS_1")throw std::runtime_error("invalid difficulty header");for(auto& n:p.unlocked)if(!(in>>n)||n>4)throw std::runtime_error("invalid difficulty unlock");if(in>>magic)throw std::runtime_error("trailing difficulty data");}else for(const auto& character:p.book.characters)p.unlocked[character.first]=unsigned(character.second.difficulty);return p;}
inline void stageDifficulty(SaveTransaction& tx,const fs::path& directory,const DifficultyProgress& p){
 const auto book=progression::encode(p.book);std::ostringstream out;out<<"DESTINIA_DIFFICULTY_UNLOCKS_1\n";for(auto n:p.unlocked){if(n>4)throw std::runtime_error("invalid unlock");out<<n<<' ';}out<<'\n';const auto unlocks=out.str();
 for(const auto& file:std::array<std::pair<const char*,std::string>,2>{{{progression::SidecarName,book},{"difficulty-unlocks-v1.txt",unlocks}}}){FILE* f=tx.open(directory/file.first,"wb");if(!f)throw std::runtime_error("cannot create difficulty");const bool ok=std::fwrite(file.second.data(),1,file.second.size(),f)==file.second.size();const int closed=tx.close(f);if(!ok||closed)throw std::runtime_error("cannot write difficulty");}
}
inline void saveDifficulty(const fs::path& directory,DifficultyProgress& p,const AtomicSaveOps& ops=AtomicSaveOps{}){
 fs::create_directories(directory);FileLock lock;if(!lock.acquire(directory/"progression-v1.lock"))throw std::runtime_error("progression busy");
 if(progression::readText(directory)!=p.previousBook||readDifficultyUnlocks(directory)!=p.previousUnlocks)throw std::runtime_error("stale difficulty snapshot");
 SaveTransaction tx(ops);tx.configure(directory);SaveCall call(tx);stageDifficulty(tx,directory,p);call.commit();p.previousBook=progression::readText(directory);p.previousUnlocks=readDifficultyUnlocks(directory);
}
}
