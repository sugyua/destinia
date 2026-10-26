#pragma once
#include "progression.h"
#include "safe_save.h"
#include <atomic>
#include <optional>
namespace destinia { namespace progression {
inline fs::path file(const fs::path& directory){return directory/SidecarName;}
inline std::string temporaryId(){
 static std::atomic<uint64_t> sequence{0};
#ifdef _WIN32
 const auto pid=GetCurrentProcessId();
#else
 const auto pid=getpid();
#endif
 return std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"_"+std::to_string(pid)+"_"+std::to_string(sequence.fetch_add(1));
}
inline void validatePath(const fs::path& directory){
 require(!isLink(directory)&&!isLink(file(directory)),"linked progression sidecar");
 require(!fs::exists(directory)||fs::is_directory(directory),"invalid progression directory");
 require(!fs::exists(file(directory))||(fs::is_regular_file(file(directory))&&fs::file_size(file(directory))<=4096),"invalid progression file");
}
inline std::optional<std::string> readText(const fs::path& directory){
 validatePath(directory);if(!fs::exists(file(directory)))return std::nullopt;
 std::ifstream in(file(directory),std::ios::binary);require(bool(in),"cannot read progression");
 std::array<char,4097> buffer{};in.read(buffer.data(),buffer.size());
 require(!in.bad()&&in.gcount()<=4096,"cannot read progression");
 return std::string(buffer.data(),static_cast<size_t>(in.gcount()));
}
inline std::optional<Ledger> load(const fs::path& directory,Mode mode){const auto text=readText(directory);if(!text)return std::nullopt;return decode(*text,mode);}
// Compare-and-swap: pass the exact previous file bytes when updating. An absent
// expected snapshot only permits creation. The lock serializes cooperating hosts.
inline void save(const fs::path& directory,const Ledger& ledger,const AtomicSaveOps& ops=AtomicSaveOps{},const std::optional<std::string>& expected=std::nullopt){
 const auto text=encode(ledger);validatePath(directory);fs::create_directories(directory);
 FileLock lock;require(lock.acquire(directory/"progression-v1.lock"),"progression busy");
 const auto previous=readText(directory);if(previous)decode(*previous,ledger.mode);
 require(previous==expected,"stale progression snapshot");
 fs::path temp=file(directory);temp+="."+temporaryId()+".tmp";bool created=false;
 try{
#ifdef _WIN32
  FILE* out=::_wfopen(temp.c_str(),L"wbx");
#else
  FILE* out=std::fopen(temp.c_str(),"wbx");
#endif
  require(out!=nullptr,"cannot create progression temporary file");
  created=true;
  bool okay=std::fwrite(text.data(),1,text.size(),out)==text.size()&&std::fflush(out)==0;
#ifdef _WIN32
  if(okay)okay=::_commit(::_fileno(out))==0;
#else
  if(okay)okay=::fsync(::fileno(out))==0;
#endif
  if(std::fclose(out)!=0)okay=false;
  require(okay,"cannot flush progression");
  validatePath(directory);ops.replace(temp,file(directory));
 }catch(...){if(created){std::error_code ec;fs::remove(temp,ec);}throw;}
}
}}
