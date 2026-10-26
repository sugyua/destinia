#pragma once
#include "codex_portraits.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>

namespace destinia {
inline bool isCodexBoss(int species) { return species >= 100; }
enum class CodexCategoryFilter : int { All = 0, Common = 1, Bosses = 2 };
struct CodexMonsterFacts {int maxHP=0,level=0;bool elite=false;std::set<std::string> locations;};
struct CodexInfoLine {std::string text;uint16_t color=0xffff;};
struct CodexInfoGroup {std::string id,title;std::vector<CodexInfoLine> rows;};
struct CodexInfoSection {std::string id,title;std::vector<CodexInfoGroup> groups;};
struct CodexInfoView {std::string hp,notes;std::vector<CodexInfoSection> sections;};

// Read-only original-asset metadata. Load into a temporary instance so malformed
// resources cannot replace an already usable catalogue. No saves or ARM memory.
class CodexMetadata {
 struct Reader {
  const std::vector<uint8_t>& bytes;size_t at=0;
  void need(size_t n) const {if(at>bytes.size() || n>bytes.size()-at)fail();}
  void skip(size_t n){need(n);at+=n;}
  int u8(){need(1);return bytes[at++];}
  int u16(){need(2);int n=int(bytes[at])|(int(bytes[at+1])<<8);at+=2;return n;}
  void counted(size_t stride){const auto n=u8();skip(size_t(n)*stride);}
 };
 struct Monster {std::string name;int quest=-1;};
 struct Item {int id=0,type=0,tier=0;std::string name;};
 struct Map {std::string id,name;};
 std::map<int,Monster> monsters;
 std::vector<Item> items;
 std::vector<std::string> equipment;
 std::map<std::string,Map> maps;
 std::map<int,std::set<std::string>> spawns;
 std::map<int,std::set<int>> baseLevels;
 static void fail(){throw std::runtime_error("Metadados originais do Codex invalidos ou incompletos.");}
 static std::vector<uint8_t> read(const std::filesystem::path& path) {
  std::error_code ec;auto size=std::filesystem::file_size(path,ec);
  if(ec || !size || size>2*1024*1024)fail();
  std::ifstream in(path,std::ios::binary);if(!in)fail();
  std::vector<uint8_t> raw(static_cast<size_t>(size));
  in.read(reinterpret_cast<char*>(raw.data()),std::streamsize(raw.size()));if(!in)fail();
  auto bytes=codexUnpackResource(raw);if(bytes.empty())fail();return bytes;
 }
 static std::string text(const std::vector<uint8_t>& bytes,size_t at,size_t size,bool fixed=false) {
  if(at>bytes.size() || size>bytes.size()-at)fail();
  std::string out;
  for(size_t i=0;i<size;++i) {
   auto c=bytes[at+i];if(fixed && !c)break;
   if(c<32 || c>126)fail();
   out+=char(c);
  }
  return out;
 }
 static std::vector<std::string> table(const std::filesystem::path& path) {
  auto b=read(path);Reader r{b};int count=r.u16();if(count>8192)fail();
  std::vector<std::string> out;out.reserve(count);
  for(int i=0;i<count;++i){auto n=r.u16();if(n>1024)fail();r.need(n);out.push_back(text(b,r.at,n));r.skip(n);}
  if(r.at!=b.size())fail();
  return out;
 }
 static std::string mapId(int area,int index){return "m"+std::to_string(area)+"_"+std::to_string(index);}
 static bool mapFilename(const std::string& name,int& area,int& index) {
  if(name.size()<8 || name[0]!='m' || name[1]<'0' || name[1]>'2' || name[2]!='_' || name.substr(name.size()-4)!=".mpd")return false;
  const auto digits=name.substr(3,name.size()-7);if(digits.empty() || digits.size()>3)return false;
  index=0;for(char c:digits){if(c<'0' || c>'9')return false;index=index*10+c-'0';}area=name[1]-'0';return true;
 }
 void loadMaps(const std::filesystem::path& assets,const std::array<std::vector<std::string>,3>& names) {
  for(const auto& entry:std::filesystem::directory_iterator(assets)) {
   int area=0,index=0;if(!mapFilename(entry.path().filename().string(),area,index))continue;
   const auto id=mapId(area,index);if(maps.count(id))fail();
   maps[id]={id,index<int(names[area].size())?names[area][index]:"NOME NAO DISPONIVEL"};
   auto b=read(entry.path());Reader r{b};const int w=r.u8(),h=r.u8(),p=r.u8(),q=r.u8();
   if(!w || !h)fail();
   r.skip(size_t(p)+q+4*size_t(w)*h);
   r.counted(3);r.skip(2);r.counted(7);r.counted(5);r.counted(6);
   std::array<std::pair<int,int>,4> slots;
   for(auto& slot:slots){slot.first=r.u8();slot.second=r.u8();r.skip(1);}
   const int count=r.u8();std::set<int> referenced;
   for(int i=0;i<count;++i){r.skip(2);int slot=r.u8();r.skip(1);if(slot>=4)fail();referenced.insert(slot);}
   for(int slot:referenced) {
    const auto species=slots[slot].first;if(!species)continue;if(!monsters.count(species))fail();
    spawns[species].insert(id);baseLevels[species].insert(slots[slot].second);
   }
   if(r.at!=b.size())fail();
  }
  if(maps.size()!=172)fail();
 }
 void loadScripts(const std::filesystem::path& assets) {
  auto b=read(assets/"script_en.sdt");Reader header{b};const int count=header.u8();if(count!=59)fail();
  std::vector<std::pair<size_t,size_t>> blocks;
  for(int i=0;i<count;++i){auto at=header.u16(),size=header.u16();blocks.push_back({size_t(at),size_t(size)});}
  constexpr int lengths[]={5,6,1,2,2,4,5,2,2,4,3,4,3,0,3,1,1,1,1,1,2,4,3,4,3,5,2};
  size_t previous=header.at;
  for(const auto& block:blocks) {
   if(block.first!=previous || block.first>b.size() || block.second>b.size()-block.first)fail();
   Reader r{b,block.first};const auto end=block.first+block.second;std::string lastMap;
   while(r.at<end) {
    const size_t at=r.at;const int opcode=r.u8();if(opcode>=int(sizeof(lengths)/sizeof(lengths[0])))fail();
    size_t length=size_t(lengths[opcode]);if(opcode==13)length=4+size_t(r.u16());
    if(!length || length>end-at)fail();
    if(opcode==0){Reader op{b,at+1};const auto area=op.u8(),index=op.u8();auto id=mapId(area,index);lastMap=maps.count(id)?id:std::string();}
    if(opcode==25) {
     Reader op{b,at+1};const int species=op.u8(),level=op.u8();if(!monsters.count(species))fail();
     if(!lastMap.empty())spawns[species].insert(lastMap);
     baseLevels[species].insert(level);
    }
    r.at=at+length;
   }
   previous=end;
  }
  if(previous!=b.size())fail();
 }
 void loadImpl(const std::filesystem::path& assets) {
  auto mon=read(assets/"mon_data_en.gdt");if(mon.size()!=120*32)fail();
  for(int id=0;id<120;++id){auto name=text(mon,id*32+12,20,true);if(name.empty())continue;Reader r{mon,size_t(id*32+6)};int quest=r.u16();if(quest>=32768)quest-=65536;monsters[id]={name,quest};}
  if(monsters.size()!=73)fail();
  auto data=read(assets/"item_en.gdt");if(data.size()!=158*166)fail();
  for(int i=0;i<158;++i){Reader r{data,size_t(i*166)};Item item;item.id=r.u16();r.skip(2);item.type=r.u16();item.tier=r.u16();item.name=text(data,size_t(i*166+14),32,true);if(item.id!=i || item.type>255 || item.tier>255 || item.name.empty())fail();items.push_back(std::move(item));}
  equipment=table(assets/"equip_name_en.tdt");if(equipment.size()!=8*31)fail();
  std::array<std::vector<std::string>,3> names;
  for(int area=0;area<3;++area)names[area]=table(assets/("mapname"+std::to_string(area)+"_en.tdt"));
  if(names[0].size()!=66 || names[1].size()!=51 || names[2].size()!=53)fail();
  loadMaps(assets,names);loadScripts(assets);
 }
 static int maxGrade(int effective){return effective<40?2:effective<80?3:4;}
 static int equipIndex(int grade,int effective) {
  return grade==4?28+(effective-80)/10:grade==3?21+(effective-40)/10:effective/5;
 }
 static const char* rarity(int grade){constexpr const char* names[]={"COMUM","MAGICO","RARO","EPICO","LENDARIO"};return names[grade];}
 static uint16_t color(int grade){constexpr uint16_t colors[]={0xffff,0x07e0,0x041f,0xbb9f,0xfc08};return colors[grade];}
 static CodexInfoGroup& add(CodexInfoSection& section,const std::string& id,const std::string& title) {
  section.groups.push_back({id,title,{}});return section.groups.back();
 }
 void addEquipment(CodexInfoSection& drops,int level,bool equipmentOnly) const {
  // +0..2 is the original drop-level variation. For an unseen monster enumerate
  // the complete valid range, rather than presenting one map's base as observed.
  const int first=level>0?std::min(level,100):0,last=level>0?first+2:102;
  for(int recipe=0;recipe<=1;++recipe) {
   if(recipe && equipmentOnly)continue;
   for(int grade=(recipe || equipmentOnly)?2:0;grade<=4;++grade) {
    std::vector<CodexInfoLine> rows;std::set<std::string> seen;
    for(int type=0;type<(recipe?7:8);++type)for(int effective=first;effective<=last;++effective) {
     if(grade>maxGrade(effective))continue;
     const int index=equipIndex(grade,effective);
     if(index<0 || index>=31)continue;
     const auto& name=equipment[size_t(type*31+index)];
     if(seen.insert(name).second)rows.push_back({std::string(recipe?"Recipe: ":"")+name+" / "+rarity(grade),color(grade)});
    }
    if(!rows.empty()){auto& g=add(drops,std::string(recipe?"recipe-":"equipment-")+std::to_string(grade),std::string(recipe?"RECEITAS / ":"EQUIPAMENTOS / ")+rarity(grade));g.rows=std::move(rows);}
   }
  }
 }
 void addItems(CodexInfoSection& drops,int level,int quest) const {
  add(drops,"gold","OURO").rows.push_back({"Gold"});
  constexpr int types[]={0,1,2,3,5,6,7,8,9,10,11,12,13,14,15,16,17,22,23,24};
  constexpr const char* titles[]={"POCOES HP","POCOES MP","FRASCOS HP","FRASCOS MP","ORBS DE FORCA","ORBS DE DESTREZA","ORBS DE VITALIDADE","ORBS DE INTELIGENCIA","ORBS DE PRECISAO","ORBS DE EVASAO","ORBS CRITICOS","ORBS VAMPIRICOS","MOCHILAS","METAIS","COUROS","TECIDOS","PLANTAS","PEDRAS DE FORJA","PERGAMINHOS DE AVALIACAO","OVOS"};
  for(size_t category=0;category<sizeof(types)/sizeof(types[0]);++category) {
   const int type=types[category];std::vector<const Item*> selected;
   if(level>0 && type!=24) {
    const Item* choice=nullptr;const Item* fallback=nullptr;
    for(const auto& item:items)if(item.type==type){fallback=&item;if(item.tier==level/20)choice=&item;}
    if(choice || fallback)selected.push_back(choice?choice:fallback);
   }else for(const auto& item:items)if(item.type==type)selected.push_back(&item);
   if(selected.empty())continue;
   auto& g=add(drops,type==24?"eggs":"items-"+std::to_string(type),titles[category]);
   for(auto item:selected)g.rows.push_back({item->name});
  }
  if(quest>=0)add(drops,"quest","MISSAO").rows.push_back({"SO COM MISSAO ATIVA COMPATIVEL"});
 }
public:
 void load(const std::filesystem::path& assets){CodexMetadata next;next.loadImpl(assets);*this=std::move(next);}
 CodexInfoView view(int species,const CodexMonsterFacts* facts=nullptr) const {
  const int level=facts && facts->level>0?facts->level:0;const bool equipmentOnly=species>=100 || (facts && facts->elite);
  CodexInfoView out;out.hp=facts && facts->maxHP>0?std::to_string(facts->maxHP):"VARIA COM O NIVEL";
  if(facts && facts->maxHP>0 && level>0)out.hp+=" / NV "+std::to_string(level);
  out.notes=facts && facts->maxHP>0?std::string("ULTIMO ENCONTRO: ")+(species>=100?"CHEFE":facts->elite?"ELITE":"NORMAL"):"HP E DROPS VARIAM COM O NIVEL.";
  out.sections={{"drops","DROPS",{}},{"spawns","APARICOES",{}},{"variants","VARIANTES",{}}};
  auto& drops=out.sections[0];auto& rules=add(drops,"rules","REGRAS");
  rules.rows.push_back({"DROPS POSSIVEIS / NAO GARANTIDOS"});
  rules.rows.push_back({level?"NIVEL OBSERVADO "+std::to_string(level):"NIVEL NAO OBSERVADO / TODAS AS FAIXAS"});
  rules.rows.push_back({"EQUIPAMENTO: NIVEL OBSERVADO +0 A +2"});rules.rows.push_back({"PREFIXO VARIA / NOMES BASE ABAIXO"});
  auto found=monsters.find(species);
  if(found!=monsters.end()) {
   addEquipment(drops,level,equipmentOnly);if(!equipmentOnly)addItems(drops,level,found->second.quest);
    if(species>=100){
     const auto* bossDrop=findBossDrop(species);
     if(bossDrop){
      auto& exclusive=add(drops,"boss-exclusive","DROP EXCLUSIVO DE CHEFE");
      exclusive.rows.push_back({bossDrop->namePt+" [LENDARIO]",color(4)});
      exclusive.rows.push_back({bossDrop->descPt,0xffff});
     }
    }
  }else rules.rows.push_back({"METADADOS INDISPONIVEIS"});
  auto& legend=add(drops,"rarities","CORES DE RARIDADE");
  for(int grade=equipmentOnly?2:0;grade<=(level?maxGrade(std::min(level,100)+2):4);++grade)legend.rows.push_back({rarity(grade),color(grade)});
  auto& spawn=add(out.sections[1],"maps","MAPAS");std::set<std::string> knownNames,knownIds;
  auto locations=spawns.find(species);
  if(locations!=spawns.end())for(const auto& id:locations->second) {
   const auto& map=maps.at(id);spawn.rows.push_back({map.name+" ["+id+"]"});knownIds.insert(id);knownNames.insert(map.name);knownNames.insert(map.name+" ["+id+"]");
  }
  if(facts)for(const auto& location:facts->locations) {
   if(location.empty() || knownNames.count(location))continue;
   auto id=location;
   if(id.size()>4 && id.substr(id.size()-4)==".mpd")id.resize(id.size()-4);
   auto match=maps.find(id);
   if(match==maps.end()) {
    // Older observers may supply the original display name. Accept only names
    // actually found in the map table, keeping unknown strings out of the UI.
    for(auto candidate=maps.begin();candidate!=maps.end();++candidate)if(candidate->second.name==location){match=candidate;break;}
   }
   if(match!=maps.end() && knownIds.insert(match->first).second) {
    spawn.rows.push_back({match->second.name+" ["+match->first+"]"});knownNames.insert(match->second.name);
   }
  }
  if(spawn.rows.empty())spawn.rows.push_back({"LOCAL NAO CONFIRMADO NOS DADOS ORIGINAIS"});
  auto levels=baseLevels.find(species);
  if(levels!=baseLevels.end() && !levels->second.empty()) {
   auto& g=add(out.sections[1],"base-levels","NIVEIS BASE NOS MAPAS / ROTEIROS");
   for(int base:levels->second)g.rows.push_back({"NIVEL BASE "+std::to_string(base)});
  }
  auto& variants=add(out.sections[2],"forms","FORMAS");
  if(species>=100){variants.rows.push_back({"CHEFE / HP VARIA COM O NIVEL"});variants.rows.push_back({"DROPS: EQUIPAMENTOS RAROS OU SUPERIORES"});}
  else {variants.rows.push_back({"NORMAL / HP VARIA COM O NIVEL"});variants.rows.push_back({"ELITE / HP 5x O NORMAL NO MESMO NIVEL"});variants.rows.push_back({"ELITE: SO EQUIPAMENTOS RAROS OU SUPERIORES"});}
  variants.rows.push_back({facts && facts->maxHP>0?std::string("ULTIMA FORMA OBSERVADA: ")+(species>=100?"CHEFE":facts->elite?"ELITE":"NORMAL"):"FORMA AINDA NAO OBSERVADA"});
  return out;
 }
};
} // namespace destinia
