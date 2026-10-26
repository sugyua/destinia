#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
namespace destinia {
// Sidecar metadata only. No ARM equipment bits or save*.dat writes.
class InventoryProtection {
 using Key=std::pair<int,int>;
 std::map<Key,std::array<uint8_t,20>> marks;
 int character=0;
public:
 void select(int slot){if(slot<0||slot>9)throw std::runtime_error("PERSONAGEM INVALIDO");character=slot;}
 void clear(){marks.clear();}
 bool empty()const{return marks.empty();}
 bool has(int pool,const std::array<uint8_t,20>& bytes){
  auto it=marks.find({character,pool});if(it==marks.end())return false;
  if(it->second!=bytes){marks.erase(it);return false;}return true;
 }
 bool toggle(int pool,const std::array<uint8_t,20>& bytes){
  if(pool<0||pool>=200)throw std::runtime_error("POOL INVALIDO");
  if(has(pool,bytes)){forget(pool);return false;}marks[{character,pool}]=bytes;return true;
 }
 void forget(int pool){marks.erase({character,pool});}
 std::string encode()const{
  std::ostringstream out;out<<"DESTINIA_INVENTORY_PROTECT_1\n";const char* hex="0123456789abcdef";
  for(const auto& entry:marks){out<<entry.first.first<<' '<<entry.first.second<<' ';for(auto b:entry.second)out<<hex[b>>4]<<hex[b&15];out<<'\n';}return out.str();
 }
 void decode(const std::string& text){
  if(text.empty()||text.size()>100000||text.back()!='\n')throw std::runtime_error("SIDECAR INVALIDO");
  std::istringstream in(text);std::string line;std::getline(in,line);if(!line.empty()&&line.back()=='\r')line.pop_back();
  if(line!="DESTINIA_INVENTORY_PROTECT_1")throw std::runtime_error("FORMATO DE PROTECAO DESCONHECIDO");
  decltype(marks) next;
  while(std::getline(in,line)){
   std::istringstream row(line);int slot,pool;std::string hex,extra;
   if(!(row>>slot>>pool>>hex)||row>>extra||slot<0||slot>9||pool<0||pool>=200||hex.size()!=40||next.count({slot,pool}))throw std::runtime_error("REGISTRO DE PROTECAO INVALIDO");
   std::array<uint8_t,20> bytes{};const std::string digits="0123456789abcdef";
   for(size_t i=0;i<20;i++){auto hi=digits.find(hex[2*i]),lo=digits.find(hex[2*i+1]);if(hi==std::string::npos||lo==std::string::npos)throw std::runtime_error("FINGERPRINT INVALIDO");bytes[i]=uint8_t(hi*16+lo);}
   next[{slot,pool}]=bytes;
  }
  if(!in.eof())throw std::runtime_error("ERRO LENDO PROTECOES");marks=std::move(next);
 }
};
}
