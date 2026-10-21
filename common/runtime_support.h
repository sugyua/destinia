#pragma once
// Suporte compartilhado pelos hosts; nao altera a logica ARM ou o formato de save.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace destinia {
namespace fs = std::filesystem;

inline bool isLink(const fs::path& path) {
#ifdef _WIN32
  DWORD attrs=GetFileAttributesW(path.c_str());
  if(attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_REPARSE_POINT)) return true;
#endif
  return fs::is_symlink(fs::symlink_status(path));
}

// Trava liberada pelo sistema mesmo se o processo falhar; o arquivo nao e o save.
class FileLock {
#ifdef _WIN32
  HANDLE handle=INVALID_HANDLE_VALUE;
#else
  int handle=-1;
#endif
public:
  FileLock()=default;
  FileLock(const FileLock&)=delete;
  FileLock& operator=(const FileLock&)=delete;
  bool acquire(const fs::path& path) {
    release();
    if(isLink(path)) return false;
#ifdef _WIN32
    handle=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    return handle!=INVALID_HANDLE_VALUE;
#else
    handle=::open(path.c_str(),O_RDWR|O_CREAT|O_NOFOLLOW,0600);
    if(handle<0) return false;
    if(::flock(handle,LOCK_EX|LOCK_NB)!=0) { release(); return false; }
    return true;
#endif
  }
  void release() {
#ifdef _WIN32
    if(handle!=INVALID_HANDLE_VALUE) { CloseHandle(handle); handle=INVALID_HANDLE_VALUE; }
#else
    if(handle>=0) { ::close(handle); handle=-1; }
#endif
  }
  ~FileLock() { release(); }
};

inline std::string utcTime(const char* format) {
  const auto now=std::chrono::system_clock::now();
  const std::time_t t=std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  gmtime_s(&tm,&t);
#else
  gmtime_r(&t,&tm);
#endif
  char text[64]; std::strftime(text,sizeof text,format,&tm); return text;
}
inline std::string uniqueId() {
  static unsigned counter=0;
  auto ticks=std::chrono::system_clock::now().time_since_epoch().count();
#ifdef _WIN32
  const auto pid=GetCurrentProcessId();
#else
  const auto pid=getpid();
#endif
  return utcTime("%Y%m%d_%H%M%S")+"_"+std::to_string(ticks)+"_"+std::to_string(pid)+"_"+std::to_string(++counter);
}

inline fs::path localDataDir() {
  const char* base=std::getenv("LOCALAPPDATA");
  return base && *base ? fs::path(base)/"Destinia" : fs::path();
}
inline std::string trim(std::string text) {
  auto first=text.find_first_not_of(" \t\r\n");
  if(first==std::string::npos) return {};
  return text.substr(first,text.find_last_not_of(" \t\r\n")-first+1);
}
inline fs::path configuredSaveDir() {
  const char* overrideDir=std::getenv("DESTINIA_SAVE_DIR");
  std::string selected;
  if(overrideDir && *overrideDir) selected=overrideDir;
  else if(!localDataDir().empty()) {
    auto path=localDataDir()/"config.ini";
    std::ifstream config(path,std::ios::binary);
    if(!config && fs::exists(path)) throw std::runtime_error("Nao foi possivel ler config.ini; a pasta de save nao sera trocada.");
    std::string line; bool found=false;
    while(std::getline(config,line)) {
      if(line.compare(0,3,"\xef\xbb\xbf")==0) line.erase(0,3);
      line=trim(line);
      if(line.empty() || line[0]=='#' || line[0]==';') continue;
      const auto equal=line.find('=');
      if(equal==std::string::npos) throw std::runtime_error("Linha invalida em config.ini.");
      if(trim(line.substr(0,equal))=="save_dir") {
        if(found) throw std::runtime_error("save_dir repetido em config.ini.");
        selected=trim(line.substr(equal+1)); found=true;
        if(selected.empty()) throw std::runtime_error("save_dir vazio em config.ini.");
      }
    }
  }
  if(selected.empty()) return localDataDir().empty() ? fs::path("save") : localDataDir()/"save";
  auto path=fs::u8path(selected);
  if(!path.is_absolute() || path==path.root_path()) throw std::runtime_error("A pasta configurada para saves precisa ser absoluta e diferente da raiz do disco.");
  return path.lexically_normal();
}

