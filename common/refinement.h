#pragma once
#include <array>
#include "refinement_system.h"
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <limits>
namespace destinia { namespace refinement {
using Bytes=std::array<uint8_t,20>;
constexpr int64_t MaxWallet=2147483647;
enum class Kind {Weapon,Armor,Other};
enum class Error {None,InvalidIdentity,InvalidKind,MaxLevel,InsufficientGold,InsufficientCrystals};
struct Result {Error error=Error::None;int level=0;int64_t goldCost=0;int crystalsCost=0;int stone=0;explicit operator bool()const{return error==Error::None;}};
inline bool valid(int slot,int pool){return slot>=0&&slot<10&&pool>=0&&pool<200;}
inline std::string tag(int level){if(level<0||level>15)throw std::runtime_error("INVALID LEVEL");return level?"[+"+std::to_string(level)+"]":"";}
inline int64_t scaleRelevant(int64_t base,int level){return refinement_policy::weaponAttack(base,level);}
class Store {
 struct Record{Bytes bytes;int level;};
 mutable std::map<std::pair<int,int>,Record> records;
 std::array<std::array<int64_t,3>,10> wallets{};
 int peek(int slot,int pool,const Bytes& bytes)const{auto i=records.find({slot,pool});return valid(slot,pool)&&i!=records.end()&&i->second.bytes==bytes?i->second.level:0;}
public:
 // Observing a replacement permanently retires metadata of the former item.
 // Quotes use peek instead: rejected transactions must not mutate metadata.
 int level(int slot,int pool,const Bytes& bytes)const{if(!valid(slot,pool))return 0;auto i=records.find({slot,pool});if(i==records.end())return 0;if(i->second.bytes!=bytes){records.erase(i);return 0;}return i->second.level;}
 int64_t crystals(int slot,int stone=0)const{return slot>=0&&slot<10&&stone>=0&&stone<3?wallets[slot][stone]:0;}
 bool addCrystals(int slot,int64_t amount,int stone=0){if(slot<0||slot>=10||stone<0||stone>=3||amount<0||amount>MaxWallet-wallets[slot][stone])return false;wallets[slot][stone]+=amount;return true;}
 Result quote(int slot,int pool,const Bytes& bytes,Kind kind)const{
  Result r;r.level=peek(slot,pool,bytes);if(!valid(slot,pool)){r.error=Error::InvalidIdentity;return r;}
  if(kind!=Kind::Weapon&&kind!=Kind::Armor){r.error=Error::InvalidKind;return r;}
  if(r.level==15){r.error=Error::MaxLevel;return r;}int n=r.level+1;r.goldCost=500*n*n;r.crystalsCost=n;r.stone=int(refinement_policy::stoneForTarget(n));return r;
 }
 Result upgrade(int slot,int pool,const Bytes& bytes,Kind kind,int64_t& gold){
  auto r=quote(slot,pool,bytes,kind);if(!r)return r;
  if(gold<r.goldCost){r.error=Error::InsufficientGold;return r;}
  if(wallets[slot][r.stone]<r.crystalsCost){r.error=Error::InsufficientCrystals;return r;}
  records[{slot,pool}]={bytes,r.level+1};gold-=r.goldCost;wallets[slot][r.stone]-=r.crystalsCost;++r.level;return r;
 }
 std::string encode()const{
  std::ostringstream out;out<<"DESTINIA_REFINEMENT_V1\n";for(int i=0;i<10;++i)out<<"W "<<i<<' '<<wallets[i][0]<<' '<<wallets[i][1]<<' '<<wallets[i][2]<<'\n';
  const char* hex="0123456789abcdef";for(const auto& e:records){out<<"R "<<e.first.first<<' '<<e.first.second<<' ';for(auto b:e.second.bytes)out<<hex[b>>4]<<hex[b&15];out<<' '<<e.second.level<<'\n';}return out.str();
 }
 void decode(const std::string& text){
  if(text.empty()||text.size()>120000||text.back()!='\n')throw std::runtime_error("INVALID REFINEMENT");
  Store next;std::istringstream in(text);std::string line;std::getline(in,line);if(line!="DESTINIA_REFINEMENT_V1")throw std::runtime_error("INVALID REFINEMENT");
  for(int slot=0;slot<10;++slot){if(!std::getline(in,line))throw std::runtime_error("MISSING WALLET");std::istringstream row(line);char type;int id;int64_t amount,pure,abyssal;std::string extra;if(!(row>>type>>id>>amount>>pure>>abyssal)||row>>extra||type!='W'||id!=slot||amount<0||amount>MaxWallet||pure<0||pure>MaxWallet||abyssal<0||abyssal>MaxWallet)throw std::runtime_error("INVALID WALLET");next.wallets[slot]={amount,pure,abyssal};}
  std::pair<int,int> previous{-1,-1};while(std::getline(in,line)){std::istringstream row(line);char type;int slot,pool,lev;std::string hex,extra;
   if(!(row>>type>>slot>>pool>>hex>>lev)||row>>extra||type!='R'||!valid(slot,pool)||lev<1||lev>15||hex.size()!=40||std::make_pair(slot,pool)<=previous)throw std::runtime_error("INVALID RECORD");
   Bytes bytes{};std::string digits="0123456789abcdef";for(int i=0;i<20;++i){auto h=digits.find(hex[i*2]),l=digits.find(hex[i*2+1]);if(h==std::string::npos||l==std::string::npos)throw std::runtime_error("INVALID FINGERPRINT");bytes[i]=uint8_t(h*16+l);}previous={slot,pool};next.records[previous]={bytes,lev};
  }
  // Canonical representation is the shared native/web grammar; rejects whitespace, signs and leading zeros.
  if(next.encode()!=text)throw std::runtime_error("NONCANONICAL REFINEMENT");
  *this=std::move(next);
 }
};
}}
