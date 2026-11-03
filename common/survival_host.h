#pragma once
#include "survival_store.h"
#include "survival_state.h"
#include "refinement_host.h"
#include "titles.h"
#include "overlay_font.h"
#include <unicorn/unicorn.h>
#include <functional>
#include <map>

namespace destinia {
// APK 1.0.6 adapter. All calls into ARM and disk writes occur outside hooks.
class SurvivalHost {
 using U=uint32_t;
 uc_engine* engine=nullptr;U base=0,game=0;fs::path directory;
 std::map<std::string,U> symbols;
 std::function<bool(U,const std::vector<U>&)> invoke;
 TitlesStore* titles=nullptr;RefinementHost* refinement=nullptr;Codex* arenaCodex=nullptr;
 std::optional<std::string> snapshot;
 std::map<U,uint64_t> instances;
 unsigned originalArea=0,originalMap=0,originalX=0,originalY=0,arenaMap=0,owner=10,pendingTitleOwner=10,pendingRewardOwner=10;
 uint64_t runSequence=0,identity=0;unsigned lastSecond=0;bool dirty=false,ownedMap=false;
 U random=0x13579bdf;
 template<class T>T read(U at)const{T v{};if(uc_mem_read(engine,at,&v,sizeof v)!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: leitura ARM invalida");return v;}
 template<class T>void write(U at,T v){if(uc_mem_write(engine,at,&v,sizeof v)!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: escrita ARM invalida");}
 U reg(int id){U v=0;if(uc_reg_read(engine,id,&v)!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: registro invalido");return v;}
 void set(int id,U n){if(uc_reg_write(engine,id,&n)!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: registro invalido");}
 U call(const char* name,std::vector<U> args={}){if(!invoke(symbols.at(name),args))throw std::runtime_error(std::string("Sobrevivencia: rotina falhou ")+name);return reg(UC_ARM_REG_R0);}
 unsigned slot()const{return read<uint16_t>(game+0x4886);}
 unsigned map()const{return unsigned(read<uint16_t>(game+0x4870))*65536u+read<uint16_t>(game+0x4872);}
 bool alive()const{return read<int16_t>(game+0x48f4)>0;}
 bool campaignMonster(U p)const{return p>=game+0x14fc&&p<game+0x14fc+20*136&&(p-game-0x14fc)%136==0&&((p-game-0x14fc)/136)<read<U>(game+0x1f9c)&&read<U>(game+0x1f9c)<=20&&read<uint16_t>(p+8)<=109&&read<uint16_t>(p+0xa)<200;}
 static void hook(uc_engine*,uint64_t pc,uint32_t,void* data){auto& s=*static_cast<SurvivalHost*>(data);try{s.at(U(pc));}catch(const std::exception&e){s.error=e.what();uc_emu_stop(s.engine);}}
 void at(U pc){
  const auto s=slot();if(s>=10)return;
  if(pc==base+0x133b2){
   if(!campaignMonster(reg(UC_ARM_REG_R6)))return;
   combat.master=titles&&titles->isUnlocked(s,TitleId::MestreDaSobrevivencia)&&titles->getEquipped(s)==TitleId::MestreDaSobrevivencia;
   if(!combat.enabled&&!combat.master&&!combat.elixirMs&&!combat.immuneMs)return;
   random^=random<<13;random^=random>>17;random^=random<<5;
   set(UC_ARM_REG_R8,combat.hit(read<uint16_t>(game+0x48f4),read<uint16_t>(game+0x48c4),reg(UC_ARM_REG_R8),random%10000));return;
  }
  if(pc==(symbols.at("checkQuestMonster")&~1u)&&arena.active){const U monster=reg(UC_ARM_REG_R0);if(instances.count(monster)){set(UC_ARM_REG_R0,0);set(UC_ARM_REG_PC,reg(UC_ARM_REG_LR));return;}}
  if(pc==base+0x25392&&titles){
   const auto id=titles->getEquipped(s);if(!titles->isUnlocked(s,id))return;
   if(id==TitleId::MestreDaSobrevivencia){
    for(U index=0;index<13;++index){if(index==4)continue;const U at=game+0x48ba+index*2;const auto value=read<uint16_t>(at);const auto scaled=uint16_t(std::min<uint64_t>(32767,uint64_t(value)*220/100));write<uint16_t>(at,scaled);if(index<4){const U delta=game+0x48d4+index*2;write<uint16_t>(delta,uint16_t(read<uint16_t>(delta)+scaled-value));}}
   }else if(titles->isExclusive(s)){
    const auto* def=findTitle(id);if(!def)return;
    const int percentages[]={def->hpBonusPct,def->mpBonusPct,def->atkBonusPct,def->defBonusPct};
    for(U i=0;i<4;++i){const U at=game+0x48c4+i*2;const auto value=read<uint16_t>(at);const auto scaled=uint16_t(std::min<uint64_t>(32767,uint64_t(value)*(100+percentages[i])/100));write<uint16_t>(at,scaled);}
    if(def->critChanceTenths){const U at=game+0x48d0;write<uint16_t>(at,uint16_t(std::min<unsigned>(1000,unsigned(read<uint16_t>(at))+unsigned(def->critChanceTenths))));}
   }
   if(titles->isExclusive(s)&&id!=TitleId::Nenhum){
    const auto hp=read<uint16_t>(game+0x48c4),mp=read<uint16_t>(game+0x48c6);
    write<uint16_t>(game+0x48f8,hp);write<uint16_t>(game+0x48f6,mp);
    if(read<uint16_t>(game+0x48f4)>hp)write<uint16_t>(game+0x48f4,hp);
    if(read<uint16_t>(game+0x48f2)>mp)write<uint16_t>(game+0x48f2,mp);
   }
  }
 }
 std::vector<std::pair<int,int>> positions(unsigned count){
  U m=symbols.at("curMapData");int w=read<uint16_t>(m),h=read<uint16_t>(m+2);U grid=read<U>(m+0x68);
  if(w<4||h<4||w>256||h>256)throw std::runtime_error("Sobrevivencia: grade invalida");
  const int hx=read<int16_t>(game+0x4890),hy=read<int16_t>(game+0x4892);std::vector<std::pair<int,int>> out;
  for(int y=1;y<h-1;++y)for(int x=1;x<w-1;++x){int px=x*16+8,py=y*16+8;
   if(read<uint8_t>(grid+y*w+x)||read<uint8_t>(grid+y*w+x-1)||read<uint8_t>(grid+y*w+x+1)||read<uint8_t>(grid+(y-1)*w+x)||read<uint8_t>(grid+(y+1)*w+x)||(px-hx)*(px-hx)+(py-hy)*(py-hy)<1024)continue;
   bool overlaps=false;for(auto p:out)if((p.first-px)*(p.first-px)+(p.second-py)*(p.second-py)<400)overlaps=true;
   if(overlaps)continue;out.emplace_back(px,py);if(out.size()==count)return out;
  }throw std::runtime_error("Sobrevivencia: sem espaco seguro");
 }
 void spawn(){
  const auto plan=survival::wave(arena.currentWave);instances.clear();
  call("Map_freeData");call("Map_freeTile");call("Map_loadData",{0,1,1});call("Map_loadTile",{0});arenaMap=map();
  for(U i=0;i<=20;++i)call("Monster_freeMonster",{i});call("Monster_freeBoss");
  const auto points=positions(plan.count);std::array<uint8_t,136*20> blank{};
  if(uc_mem_write(engine,game+0x14fc,blank.data(),blank.size())!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: pool invalido");
  write<U>(game+0x1f9c,plan.count);
  for(U i=0;i<plan.count;++i){U p=game+0x14fc+136*i;unsigned species=i==0&&plan.bossSpecies?plan.bossSpecies:1;
   write<uint16_t>(p+8,uint16_t(species));write<uint16_t>(p+0xa,uint16_t(species>=100?species:0));
   write<uint16_t>(p+0x10,uint16_t(std::max(1u,std::min(99u,unsigned(read<uint16_t>(game+0x48fa))+plan.number-1))));
   write<uint16_t>(p+0x16,uint16_t(points[i].first));write<uint16_t>(p+0x18,uint16_t(points[i].second));
  }
  call("Mon_createMonster");
   for(U i=0;i<plan.count;++i){U p=game+0x14fc+136*i;auto maximum=read<int32_t>(p+0x24);if(maximum<=0)throw std::runtime_error("Sobrevivencia: spawn incompleto");write<uint32_t>(p+0x4c,plan.elite?1u:0u);
   auto hp=std::min<uint64_t>(INT32_MAX/8,uint64_t(maximum)*plan.hpBps/10000);
   auto attack=std::min<uint64_t>(32767,uint64_t(read<uint16_t>(p+0x28))*plan.attackBps/10000);
   write<int32_t>(p+0x24,int32_t(hp));write<int32_t>(p+0x20,int32_t(hp));write<uint16_t>(p+0x28,uint16_t(attack));

   const unsigned species=i==0&&plan.bossSpecies?plan.bossSpecies:1;
   const auto id=++identity;if(!arena.registerMonster(id,species))throw std::runtime_error("Sobrevivencia: registro rejeitado");instances.emplace(p,id);
  }
  dirty=true;message="ONDA "+std::to_string(plan.number);
 }
public:
 survival::Arena arena;survival::CombatState combat;std::string error,message;
 std::function<void(unsigned,uint64_t)> onFungusKilled;
 void bindCodex(Codex* value){arenaCodex=value;}
 bool rewardPending()const{return pendingRewardOwner<10;}
 bool ready=false;
 // Slot deletion must not discard a live arena or progress not yet persisted.
 bool canDeleteCharacter() const {
  return !arena.active&&!dirty&&pendingRewardOwner>=10&&(!ready||survival::readText(directory)==snapshot);
 }
 void reloadAfterCharacterDeletion() {
  if(!ready)return;
  const auto fresh=survival::readText(directory);
  auto ledger=fresh?survival::decode(*fresh):survival::Ledger{};
  arena.ledger=std::move(ledger);snapshot=fresh;
 }
 void install(uc_engine* e,U soBase,const std::map<std::string,U>& exports,const std::string& library,const fs::path& dir,TitlesStore& titleStore,RefinementHost& refiner,std::function<bool(U,const std::vector<U>&)> caller){
  std::ifstream in(library,std::ios::binary);uint64_t hash=14695981039346656037ull;char b;while(in.get(b))hash=(hash^uint8_t(b))*1099511628211ull;
  if(!in.eof()||hash!=0x74b2547839b88777ull||(exports.at("Mon_initMonster")&~1u)!=soBase+0x1740c||(exports.at("GameUI_calcHeroStat")&~1u)!=soBase+0x24fb0)throw std::runtime_error("Sobrevivencia: ABI incompativel");
  engine=e;base=soBase;symbols=exports;game=exports.at("Game");directory=dir;titles=&titleStore;refinement=&refiner;invoke=std::move(caller);
  snapshot=survival::readText(dir);if(snapshot)arena.ledger=survival::decode(*snapshot);titles->load(dir);
  for(U offset:{0x25392u,0x133b2u,(symbols.at("checkQuestMonster")&~1u)-base}){uc_hook h;auto at=base+offset;if(uc_hook_add(engine,&h,UC_HOOK_CODE,reinterpret_cast<void*>(hook),this,at,at)!=UC_ERR_OK)throw std::runtime_error("Sobrevivencia: hook indisponivel");}
  ready=true;
 }
 bool start(){
  if(!ready||arena.active||!error.empty())return false;
  if(pendingRewardOwner<10){message="RECOMPENSA PENDENTE";return false;}
  const auto s=slot();
  if(s>=10||!alive()||read<int16_t>(game+0x4854)!=0){message="ENTRE NO JOGO PRIMEIRO";return false;}
  originalArea=read<uint16_t>(game+0x4870);originalMap=read<uint16_t>(game+0x4872);originalX=read<uint16_t>(game+0x4890);originalY=read<uint16_t>(game+0x4892);owner=s;ownedMap=true;
  try{if(!arena.start(s,++runSequence,1))return false;spawn();flush();return true;}catch(const std::exception&e){message=e.what();stop(true);return false;}
 }
 void clearArenaMonsters(){if(map()!=arenaMap)return;for(U i=0;i<=20;++i)call("Monster_freeMonster",{i});call("Monster_freeBoss");instances.clear();}
 void stop(bool restore){
  arena.abort();if(ownedMap)clearArenaMonsters();instances.clear();dirty=true;
  if(restore&&ownedMap&&slot()==owner&&map()==arenaMap){call("Map_freeData");call("Map_freeTile");call("Map_loadData",{originalArea,originalMap,1});call("Map_loadTile",{0});write<uint16_t>(game+0x4890,uint16_t(originalX));write<uint16_t>(game+0x4892,uint16_t(originalY));}
  ownedMap=false;
 }
 void tick(unsigned ms){
  if(!ready)return;
  try{
   const auto s=slot();
   // An earned title remains tied to the character that earned it even if
   // the arena was exited while its transaction was blocked.
   if(pendingRewardOwner<10&&!arena.active){if(s!=pendingRewardOwner||!credit())return;}
   if(s!=owner&&!arena.active){owner=s;combat.reset();}
   if(arena.active&&(s!=owner||map()!=arenaMap||!alive())){stop(s==owner&&map()==arenaMap);flush();return;}
   if(read<int16_t>(game+0x4854)!=0)return;
   combat.tick(ms);const auto oldBestTime=arena.slot>=0?arena.ledger.slots[arena.slot].bestTimeSeconds:0;arena.tick(ms);if(arena.slot>=0&&arena.ledger.slots[arena.slot].bestTimeSeconds!=oldBestTime)dirty=true;

   if(arena.active){
    if(s!=owner||map()!=arenaMap||!alive()){stop(false);flush();return;}

    for(auto entry:instances)if(read<int32_t>(entry.first+0x20)<=0&&arena.observeKill(arena.runId,entry.second,arenaMap,int(s))){
     dirty=true;if(onFungusKilled&&read<uint16_t>(entry.first+8)==1)onFungusKilled(s,entry.second);
    }
    if(arena.clearedWave()){
     pendingRewardOwner=owner;
     if(arena.currentWave>=20&&titles&&!titles->isUnlocked(int(owner),TitleId::MestreDaSobrevivencia))pendingTitleOwner=owner;
     if(!credit())return;if(arena.beginNextWave()){spawn();}else stop(true);
    }

   }flush();
  }catch(const std::exception&e){error=e.what();message=error;try{stop(true);}catch(...){}ready=false;}
 }
 bool credit(){
  if(!refinement||owner>=10||slot()!=owner)return false;
  auto nextArena=arena;const auto reward=nextArena.takePending(int(owner));
  if(!reward.gold&&!reward.cash&&!reward.crystals&&!reward.pure&&!reward.abyssal&&pendingTitleOwner>=10){pendingRewardOwner=10;return true;}
  // Only an actually completed wave in this run can earn the title. Old
  // ledgers may belong to a deleted character that once occupied this slot.
  const bool grant=pendingTitleOwner==owner&&titles&&!titles->isUnlocked(int(owner),TitleId::MestreDaSobrevivencia);
  FileLock titleLock;
  if(grant&&!titleLock.acquire(directory/"titles-v1.lock")){message="TITULOS OCUPADOS";return false;}
  auto nextTitles=grant?*titles:TitlesStore{};
  if(grant){if(!nextTitles.unchanged(directory)){message="TITULOS ALTERADOS";return false;}nextTitles.unlock(int(owner),TitleId::MestreDaSobrevivencia);}
  FileLock survivalLock;
  if(!survivalLock.acquire(directory/"survival-v1.lock")){message="SOBREVIVENCIA OCUPADA";return false;}
  const bool saveCodex=arenaCodex&&arenaCodex->dirty;FileLock codexLock;std::string codexText;
  if(saveCodex){
   if(!codexLock.acquire(directory/"codex-v1.lock")){message="CODEX OCUPADO";return false;}
   if(!arenaCodex->matchesBaseline(directory/"codex-v1.txt")){message="CODEX ALTERADO";return false;}
   codexText=arenaCodex->encode();
  }
  auto next=refinement->store;
  if(!next.addCrystals(owner,reward.crystals,0)||!next.addCrystals(owner,reward.pure,1)||!next.addCrystals(owner,reward.abyssal,2))return false;
  const U gold=call("getMyGold"),cash=call("getMyCash");
  if(gold>2147483647u-reward.gold||cash>2147483647u-reward.cash)return false;
  const std::string text=survival::encode(nextArena.ledger);
  call("setMyGold",{gold+reward.gold});call("setMyCash",{cash+reward.cash});
  // Save the original campaign location, never the temporary arena location.
  const std::array<U,4> offsets{0x4870,0x4872,0x4890,0x4892};std::array<uint16_t,4> live{};
  for(size_t i=0;i<4;++i)live[i]=read<uint16_t>(game+offsets[i]);
  if(ownedMap){const std::array<U,4> original{originalArea,originalMap,originalX,originalY};for(size_t i=0;i<4;++i)write<uint16_t>(game+offsets[i],uint16_t(original[i]));}
  const bool okay=refinement->commit(next,[&](SaveTransaction& tx){
   if(grant){
    if(!titles->unchanged(directory))throw std::runtime_error("Sobrevivencia: titulos mudaram externamente");
    const auto titleText=nextTitles.encode();FILE* titleOut=tx.open(directory/"titles-v1.txt","wb");
    if(!titleOut)throw std::runtime_error("Sobrevivencia: falha preparando titulo");
    const bool titleWrote=std::fwrite(titleText.data(),1,titleText.size(),titleOut)==titleText.size();const int titleClosed=tx.close(titleOut);
    if(!titleWrote||titleClosed)throw std::runtime_error("Sobrevivencia: titulo incompleto");
   }
   if(saveCodex){
    if(!arenaCodex->matchesBaseline(directory/"codex-v1.txt")||arenaCodex->encode()!=codexText)throw std::runtime_error("Sobrevivencia: codex mudou externamente");
    FILE* out=tx.open(directory/"codex-v1.txt","wb");if(!out)throw std::runtime_error("Sobrevivencia: falha preparando codex");
    const bool wrote=std::fwrite(codexText.data(),1,codexText.size(),out)==codexText.size();const int closed=tx.close(out);
    if(!wrote||closed)throw std::runtime_error("Sobrevivencia: codex incompleto");
   }
   if(survival::readText(directory)!=snapshot)throw std::runtime_error("Sobrevivencia: sidecar mudou externamente");
   FILE* out=tx.open(directory/survival::SidecarName,"wb");if(!out)throw std::runtime_error("Sobrevivencia: falha preparando premio");
   const bool wrote=std::fwrite(text.data(),1,text.size(),out)==text.size();const int closed=tx.close(out);if(!wrote||closed)throw std::runtime_error("Sobrevivencia: premio incompleto");
  });
  for(size_t i=0;i<4;++i)write<uint16_t>(game+offsets[i],live[i]);
  if(!okay){call("setMyGold",{gold});call("setMyCash",{cash});message=refinement->message;return false;}
  arena.ledger=nextArena.ledger;snapshot=text;dirty=false;
  if(grant){if(!nextTitles.load(directory))throw std::runtime_error("Sobrevivencia: titulo salvo mas recarga falhou");*titles=std::move(nextTitles);}
  if(saveCodex)arenaCodex->adoptPersisted(directory/"codex-v1.txt");
  pendingTitleOwner=10;pendingRewardOwner=10;return true;
 }
 void flush(){if(!dirty||!ready||pendingRewardOwner<10)return;survival::save(directory,arena.ledger,AtomicSaveOps{},snapshot);snapshot=survival::encode(arena.ledger);dirty=false;}
 void paint(uint8_t* pixels,int w,int h)const{
  if(!ready||!pixels)return;auto s=slot();if(s>=10)return;
  if(arena.active){paintOverlayText(pixels,w,h,5,36,"ONDA "+std::to_string(arena.currentWave)+"  INIMIGOS RESTANTES "+std::to_string(arena.remaining),0xffc0);}
 }
};
}
