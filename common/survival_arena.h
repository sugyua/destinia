#pragma once
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <cstdint>
namespace destinia { namespace survival {
inline constexpr const char* SidecarName="survival-v1.txt";
inline constexpr unsigned MaxValue=1000000000;
inline constexpr uint64_t MaxIdentity=9007199254740991ULL;
struct Wave {unsigned number,count,bossSpecies,hpBps,attackBps,gold,crystals;bool elite;};
inline Wave wave(unsigned n){
 if(n<1||n>MaxValue)throw std::invalid_argument("invalid wave");
 return {n,std::min(12u,3+n),n%5==0?100+((n/5-1)%10):0,
 unsigned(std::min<uint64_t>(1000000,10000+uint64_t(n-1)*1500)),
 unsigned(std::min<uint64_t>(1000000,10000+uint64_t(n-1)*1000)),
 unsigned(std::min<uint64_t>(MaxValue,uint64_t(500)*n)),1+n/5,n%3==0};
}
struct SlotProgress {unsigned bestWave=0,completions=0,totalKills=0,bestTimeSeconds=0,survivalScore=0,pendingGold=0,pendingCrystals=0;bool titleUnlocked=false;unsigned pendingCash=0,pendingPure=0,pendingAbyssal=0;};
struct Ledger {std::array<SlotProgress,10> slots{};};
struct Reward {unsigned gold=0,crystals=0,cash=0,pure=0,abyssal=0;};
inline unsigned add(unsigned a,uint64_t b){return unsigned(std::min<uint64_t>(MaxValue,uint64_t(a)+b));}
inline bool valid(const SlotProgress& s){return s.bestWave<=MaxValue&&s.completions<=MaxValue&&s.totalKills<=MaxValue&&s.bestTimeSeconds<=MaxValue&&s.survivalScore<=MaxValue&&s.pendingGold<=MaxValue&&s.pendingCrystals<=MaxValue&&s.pendingCash<=MaxValue&&s.pendingPure<=MaxValue&&s.pendingAbyssal<=MaxValue&&s.titleUnlocked==(s.bestWave>=20);}
inline std::string encode(const Ledger& l){std::string out="DESTINIA_SURVIVAL_V1\n";for(unsigned i=0;i<10;++i){const auto&s=l.slots[i];if(!valid(s))throw std::invalid_argument("invalid survival state");out+="slot "+std::to_string(i)+" "+std::to_string(s.bestWave)+" "+std::to_string(s.completions)+" "+std::to_string(s.totalKills)+" "+std::to_string(s.bestTimeSeconds)+" "+std::to_string(s.survivalScore)+" "+std::to_string(s.pendingGold)+" "+std::to_string(s.pendingCrystals)+" "+(s.titleUnlocked?"1":"0")+" "+std::to_string(s.pendingCash)+" "+std::to_string(s.pendingPure)+" "+std::to_string(s.pendingAbyssal)+"\n";}return out+"end\n";}
inline Ledger decode(const std::string& text){if(text.size()>4096)throw std::invalid_argument("oversized survival");std::istringstream in(text);std::string tag;Ledger l;if(!(in>>tag)||tag!="DESTINIA_SURVIVAL_V1")throw std::invalid_argument("invalid survival header");for(unsigned i=0;i<10;++i){unsigned index,title;auto&s=l.slots[i];if(!(in>>tag>>index>>s.bestWave>>s.completions>>s.totalKills>>s.bestTimeSeconds>>s.survivalScore>>s.pendingGold>>s.pendingCrystals>>title>>s.pendingCash>>s.pendingPure>>s.pendingAbyssal)||tag!="slot"||index!=i||title>1)throw std::invalid_argument("invalid survival slot");s.titleUnlocked=title!=0;}if(!(in>>tag)||tag!="end"||encode(l)!=text)throw std::invalid_argument("invalid survival text");return l;}
class Arena {
 struct Member {unsigned species;bool dead=false;};
 std::map<uint64_t,Member> members;unsigned arenaMap=0;uint64_t lastRun=0,highestInstance=0;bool bossRegistered=false,cleared=false;
public:
 Ledger ledger{};int slot=-1;uint64_t runId=0,runElapsedMs=0;unsigned currentWave=0,remaining=0;bool active=false;
 bool start(int s,uint64_t run,unsigned mapId=0){if(s<0||s>=10||run==0||run>MaxIdentity||run<=lastRun)return false;abort();slot=s;runId=lastRun=run;highestInstance=0;arenaMap=mapId;runElapsedMs=0;active=true;currentWave=1;remaining=wave(1).count;return true;}
 void tick(unsigned ms,bool paused=false){if(!active||paused||slot<0)return;auto&s=ledger.slots[slot];runElapsedMs=std::min<uint64_t>(runElapsedMs+ms,uint64_t(MaxValue)*1000);const auto seconds=unsigned(runElapsedMs/1000);if(seconds>s.bestTimeSeconds){s.bestTimeSeconds=seconds;const uint64_t score=uint64_t(s.totalKills)*100+uint64_t(s.bestWave)*500+uint64_t(s.completions)*5000+uint64_t(seconds)*10;s.survivalScore=std::max(s.survivalScore,unsigned(std::min<uint64_t>(MaxValue,score)));}}
 bool registerMonster(uint64_t id,unsigned species){if(!active||cleared||id==0||id>MaxIdentity||id<=highestInstance||members.size()>=wave(currentWave).count)return false;auto w=wave(currentWave);if(species>=100){if(!w.bossSpecies||species!=w.bossSpecies||bossRegistered)return false;bossRegistered=true;}else if(w.bossSpecies&&!bossRegistered&&members.size()+1==w.count)return false;members.emplace(id,Member{species,false});highestInstance=id;return true;}
 bool observeKill(uint64_t run,uint64_t id,unsigned mapId=0,int observedSlot=-1){if(!active||cleared||run!=runId||mapId!=arenaMap||(observedSlot>=0&&observedSlot!=slot))return false;auto it=members.find(id);if(it==members.end()||it->second.dead)return false;it->second.dead=true;--remaining;auto&s=ledger.slots[slot];s.totalKills=add(s.totalKills,1);s.survivalScore=add(s.survivalScore,100);if(!remaining){auto w=wave(currentWave);s.bestWave=std::max(s.bestWave,currentWave);s.pendingGold=add(s.pendingGold,w.gold);s.pendingCash=add(s.pendingCash,currentWave%5==0?currentWave:1);if(currentWave<=10)s.pendingCrystals=add(s.pendingCrystals,w.crystals);else if(currentWave<=25)s.pendingPure=add(s.pendingPure,w.crystals);else s.pendingAbyssal=add(s.pendingAbyssal,w.crystals);s.titleUnlocked=s.bestWave>=20;s.survivalScore=add(s.survivalScore,500);cleared=true;if(currentWave%50==0){s.completions=add(s.completions,1);s.survivalScore=add(s.survivalScore,5000);}}return true;}
 bool beginNextWave(){if(!active||!cleared||currentWave>=MaxValue)return false;++currentWave;remaining=wave(currentWave).count;members.clear();bossRegistered=cleared=false;return true;}
 bool clearedWave()const{return cleared;}
 void abort(){active=false;slot=-1;runId=0;currentWave=remaining=0;runElapsedMs=0;members.clear();bossRegistered=cleared=false;}
 Reward takePending(int s){if(s<0||s>=10)return {};auto&p=ledger.slots[s];Reward r{p.pendingGold,p.pendingCrystals,p.pendingCash,p.pendingPure,p.pendingAbyssal};p.pendingGold=p.pendingCrystals=p.pendingCash=p.pendingPure=p.pendingAbyssal=0;return r;}
};
}}
