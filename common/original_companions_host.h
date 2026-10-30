#pragma once
#include "original_ui_host.h"
#include "codex_portraits.h"
// Cosmetic mappings verified against original resources; save/buff IDs are unchanged.
class OriginalCompanionsHost {
 struct Art {int w=0,h=0;std::vector<uint32_t> pixels;};
 std::map<int,std::vector<Art>> cache;
 Art crop(const std::vector<uint8_t>& raster){
  int left=SCREEN_W,top=SCREEN_H,right=-1,bottom=-1;
  for(int y=0;y<SCREEN_H;y++)for(int x=0;x<SCREEN_W;x++){const auto at=(y*SCREEN_W+x)*2;if((uint16_t(raster[at])|uint16_t(raster[at+1])<<8)!=0xf81f){left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);}}
  if(right<left)throw std::runtime_error("Companion raster empty");const int naturalW=right-left+1,naturalH=bottom-top+1,longest=std::max(naturalW,naturalH);
  Art art;art.w=longest>24?std::max(1,naturalW*24/longest):naturalW;art.h=longest>24?std::max(1,naturalH*24/longest):naturalH;art.pixels.resize(art.w*art.h);
  for(int y=0;y<art.h;y++)for(int x=0;x<art.w;x++){const auto at=((top+y*naturalH/art.h)*SCREEN_W+left+x*naturalW/art.w)*2;const auto c=uint16_t(raster[at])|uint16_t(raster[at+1])<<8;art.pixels[y*art.w+x]=c==0xf81f?0:0x10000u|c;}return art;
 }

 std::vector<Art> load(int id){
  static const char* stems[]={"pet3","pet14","pet21","boss8"};
  const std::string stem=stems[id],wmb=stem+(id==3?"_head.wmb":".wmb"),agdName=stem+".agd";
  std::vector<uint8_t> packed;if(!readFile(assetsDir+"/"+wmb,packed))throw std::runtime_error("Companion original resource missing");
  const auto unpacked=destinia::codexUnpackResource(packed);if(unpacked.size()<8||unpacked[0]!='W'||unpacked[1]!='B'||unpacked[2]!='I')throw std::runtime_error("Companion WMB invalid");
  const unsigned imageCount=unpacked[4]|unsigned(unpacked[5])<<8;if(!imageCount||imageCount>256)throw std::runtime_error("Companion image count invalid");
  const auto filename=hmalloc(128),agd=hmalloc(8),groups=hmalloc(12);std::array<uint8_t,12> blank{};wr(agd,blank.data(),8);wr(groups,blank.data(),12);std::vector<unsigned> valid;
  originalUiHost.capture(SCREEN_W,SCREEN_H,[&](){
   wr(filename,wmb.c_str(),wmb.size()+1);const auto images=originalUiHost.call("WImg_loadNewWMBs",{filename,imageCount});if(!images)throw std::runtime_error("Companion WMB load failed");
   wr(filename,agdName.c_str(),agdName.size()+1);originalUiHost.call("loadAGD",{agd,filename});if(!rd16(agd))throw std::runtime_error("Companion AGD load failed");
   const auto actions=rd32(agd+4);if(!actions)throw std::runtime_error("Companion action table missing");const unsigned count=rd16(actions);const auto frames=rd32(actions+4);
   if(!count||count>128||!frames)throw std::runtime_error("Companion idle frames invalid");
   for(unsigned f=0;f<count;f++){
    const auto frame=frames+f*16;uint8_t partsCount=0;rd(frame+8,&partsCount,1);if(partsCount>64)throw std::runtime_error("Companion parts invalid");if(!partsCount)continue;
    const auto parts=originalUiHost.call("MC_GETDPTR",{rd32(frame+12)});if(!parts)throw std::runtime_error("Companion parts missing");std::vector<std::array<uint8_t,20>> body;
    for(unsigned i=0;i<partsCount;i++){std::array<uint8_t,20> part{};if(!rd(parts+i*20,part.data(),20))throw std::runtime_error("Companion part unreadable");if(!(part[12]&63))body.push_back(part);}
    for(size_t i=0;i<body.size();i++)wr(parts+u32(i)*20,body[i].data(),20);partsCount=uint8_t(body.size());wr(frame+8,&partsCount,1);if(partsCount)valid.push_back(f);
   }
   wr32(groups,images);
  },true);
  std::vector<Art> idle;for(auto frame:valid)idle.push_back(crop(originalUiHost.capture(SCREEN_W,SCREEN_H,[&](){originalUiHost.call("drawAGDImageT",{4,u32(SCREEN_W/2),u32(SCREEN_H/2),groups,agd,0,frame,0,0});},true)));
  if(idle.empty())throw std::runtime_error("Companion idle body missing");return idle;
 }
public:
 bool paint(uint8_t* output,int width,int height,int id,int heroX,int heroY,uint64_t frame,int level){
  if(!output||width<=0||height<=0||id<0||id>3)return false;
  try{
   auto found=cache.find(id);if(found==cache.end())found=cache.emplace(id,load(id)).first;const auto& idle=found->second;const auto& art=idle[(frame/6)%idle.size()];
   const int px=heroX-16-art.w/2,py=heroY-6-art.h;
   for(int y=0;y<art.h;y++)for(int x=0;x<art.w;x++){const auto c=art.pixels[y*art.w+x];const int xx=px+x,yy=py+y;if(!(c&0x10000)||xx<0||yy<0||xx>=width||yy>=height)continue;const auto at=(yy*width+xx)*2;output[at]=c&255;output[at+1]=(c>>8)&255;}
   destinia::paintOriginalText(output,width,height,px,py-12,"Lv"+std::to_string(level),0xd7bf,50,1);return true;
  }catch(const std::exception& error){hostReport("[companion] %s\n",error.what());return true;}
 }
 void install(){destinia::originalCompanionPainter=[this](uint8_t* p,int w,int h,int id,int x,int y,uint64_t frame,int level){return paint(p,w,h,id,x,y,frame,level);};}
};
static OriginalCompanionsHost originalCompanionsHost;