inline fs::path defaultReportsDir() {
  std::vector<fs::path> starts;
#ifdef _WIN32
  std::array<wchar_t,32768> buffer{};
  DWORD size=GetModuleFileNameW(nullptr,buffer.data(),static_cast<DWORD>(buffer.size()));
  if(size && size<buffer.size()) starts.push_back(fs::path(buffer.data()).parent_path());
#endif
  starts.push_back(fs::current_path());
  for(auto start:starts) for(int depth=0;depth<5 && !start.empty();++depth,start=start.parent_path())
    if(fs::is_regular_file(start/"VERSION") && fs::is_directory(start/"host")) return start/"relatorios";
  return fs::current_path()/"relatorios";
}

class SessionLog {
  fs::path folder;
  std::ofstream output;
  FileLock lock;
  size_t maxBytes=2*1024*1024, bytes=0;
  std::chrono::steady_clock::time_point lastFlush;
  bool enabled=false;
  void rotate() {
    output.close();
    fs::remove(folder/"log.2.txt");
    if(fs::exists(folder/"log.1.txt")) fs::rename(folder/"log.1.txt",folder/"log.2.txt");
    fs::rename(folder/"log.txt",folder/"log.1.txt");
    output.open(folder/"log.txt",std::ios::binary|std::ios::trunc); bytes=0;
    if(!output) throw std::runtime_error("Nao foi possivel rotacionar o log.");
  }
public:
  bool start(const fs::path& root,const std::string& variant,size_t limit=2*1024*1024,size_t sessions=10) {
    close(); maxBytes=std::max<size_t>(limit,128);
    try {
      fs::create_directories(root);
      folder=root/("jogo_"+variant+"_"+uniqueId());
      if(!fs::create_directory(folder)) throw std::runtime_error("Pasta de log ja existe.");
      if(!lock.acquire(folder/".active.lock")) throw std::runtime_error("Nao foi possivel proteger o log.");
      std::ofstream(folder/".destinia-session")<<"Destinia log v1\n";
      output.open(folder/"log.txt",std::ios::binary|std::ios::trunc);
      if(!output) throw std::runtime_error("Nao foi possivel criar o log.");
      enabled=true; bytes=0; lastFlush=std::chrono::steady_clock::now();
      std::vector<fs::path> old;
      const std::string prefix="jogo_"+variant+"_";
      for(auto& entry:fs::directory_iterator(root)) {
        auto p=entry.path();
        if(!isLink(p) && entry.is_directory() && p.filename().string().find(prefix)==0 &&
           !isLink(p/".destinia-session") && fs::is_regular_file(p/".destinia-session")) old.push_back(p);
      }
      std::sort(old.begin(),old.end());
      size_t remaining=old.size();
      for(auto& p:old) {
        if(remaining<=std::max<size_t>(sessions,1)) break;
        if(p==folder) continue;
        FileLock candidate;
        if(!candidate.acquire(p/".active.lock")) continue;
        candidate.release(); fs::remove_all(p); --remaining;
      }
      return true;
    } catch(const std::exception& error) {
      std::fprintf(stderr,"[host] AVISO: log TXT indisponivel: %s\n",error.what());
      // Se a limpeza de logs antigos falhar, manter o log novo quando possivel.
      if(enabled && output) return true;
      close(); return false;
    }
  }
  const fs::path& directory() const { return folder; }
  void write(std::string message,bool flushNow=false) {
    if(!enabled) return;
    try {
      while(!message.empty() && (message.back()=='\n' || message.back()=='\r')) message.pop_back();
      std::string line=utcTime("%Y-%m-%dT%H:%M:%SZ")+" "+message+"\n";
      if(line.size()>maxBytes) line=line.substr(0,maxBytes-15)+" [truncado]\n";
      if(bytes+line.size()>maxBytes && bytes) rotate();
      output.write(line.data(),static_cast<std::streamsize>(line.size())); bytes+=line.size();
      auto now=std::chrono::steady_clock::now();
      if(flushNow || now-lastFlush>=std::chrono::seconds(1)) { output.flush(); lastFlush=now; }
      if(!output) throw std::runtime_error("Falha ao escrever o log (disco/permissoes).");
    } catch(const std::exception& error) {
      std::fprintf(stderr,"[host] AVISO: log TXT interrompido: %s\n",error.what()); close();
    }
  }
  void close() { enabled=false; if(output.is_open()) output.close(); lock.release(); }
  ~SessionLog() { close(); }
};
inline SessionLog sessionLog;

