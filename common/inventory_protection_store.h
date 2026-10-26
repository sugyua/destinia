#pragma once
#include "inventory_protection.h"
#include "safe_save.h"
namespace destinia {
// Fixed sidecar name; this API cannot target an original save*.dat.
inline fs::path inventoryProtectionFile(const fs::path& directory){return directory/"inventory-protect-v1.txt";}
inline void validateInventoryProtectionFile(const fs::path& directory){
 const auto file=inventoryProtectionFile(directory);
 if(isLink(directory)||isLink(file)||(fs::exists(file)&&(!fs::is_regular_file(file)||fs::file_size(file)>100000)))throw std::runtime_error("SIDECAR INVALIDO - ORIGINAL PRESERVADO");
}
inline void loadInventoryProtection(const fs::path& directory,InventoryProtection& protection){
 validateInventoryProtectionFile(directory);const auto file=inventoryProtectionFile(directory);if(!fs::exists(file))return;
 std::ifstream in(file,std::ios::binary);if(!in)throw std::runtime_error("FALHA LENDO PROTECOES");
 const std::string text{std::istreambuf_iterator<char>(in),{}};if(in.bad())throw std::runtime_error("FALHA LENDO PROTECOES");protection.decode(text);
}
inline void saveInventoryProtection(const fs::path& directory,const InventoryProtection& protection,const AtomicSaveOps& ops=AtomicSaveOps{}){
 validateInventoryProtectionFile(directory);InventoryProtection existing;loadInventoryProtection(directory,existing);fs::create_directories(directory);
 const auto file=inventoryProtectionFile(directory);const fs::path temp=file.string()+"."+uniqueId()+".tmp";
 try{
  const auto text=protection.encode();FILE* out=std::fopen(temp.string().c_str(),"wb");if(!out)throw std::runtime_error("FALHA CRIANDO SIDECAR");
  bool okay=std::fwrite(text.data(),1,text.size(),out)==text.size()&&std::fflush(out)==0;
#ifdef _WIN32
  if(okay)okay=::_commit(::_fileno(out))==0;
#else
  if(okay)okay=::fsync(::fileno(out))==0;
#endif
  if(std::fclose(out)!=0)okay=false;if(!okay)throw std::runtime_error("FALHA GRAVANDO PROTECOES");
  validateInventoryProtectionFile(directory);ops.replace(temp,file);
 }catch(...){std::error_code ec;fs::remove(temp,ec);throw;}
}
}
