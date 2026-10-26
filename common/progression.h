#pragma once
// Canonical, host-independent progression. No ARM access and no native save I/O.
#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace destinia { namespace progression {
constexpr uint32_t MaxLevel=999, NativeMaxLevel=99, Basis=10000;
constexpr uint32_t ArmLevelOffset=0x48FA, ArmXpOffset=0x48FC, ArmStatsOffset=0x48BA;
constexpr const char* SidecarName="progression-v1.txt";
enum class Rank:uint8_t {Aprendiz,Aventureiro,Guerreiro,Cavaleiro,Campeao,Mestre,GraoMestre,Lenda,Mitico,Semideus,Divino,Transcendente,Celestial,Supremo};
enum class Difficulty:uint8_t {Normal,Dificil,Pesadelo,Inferno,Abissal};
enum class Mode:uint8_t {Classic,Expanded};
inline void require(bool condition,const char* message){if(!condition)throw std::invalid_argument(message);}
inline uint64_t checkedAdd(uint64_t a,uint64_t b){if(b>UINT64_MAX-a)throw std::overflow_error("progression addition overflow");return a+b;}
inline uint64_t checkedMul(uint64_t a,uint64_t b){if(b&&a>UINT64_MAX/b)throw std::overflow_error("progression multiplication overflow");return a*b;}
inline uint32_t checkedU32(uint64_t value){if(value>UINT32_MAX)throw std::overflow_error("progression uint32 overflow");return static_cast<uint32_t>(value);}
inline void validateLevel(uint32_t level){require(level>=1&&level<=MaxLevel,"level outside 1..999");}
inline Rank rankForLevel(uint32_t level){
 validateLevel(level);
 if(level<=20)return Rank::Aprendiz;
 if(level<=40)return Rank::Aventureiro;
 if(level<=60)return Rank::Guerreiro;
 if(level<=80)return Rank::Cavaleiro;
 if(level<=99)return Rank::Campeao;
 return static_cast<Rank>(5+(level-100)/100);
}
inline const char* rankName(Rank rank){
 static constexpr const char* names[]={"Aprendiz","Aventureiro","Guerreiro","Cavaleiro",u8"Campeão","Mestre",u8"Grão-Mestre","Lenda",u8"Mítico","Semideus","Divino","Transcendente","Celestial","Supremo"};
 require(static_cast<unsigned>(rank)<14,"invalid rank");return names[static_cast<unsigned>(rank)];
}
// Exact extension of the legacy host polynomial; the original engine still caps at 99.
inline uint64_t xpForNextLevel(uint32_t level){validateLevel(level);if(level==MaxLevel)return 0;const uint64_t n=level;const uint64_t adjusted=n+2+n*n/10;return adjusted*adjusted+20;}
inline const std::array<uint64_t,MaxLevel+1>& xpTable(){
 static const auto table=[](){std::array<uint64_t,MaxLevel+1> result{};for(uint32_t n=2;n<=MaxLevel;++n)result[n]=checkedAdd(result[n-1],xpForNextLevel(n-1));return result;}();return table;
}
inline uint64_t xpToReachLevel(uint32_t level){validateLevel(level);return xpTable()[level];}
struct Progress {uint32_t level;Rank rank;uint64_t totalXp,currentXp,requiredXp;};
inline Progress query(uint64_t totalXp){
 const auto& table=xpTable();require(totalXp<=table[MaxLevel],"XP exceeds level cap");
 uint32_t low=1,high=MaxLevel;while(low<high){const uint32_t middle=(low+high+1)/2;if(table[middle]<=totalXp)low=middle;else high=middle-1;}
 return {low,rankForLevel(low),totalXp,totalXp-table[low],xpForNextLevel(low)};
}
struct Award {Progress progress;uint64_t accepted,discarded;};
inline Award awardXp(uint64_t totalXp,uint64_t amount){query(totalXp);const auto remaining=xpToReachLevel(MaxLevel)-totalXp;const auto accepted=amount<remaining?amount:remaining;return {query(totalXp+accepted),accepted,amount-accepted};}
// Floor(value*basisPoints/10000), without requiring the product to fit uint64.
inline uint64_t scale(uint64_t value,uint32_t basisPoints){return checkedAdd(checkedMul(value/Basis,basisPoints),(value%Basis)*basisPoints/Basis);}
struct StatCurve {uint32_t base=0,linear=0,quadratic=0,divisor=1;};
inline uint32_t statAtLevel(uint32_t level,const StatCurve& curve){
 validateLevel(level);require(curve.divisor!=0,"zero stat divisor");const uint64_t n=level-1;
 return checkedU32(checkedAdd(checkedAdd(curve.base,checkedMul(curve.linear,n)),checkedMul(curve.quadratic,n*n)/curve.divisor));
}
struct Multipliers {uint32_t hp=Basis,attack=Basis,xp=Basis,gold=Basis;};
struct Rules {
 std::array<Multipliers,5> profiles{{{10000,10000,10000,10000},{15000,15000,13000,13000},{25000,25000,18000,18000},{40000,40000,25000,25000},{60000,60000,40000,40000}}};
 // Linear growth per cycle, configurable; no campaign reset is performed here.
 Multipliers ngGrowth{2500,2500,1000,1000};
};
inline void validateRules(const Rules& rules){
 const auto& normal=rules.profiles[0];require(normal.hp==Basis&&normal.attack==Basis&&normal.xp==Basis&&normal.gold==Basis,"Normal must remain 1x");
 for(const auto& m:rules.profiles)require(m.hp&&m.attack&&m.xp&&m.gold,"zero difficulty multiplier");
}
inline Multipliers multipliers(Difficulty difficulty,uint32_t ngCycle,const Rules& rules=Rules{}){
 validateRules(rules);const auto index=static_cast<unsigned>(difficulty);require(index<5,"invalid difficulty");const auto& m=rules.profiles[index];
 const auto factor=[&](uint32_t base,uint32_t growth){return checkedU32(scale(checkedAdd(Basis,checkedMul(ngCycle,growth)),base));};
 return {factor(m.hp,rules.ngGrowth.hp),factor(m.attack,rules.ngGrowth.attack),factor(m.xp,rules.ngGrowth.xp),factor(m.gold,rules.ngGrowth.gold)};
}
// These are explicit checked conversions, never masks, casts or ARM writes.
inline uint16_t checkedArmU16(uint32_t value){require(value<=UINT16_MAX,"native uint16 truncation");return static_cast<uint16_t>(value);}
inline int16_t checkedArmStat(uint32_t value){require(value<=INT16_MAX,"native signed stat truncation");return static_cast<int16_t>(value);}
inline uint32_t checkedArmXp(uint64_t value){return checkedU32(value);}
inline bool nativeLevelCompatible(uint32_t level){return level>=1&&level<=NativeMaxLevel;}
inline uint16_t checkedArmLevel(uint32_t level){require(nativeLevelCompatible(level),"native engine still capped at 99");return checkedArmU16(level);}
// Explicit snapshot import only. At native cap 99 the host exposes no current XP.
inline uint64_t importNative(uint32_t level,uint64_t currentXp){require(nativeLevelCompatible(level),"unsupported native level");require(level==NativeMaxLevel?currentXp==0:currentXp<xpForNextLevel(level),"invalid native current XP");return xpToReachLevel(level)+currentXp;}
struct Character {uint64_t totalXp=0;uint32_t ngCycle=0;Difficulty difficulty=Difficulty::Normal;};
struct Ledger {Mode mode=Mode::Expanded;Rules rules;std::map<uint32_t,Character> characters;};
inline const char* modeName(Mode mode){require(mode==Mode::Classic||mode==Mode::Expanded,"invalid mode");return mode==Mode::Classic?"classic":"expanded";}
inline void validate(const Ledger& ledger){modeName(ledger.mode);validateRules(ledger.rules);require(ledger.characters.size()<=10,"too many characters");for(const auto& pair:ledger.characters){require(pair.first<10,"invalid character slot");query(pair.second.totalXp);multipliers(pair.second.difficulty,pair.second.ngCycle,ledger.rules);}}
inline std::string encode(const Ledger& ledger){
 validate(ledger);std::ostringstream out;out.imbue(std::locale::classic());out<<"DESTINIA_PROGRESSION_V1\nmode "<<modeName(ledger.mode)<<"\ncurve legacy-extended-v1\n";
 const auto write=[&](const Multipliers& m){out<<m.hp<<' '<<m.attack<<' '<<m.xp<<' '<<m.gold<<'\n';};
 out<<"ng ";write(ledger.rules.ngGrowth);for(unsigned i=0;i<5;++i){out<<"profile "<<i<<' ';write(ledger.rules.profiles[i]);}
 out<<"characters "<<ledger.characters.size()<<'\n';for(const auto& pair:ledger.characters)out<<"character "<<pair.first<<' '<<pair.second.totalXp<<' '<<pair.second.ngCycle<<' '<<static_cast<unsigned>(pair.second.difficulty)<<'\n';out<<"end\n";return out.str();
}
// Strict bounded decimal parser; decode builds a new ledger and cannot mutate its caller.
inline Ledger decode(const std::string& text,Mode expectedMode){
 require(text.size()<=4096,"sidecar too large");for(unsigned char c:text)require((c>=32&&c<=126)||c=='\t'||c=='\r'||c=='\n',"invalid sidecar character");
 std::istringstream in(text);in.imbue(std::locale::classic());const auto token=[&](){std::string value;require(bool(in>>value),"truncated sidecar");return value;};
 const auto expect=[&](const std::string& value){require(token()==value,"invalid sidecar schema");};
 const auto number=[&](){const auto value=token();require(!value.empty()&&(value.size()==1||value[0]!='0'),"noncanonical decimal");uint64_t n=0;for(char c:value){require(c>='0'&&c<='9',"invalid decimal");n=checkedAdd(checkedMul(n,10),c-'0');}return n;};
 const auto u32=[&](){return checkedU32(number());};const auto read=[&](){return Multipliers{u32(),u32(),u32(),u32()};};
 Ledger ledger;ledger.mode=expectedMode;expect("DESTINIA_PROGRESSION_V1");expect("mode");expect(modeName(expectedMode));expect("curve");expect("legacy-extended-v1");expect("ng");ledger.rules.ngGrowth=read();
 for(unsigned i=0;i<5;++i){expect("profile");require(u32()==i,"profile order");ledger.rules.profiles[i]=read();}
 expect("characters");const auto count=u32();require(count<=10,"too many characters");for(unsigned i=0;i<count;++i){expect("character");const auto slot=u32();const auto total=number();const auto cycle=u32();const auto difficulty=u32();require(difficulty<5,"invalid difficulty");require(ledger.characters.emplace(slot,Character{total,cycle,static_cast<Difficulty>(difficulty)}).second,"duplicate character");}
 expect("end");std::string extra;require(!(in>>extra),"trailing sidecar data");validate(ledger);return ledger;
}
}} // namespace destinia::progression