// Conserva o stderr usado pelo diagnostico e o duplica no TXT, com entrada limitada.
inline void hostReport(const char* format,...) {
  va_list args,copy; va_start(args,format); va_copy(copy,args);
  std::vfprintf(stderr,format,args); va_end(args);
  std::array<char,4096> text{};
  int needed=std::vsnprintf(text.data(),text.size(),format,copy); va_end(copy);
  if(needed>=0) sessionLog.write(std::string(text.data())+(static_cast<size_t>(needed)>=text.size()?" [truncado]":""));
}

inline bool sameFileContents(const fs::path& a,const fs::path& b) {
  if(fs::file_size(a)!=fs::file_size(b)) return false;
  std::ifstream left(a,std::ios::binary),right(b,std::ios::binary);
  if(!left || !right) return false;
  std::array<char,16384> x{},y{};
  while(left) {
    left.read(x.data(),x.size()); right.read(y.data(),y.size());
    if(left.gcount()!=right.gcount() || !std::equal(x.begin(),x.begin()+left.gcount(),y.begin())) return false;
  }
  return left.eof() && right.eof();
}

// A pasta selecionada ja deve estar protegida por FileLock durante esta operacao.
inline fs::path backupSave(const fs::path& selected,size_t keep=10) {
  const auto source=fs::canonical(selected);
  if(source==source.root_path()) throw std::runtime_error("A raiz do disco nao pode ser usada como pasta de save.");
  std::vector<fs::path> files;
  for(auto& entry:fs::recursive_directory_iterator(source)) {
    if(isLink(entry.path())) throw std::runtime_error("Backup recusado: link encontrado dentro da pasta de save.");
    if(entry.is_regular_file() && entry.path().filename()!=".destinia.lock") files.push_back(entry.path());
    else if(!entry.is_directory() && entry.path().filename()!=".destinia.lock") throw std::runtime_error("Backup recusado: arquivo especial na pasta de save.");
  }
  if(files.empty()) return {};
  const auto root=source.parent_path()/(source.filename().string()+"_backups");
  if(isLink(root)) throw std::runtime_error("Backup recusado: pasta de backups e um link.");
  fs::create_directories(root);
  const std::string id=uniqueId(); const auto temporary=root/("parcial_"+id),complete=root/("backup_"+id);
  if(!fs::create_directory(temporary)) throw std::runtime_error("Pasta de backup ja existe.");
  try {
    for(auto& file:files) {
      auto relative=file.lexically_relative(source),target=temporary/relative;
      auto modified=fs::last_write_time(file);
      fs::create_directories(target.parent_path());
      fs::copy_file(file,target,fs::copy_options::none);
      if(modified!=fs::last_write_time(file) || !sameFileContents(file,target)) throw std::runtime_error("Backup incompleto ou save alterado durante a copia; feche outras versoes do jogo.");
    }
    std::ofstream marker(temporary/".destinia-backup",std::ios::binary);
    marker<<"Destinia backup v1\nOrigem: "<<source.u8string()<<"\n"; marker.close();
    if(!marker) throw std::runtime_error("Nao foi possivel concluir o backup.");
    fs::rename(temporary,complete);
  } catch(...) { std::error_code ignored; fs::remove_all(temporary,ignored); throw; }
  std::vector<fs::path> old;
  for(auto& entry:fs::directory_iterator(root)) {
    auto p=entry.path();
    if(!isLink(p) && entry.is_directory() && p.filename().string().find("backup_")==0 &&
       !isLink(p/".destinia-backup") && fs::is_regular_file(p/".destinia-backup")) old.push_back(p);
  }
  std::sort(old.begin(),old.end());
  const size_t remove=old.size()>std::max<size_t>(keep,1)?old.size()-std::max<size_t>(keep,1):0;
  for(size_t i=0;i<remove;++i) fs::remove_all(old[i]);
  return complete;
}
} // namespace destinia
