#include "../common/codex_details.h"
#include <cassert>
#include <cstdio>
#include <fstream>
using namespace destinia;
static const CodexInfoSection& section(const CodexInfoView& v,const std::string& id) {
 for(const auto& s:v.sections)if(s.id==id)return s;
 assert(false);return v.sections.front();
}
static const CodexInfoGroup* group(const CodexInfoView& v,const std::string& id) {
 for(const auto& s:v.sections)for(const auto& g:s.groups)if(g.id==id)return &g;
 return nullptr;
}
static bool hasText(const CodexInfoView& v,const std::string& text) {
 for(const auto& s:v.sections)for(const auto& g:s.groups)for(const auto& r:g.rows)if(r.text.find(text)!=std::string::npos)return true;
 return false;
}
static void put(const std::filesystem::path& p,const std::vector<uint8_t>& b) {
 std::ofstream f(p,std::ios::binary|std::ios::trunc);f.write(reinterpret_cast<const char*>(b.data()),b.size());assert(bool(f));
}
int main(int argc,char** argv) {
 assert(argc==3);const std::filesystem::path assets=argv[1],scratch=std::filesystem::path(argv[2])/"metadata";
 CodexMetadata metadata;metadata.load(assets);
 // Map membership comes from referenced placement slots, never unused MPD slots.
 auto imperial=metadata.view(61);assert(imperial.hp=="VARIA COM O NIVEL");
 const auto* maps=group(imperial,"maps");assert(maps && maps->rows.size()==8);
 for(const auto& row:maps->rows)assert(row.text.find("m2_51")==std::string::npos);
 assert(hasText(imperial,"Lagrant Castle Dungeon - 15"));assert(hasText(imperial,"Dungeon Prison - 2"));
 assert(hasText(metadata.view(100),"Spirit Den - 10"));assert(hasText(metadata.view(101),"Salas Lair - 10"));
 assert(hasText(metadata.view(103),"NAO CONFIRMADO"));assert(hasText(metadata.view(109),"NAO CONFIRMADO"));
 // All 73 named species expose the same three sections; maps are loaded from all 172 originals.
 for(int species=1;species<=109;++species) {
  if(species>63 && species<100)continue;
  auto v=metadata.view(species);assert(v.sections.size()==3);
  assert(v.sections[0].id=="drops" && v.sections[1].id=="spawns" && v.sections[2].id=="variants");
  assert(!section(v,"drops").groups.empty());assert(!section(v,"variants").groups.empty());
  for(const auto& g:section(v,"drops").groups)for(const auto& r:g.rows)assert(r.text.size()<=55);
 }
 // Effective equipment levels include observed level +0..2, including rarity boundaries.
 for(const auto& boundary:std::vector<std::pair<int,int>>{{37,2},{38,3},{77,3},{78,4}}) {
  CodexMonsterFacts f;f.level=boundary.first;f.maxHP=1234;
  auto v=metadata.view(61,&f);assert(v.hp=="1234 / NV "+std::to_string(f.level));assert(hasText(v,"NIVEL OBSERVADO "+std::to_string(f.level)));
  for(int grade=0;grade<=4;++grade)assert(bool(group(v,"equipment-"+std::to_string(grade)))==(grade<=boundary.second));
 }
 const uint16_t colors[]={0xffff,0x07e0,0x041f,0xbb9f,0xfc08};
 CodexMonsterFacts f;f.level=80;f.maxHP=900;
 auto normal=metadata.view(1,&f);
 for(int grade=0;grade<=4;++grade){const auto* g=group(normal,"equipment-"+std::to_string(grade));assert(g && g->rows.size()==8);for(const auto& r:g->rows)assert(r.color==colors[grade]);}
 for(int grade=2;grade<=4;++grade){const auto* g=group(normal,"recipe-"+std::to_string(grade));assert(g && g->rows.size()==7);for(const auto& r:g->rows){assert(r.color==colors[grade]);assert(r.text.find("Recipe: ")==0);}}
 const auto* eggs=group(normal,"eggs");assert(eggs && eggs->rows.size()==4);
 for(const auto& r:eggs->rows)assert(r.color==0xffff);
 assert(eggs->rows[0].text=="Normal Monster Egg");assert(eggs->rows[2].text=="Rare Monster Egg");
 assert(group(normal,"quest"));assert(!group(metadata.view(61,&f),"quest"));
 assert(!hasText(normal,"Magic Powder"));assert(!hasText(normal,"Magic Fountain"));assert(!hasText(normal,"Magic Essence"));assert(!hasText(normal,"Magic Core"));
 assert(hasText(normal,"Supreme HP Potion"));assert(hasText(normal,"Orihalcon"));assert(!hasText(normal,"Inferior HP Potion"));
 assert(hasText(metadata.view(1),"Inferior HP Potion"));assert(hasText(metadata.view(1),"Supreme HP Potion"));
 // Elite and boss loot takes the equipment-only branch and starts at rare.
 f.elite=true;auto elite=metadata.view(61,&f);assert(!group(elite,"equipment-0") && !group(elite,"equipment-1"));
 assert(group(elite,"equipment-2") && group(elite,"equipment-3") && group(elite,"equipment-4"));
 assert(!group(elite,"eggs") && !group(elite,"gold") && !group(elite,"recipe-2") && !group(elite,"quest"));
 f.elite=false;auto boss=metadata.view(100,&f);assert(!group(boss,"equipment-0") && !group(boss,"eggs"));
 f.locations={"m2_30.mpd","Lagrant Castle Dungeon - 15","m0_0.mpd","m9_999.mpd"};
 auto merged=metadata.view(61,&f);assert(group(merged,"maps")->rows.size()==9);assert(hasText(merged,"South Darkwoods - 1"));assert(!hasText(merged,"m9_999"));assert(!hasText(merged,".mpd"));
 assert(hasText(normal,"5x"));
 assert(hasText(normal,"SO COM MISSAO ATIVA COMPATIVEL"));assert(!hasText(normal,"chave"));
 assert(hasText(boss,"ULTIMA FORMA OBSERVADA: CHEFE"));
 f.locations.clear();f.level=2;auto low=metadata.view(61,&f);assert(group(low,"rarities")->rows.size()==3);
 f.elite=true;auto lowElite=metadata.view(61,&f);assert(group(lowElite,"rarities")->rows.size()==1);
 assert(lowElite.notes.find("ULTIMO ENCONTRO: ELITE")!=std::string::npos);
assert(hasText(normal,"PREFIXO VARIA"));assert(hasText(normal,"NAO GARANTIDOS"));
 // Loading is transactional and every supported binary format rejects bounded malformed data.
 std::filesystem::create_directories(scratch);
 for(const auto& e:std::filesystem::directory_iterator(assets)) {
  const auto n=e.path().filename().string();
  if(e.path().extension()==".mpd" || n=="mon_data_en.gdt" || n=="item_en.gdt" || n=="equip_name_en.tdt" || n=="script_en.sdt" || n.find("mapname")==0)
   std::filesystem::copy_file(e.path(),scratch/e.path().filename(),std::filesystem::copy_options::overwrite_existing);
 }
 for(const std::string name:{"mon_data_en.gdt","item_en.gdt","equip_name_en.tdt","mapname0_en.tdt","script_en.sdt","m0_0.mpd"}) {
  auto path=scratch/name;std::ifstream in(path,std::ios::binary);std::vector<uint8_t> original{std::istreambuf_iterator<char>(in),{}};in.close();
  for(const auto& bad:std::vector<std::vector<uint8_t>>{{},{255},{255,255,255,255},{'Z','C','P',0,0,0,0,0,255,255,255,127}}) {
   put(path,bad);bool failed=false;try{metadata.load(scratch);}catch(const std::exception&){failed=true;}assert(failed);
   assert(group(metadata.view(61),"maps")->rows.size()==8);
  }
  put(path,original);
  if(name=="m0_0.mpd") {
   auto trailing=codexUnpackResource(original);trailing.push_back(0);put(path,trailing);
   bool failed=false;try{metadata.load(scratch);}catch(const std::exception&){failed=true;}assert(failed);put(path,original);
  }
 }
 std::puts("CODEX_DETAILS_73_SPECIES_172_MAPS_OK");
}
