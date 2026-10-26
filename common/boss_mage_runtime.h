#pragma once
#include "boss_mage.h"
#include "overlay_font.h"
#include <unicorn/unicorn.h>
#include <fstream>
namespace destinia {
class BossMageRuntime {
 uc_engine* engine=nullptr;uint32_t base=0,game=0,canvas=0,curMapData=0;int width=0,height=0;bool held=false,ready=false;std::optional<MageSite> request;
 BossMageArt art;std::map<std::pair<int,int>,MageSite> sites;std::function<bool()> manualAllowed;
 template<class T>T read(uint32_t at)const{T value{};if(uc_mem_read(engine,at,&value,sizeof value)!=UC_ERR_OK)throw std::runtime_error("Mago: leitura ARM invalida");return value;}
 uint32_t reg(int id)const{uint32_t value=0;uc_reg_read(engine,id,&value);return value;}
 static std::vector<uint8_t> resource(const std::string& path){std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Mago: recurso ausente");return {std::istreambuf_iterator<char>(in),{}};}
 static void hook(uc_engine*,uint64_t,uint32_t,void* data){auto& self=*static_cast<BossMageRuntime*>(data);if(!self.ready)return;try{self.input();}catch(const std::exception& ex){self.error=ex.what();uc_emu_stop(self.engine);}}
 void input(){
  const auto pc=reg(UC_ARM_REG_PC);const bool down=(pc&~1u)==press;const int key=int(reg(UC_ARM_REG_R0));if(key!=6&&key!=17)return;
  // Autoloot deliberately invokes the same ARM action; never turn it into dialogue.
  if(manualAllowed&&!manualAllowed())return;
  if(!down){if(!held)return;held=false;}else if(!held){const auto site=visibleSite();if(!site||!canInteract(*site))return;held=true;request=site;menu.openBossRushMenu();}
  const auto lr=reg(UC_ARM_REG_LR);uc_reg_write(engine,UC_ARM_REG_PC,&lr);
 }
 uint32_t press=0;
 bool canInteract(const MageSite& site)const{
  if(!focused||read<uint16_t>(game+0x4866)!=0||!mageFacing(site,read<int16_t>(game+0x4890),read<int16_t>(game+0x4892),read<int16_t>(game+0x4898)))return false;
  const int x=read<int16_t>(game+0x4890),y=read<int16_t>(game+0x4892);
  for(int i=0;i<20;++i)if(read<int8_t>(game+0x3e76+i)>0){const auto p=game+0x3e90+60*i;if(std::abs(int(read<int16_t>(p+40))-x)<=19&&std::abs(int(read<int16_t>(p+42))-y)<=19)return false;}
  return true;
 }
 void enforceCollision(const MageSite& site)const{
  if(!engine||!curMapData)return;
  const uint32_t p3=read<uint32_t>(curMapData+0x68);
  const uint16_t w=read<uint16_t>(curMapData),h=read<uint16_t>(curMapData+2);
  if(p3&&w>0&&h>0){
   int tx=site.x/16,ty=site.y/16;
   if(tx>=0&&tx<w&&ty>=0&&ty<h){
    uint8_t solid=1;uc_mem_write(engine,p3+size_t(ty)*w+tx,&solid,1);
   }
  }
 }
public:
 BossRushMenuEvent menu;std::string error;bool focused=true;
 void addCity(int area,int map,const std::vector<uint8_t>& mpd){auto site=placeBossMage(area,map,codexUnpackResource(mpd));if(site)sites[{area,map}]=*site;else sites.erase({area,map});}
 std::optional<MageSite> visibleSite()const{
  if(!ready||!engine||!error.empty())return std::nullopt;
  const auto active=read<uint32_t>(base+0x44240);if(!active||read<uint32_t>(active)!=canvas||read<uint16_t>(game+0x4854)!=0)return std::nullopt;
  const auto it=sites.find({read<uint16_t>(game+0x4870),read<uint16_t>(game+0x4872)});if(it==sites.end())return std::nullopt;const auto& site=it->second;
  const auto count=read<uint32_t>(game+0x29f0);if(count>20)return std::nullopt;
  for(uint32_t i=0;i<count;++i){const auto npc=game+0x1fa8+132*i;if(read<uint32_t>(npc+108)&&std::abs(int(read<int16_t>(npc+4))-site.x)<64&&std::abs(int(read<int16_t>(npc+6))-site.y)<64)return std::nullopt;}
  enforceCollision(site);
  return site;
 }
 void install(uc_engine* e,uint32_t soBase,const std::map<std::string,uint32_t>& symbols,int w,int h,const std::string& assets,const std::string& library,std::function<bool()> allowed={}){
  auto bytes=resource(library);uint64_t hash=14695981039346656037ull;for(auto b:bytes)hash=(hash^b)*1099511628211ull;if(hash!=0x74b2547839b88777ull)throw std::runtime_error("Mago: biblioteca incompativel");
  art=loadBossMageArt(resource(assets+"/mon6_0_head.wmb"),resource(assets+"/mon6.agd"),resource(assets+"/shadow.wmb"));for(int map:{61,62,63,64,65})addCity(0,map,resource(assets+"/m0_"+std::to_string(map)+".mpd"));
  engine=e;base=soBase;game=symbols.at("Game");canvas=symbols.at("GameCanvas");curMapData=symbols.count("curMapData")?symbols.at("curMapData"):0;width=w;height=h;manualAllowed=std::move(allowed);press=symbols.at("GameCanvas_mKeyPressed")&~1u;
  for(const char* name:{"GameCanvas_mKeyPressed","GameCanvas_mKeyReleased"}){uc_hook h;const auto at=symbols.at(name)&~1u;if(uc_hook_add(e,&h,UC_HOOK_CODE,reinterpret_cast<void*>(hook),this,at,at)!=UC_ERR_OK)throw std::runtime_error("Mago: falha no evento ARM");}
  ready=true;
 }
 void dispatch(){
  if(!menu.pending)return;const auto site=visibleSite();if(!site||!request||site->area!=request->area||site->map!=request->map||!canInteract(*site)){menu.pending=false;request.reset();return;}menu.dispatch();
 }
 void paint(uint8_t* pixels)const{
  const auto site=visibleSite();if(!site)return;
  const int hx=read<int16_t>(game+0x4890),hy=read<int16_t>(game+0x4892);
  const int x=site->x-read<int16_t>(game+0x4878),y=site->y-read<int16_t>(game+0x487a);
  paintBossMage(pixels,width,height,art.body,art.shadow,x,y);

  // Nome só aparece se o herói estiver por perto (raio de 48px, igual aos outros NPCs)
  if(std::abs(hx-site->x)>48||std::abs(hy-site->y)>48)return;

  const std::string label="MAGO DOS BOSSES";
  const int textW=overlayTextWidth(label),textH=11;
  const int left=x-textW/2,top=y-art.body.height-12;
  const int padX=4,padY=2;
  const int bx=left-padX,by=top-padY,bw=textW+padX*2,bh=textH+padY*2;

  // Fundo escuro translúcido com cantos arredondados (UI de NPC nativo)
  for(int py=std::max(0,by);py<std::min(height,by+bh);++py){
   for(int px=std::max(0,bx);px<std::min(width,bx+bw);++px){
    if((px==bx||px==bx+bw-1)&&(py==by||py==by+bh-1))continue;
    const size_t at=(size_t(py)*width+px)*2;
    uint16_t c=pixels[at]|(pixels[at+1]<<8);
    uint16_t dark=(c>>1)&0x7bef;
    pixels[at]=uint8_t(dark);pixels[at+1]=uint8_t(dark>>8);
   }
  }

  paintOverlayText(pixels,width,height,left+1,top+1,label,0);
  paintOverlayText(pixels,width,height,left,top,label,0xd7bf);
 }
};
}
