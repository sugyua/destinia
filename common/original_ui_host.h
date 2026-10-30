#pragma once
#include "original_ui.h"
#include <map>
#include <vector>
#include <functional>
// Included after host ARM helpers. Calls only between completed top-level ARM runs.
class OriginalUiHost {
 struct Raster {int w=0,h=0;bool transparent=false;std::vector<uint8_t> bytes;};
 u32 descriptor=0,pixels=0,string=0;
 std::map<std::string,Raster> cache;
 std::vector<uint8_t> menuSkin;
 bool capturing=false;
 void ensure(){if(descriptor)return;descriptor=hmalloc(12);pixels=hmalloc(FB_BYTES);string=hmalloc(1024);wr32(descriptor,SCREEN_W);wr32(descriptor+4,SCREEN_H);wr32(descriptor+8,pixels);}
 void invoke(const char* name,std::initializer_list<u32> args){
  std::vector<u32> values(args);u32 sp=STACK_BASE+STACK_SIZE-256-u32(values.size()>4?(values.size()-4)*4:0);sp&=~7u;
  for(size_t i=4;i<values.size();i++)wr32(sp+u32(i-4)*4,values[i]);
  for(int i=0;i<4;i++)W(i,i<int(values.size())?values[i]:0);
  uc_reg_write(uc,UC_ARM_REG_SP,&sp);u32 lr=MAGIC_RET;uc_reg_write(uc,UC_ARM_REG_LR,&lr);
  fatal=false;hitMagic=false;
  const auto error=uc_emu_start(uc,exportsMap.at(name),MAGIC_RET,1000000,1000000);
  if(error!=UC_ERR_OK||fatal||(regPC()&~1u)!=MAGIC_RET)throw std::runtime_error(std::string("Original UI ARM capture failed: ")+name);
 }
 static void blit(uint8_t* output,int width,int height,int x,int y,const Raster& art,int scale){
  for(int sy=0;sy<art.h;sy++)for(int sx=0;sx<art.w;sx++){
   const auto at=(sy*art.w+sx)*2;const uint16_t c=uint16_t(art.bytes[at])|(uint16_t(art.bytes[at+1])<<8);if(art.transparent&&c==0xf81f)continue;
   for(int dy=0;dy<scale;dy++)for(int dx=0;dx<scale;dx++){const int xx=x+sx*scale+dx,yy=y+sy*scale+dy;if(xx<0||yy<0||xx>=width||yy>=height)continue;const auto dst=(yy*width+xx)*2;output[dst]=c&255;output[dst+1]=c>>8;}
  }
 }
 static std::string ascii(std::string value){
  const std::pair<const char*,const char*> accents[]={{u8"á","a"},{u8"à","a"},{u8"ã","a"},{u8"â","a"},{u8"é","e"},{u8"ê","e"},{u8"í","i"},{u8"ó","o"},{u8"ô","o"},{u8"õ","o"},{u8"ú","u"},{u8"ç","c"},{u8"Á","A"},{u8"Ã","A"},{u8"É","E"},{u8"Í","I"},{u8"Ó","O"},{u8"Ú","U"},{u8"Ç","C"}};
  for(const auto& pair:accents){size_t at=0;while((at=value.find(pair.first,at))!=std::string::npos)value.replace(at,strlen(pair.first),pair.second);}
  for(char& c:value)if(static_cast<unsigned char>(c)<32||static_cast<unsigned char>(c)>126)c='?';if(value.size()>160)value.resize(160);return value;
 }
public:
 // Resource adapters may issue raw calls only inside the isolated capture scope.
 u32 call(const char* name,std::initializer_list<u32> args){if(!capturing)throw std::runtime_error("Original UI call requires capture scope");invoke(name,args);return R(0);}
 size_t draws=0;
 std::vector<uint8_t> capture(int w,int h,const std::function<void()>& draw,bool transparent){
  if(capturing||(regPC()&~1u)!=MAGIC_RET)throw std::runtime_error("Original UI capture requires idle ARM runtime");
  if(w<=0||h<=0||w>SCREEN_W||h>SCREEN_H)throw std::runtime_error("Original UI capture dimensions invalid");
  ensure();
  struct Block{u32 at;std::vector<uint8_t> bytes;};std::vector<Block> blocks;
  for(const auto& item:std::initializer_list<std::pair<u32,size_t>>{{exportsMap.at("Game"),32056},{exportsMap.at("GameUI"),12088},{exportsMap.at("Grp"),124},{exportsMap.at("Font"),64},{exportsMap.at("MMain"),0x3922},{STACK_BASE,STACK_SIZE}}){Block b{item.first,std::vector<uint8_t>(item.second)};if(!rd(b.at,b.bytes.data(),b.bytes.size()))throw std::runtime_error("Original UI snapshot failed");blocks.push_back(std::move(b));}
  uc_context* context=nullptr;if(uc_context_alloc(uc,&context)!=UC_ERR_OK)throw std::runtime_error("Original UI context allocation failed");
  if(uc_context_save(uc,context)!=UC_ERR_OK){uc_context_free(context);throw std::runtime_error("Original UI context snapshot failed");}
  const auto table=exportsMap.at("frameBuffer")+16,slot=rd32(table);const bool oldPass=uiRenderer.originalPass,oldFatal=fatal,oldMagic=hitMagic,oldFailed=executionFailed;
  auto restore=[&](){for(const auto& b:blocks)wr(b.at,b.bytes.data(),b.bytes.size());wr32(table,slot);uc_context_restore(uc,context);uc_context_free(context);uiRenderer.originalPass=oldPass;fatal=oldFatal;hitMagic=oldMagic;executionFailed=oldFailed;capturing=false;};
  capturing=true;uiRenderer.originalPass=true;
  try{
   wr32(table,descriptor);uint16_t dimensions[2]={uint16_t(SCREEN_W),uint16_t(SCREEN_H)};wr(exportsMap.at("Grp")+92,dimensions,4);
   std::vector<uint8_t> blank(FB_BYTES);if(transparent)for(size_t i=0;i<blank.size();i+=2){blank[i]=31;blank[i+1]=248;}wr(pixels,blank.data(),blank.size());
   invoke("Grp_setDrawOpVal",{0,0xffffffff});draw();std::vector<uint8_t> full(FB_BYTES),out(w*h*2);rd(pixels,full.data(),full.size());for(int y=0;y<h;y++)std::memcpy(out.data()+y*w*2,full.data()+y*SCREEN_W*2,w*2);draws++;restore();return out;
  }catch(...){restore();throw;}
 }
 bool panel(uint8_t* output,int width,int height,int x,int y,int w,int h){
  if(!output||width<=0||height<=0||w<12||h<12||w>SCREEN_W||h>SCREEN_H)return false;
  const auto menu=exportsMap.at("GameUI")+0x24fc;const bool menuReady=rd16(menu+8)>0&&rd16(menu+10)>0;
  const auto key=std::string(menuReady&&w>=140&&h>=40?"window:":"panel:")+std::to_string(w)+":"+std::to_string(h);auto it=cache.find(key);
  if(it==cache.end()){
   Raster art{w,h,false,{}};
   if(w>=140&&h>=40&&menuReady){
    if(menuSkin.empty())menuSkin=capture(SCREEN_W,SCREEN_H,[&](){const auto ui=exportsMap.at("GameUI");wr32(ui+0x7c,0);uint16_t animation=16,hidden=2;wr(ui+0x76,&animation,2);wr(exportsMap.at("MMain")+0x3920,&hidden,2);invoke("GameUI_drawMainMenu",{4});},false);
    // Original 400x240 menu frame [84,25,232,215], centered by native runtime.
    // Preserve the four-pixel bevel; sample the clean interior away from labels.
    const int ox=(SCREEN_W-400)/2,oy=(SCREEN_H-240)/2;art.bytes.resize(w*h*2);
    for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
     const bool middleX=xx>=4&&xx<w-4,middleY=yy>=4&&yy<h-4;
     int sx=xx<4?84+xx:xx>=w-4?316-(w-xx):200;
     int sy=yy<4?25+yy:yy>=h-4?240-(h-yy):132;
     if(middleX&&middleY)sx=306;
     const auto src=((oy+sy)*SCREEN_W+ox+sx)*2,dst=(yy*w+xx)*2;art.bytes[dst]=menuSkin[src];art.bytes[dst+1]=menuSkin[src+1];
    }
   }else art.bytes=capture(w,h,[&](){invoke("_drawDescBox",{4,0,0,u32(w),u32(h)});},false);
   if(cache.size()>=256)cache.clear();it=cache.emplace(key,std::move(art)).first;
  }
  blit(output,width,height,x,y,it->second,1);return true;
 }
 bool text(uint8_t* output,int width,int height,int x,int y,const std::string& value,uint16_t color,int limit,int scale){
  if(!output||width<=0||height<=0||limit<=0||scale<1)return false;const int w=std::min(SCREEN_W,std::max(1,limit/scale));const auto label=ascii(value),key="text:"+std::to_string(w)+":"+std::to_string(color)+":"+label;auto it=cache.find(key);
  if(it==cache.end()){Raster art{w,12,true,capture(w,12,[&](){wr(string,label.c_str(),label.size()+1);invoke("Font_setColorPixel",{color});invoke("Font_drawStringChar",{4,string,0,0,u32(w),12,1,0x11});},true)};if(cache.size()>=256)cache.clear();it=cache.emplace(key,std::move(art)).first;}
  blit(output,width,height,x,y,it->second,scale);return true;
 }
 bool cursor(uint8_t* output,int width,int height,int x,int y){
  if(!output||width<=0||height<=0)return false;
  const auto pointer=exportsMap.at("MMain")+12;const int w=rd16(pointer+8),h=rd16(pointer+10),ax=int16_t(rd16(pointer+12)),ay=int16_t(rd16(pointer+14));
  if(w<1||h<1||w>128||h>128)return false;
  const auto key="cursor:"+std::to_string(pointer)+":"+std::to_string(w)+":"+std::to_string(h);auto it=cache.find(key);
  if(it==cache.end()){Raster art{w,h,true,capture(w,h,[&](){invoke("Grp_drawWImage",{4,u32(ax),u32(ay),pointer});},true)};if(cache.size()>=256)cache.clear();it=cache.emplace(key,std::move(art)).first;}
  blit(output,width,height,x-ax,y-ay,it->second,1);return true;
 }
 void install(){ensure();destinia::originalCursorPainter=[this](uint8_t* p,int w,int h,int x,int y){return cursor(p,w,h,x,y);};destinia::originalPanelPainter=[this](uint8_t* p,int w,int h,int x,int y,int bw,int bh){return panel(p,w,h,x,y,bw,bh);};destinia::originalTextPainter=[this](uint8_t* p,int w,int h,int x,int y,const std::string& value,uint16_t c,int limit,int scale){return text(p,w,h,x,y,value,c,limit,scale);};}
};
static OriginalUiHost originalUiHost;
