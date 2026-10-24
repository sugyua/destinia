#pragma once
#include "ui_transform.h"
#include <unicorn/unicorn.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
namespace destinia {
// APK 1.0.6: intercept complete raster blocks, never individual pixel writes.
// A padded virtual destination keeps native clipping from discarding pixels
// which the UI matrix moves back inside the real canvas.
class UiRenderer {
 friend struct UiRendererFixture;
 using U=uint32_t;
 static constexpr U virtualBase=0x60000000,virtualSide=2048,padding=512,returnPc=0x60800000;
 uc_engine* e=nullptr;U base=0,grp=0,fb=0;int width=400,height=240,percent=100;
 enum class Kind{None,Status,Quick,Controls,Panel,Minimap};Kind kind=Kind::None;
 struct Frame{U lr;Kind previous;bool primitive=false;U descriptor=0;std::array<U,3> saved{};U borrowed=0,previousPointer=0;};
 std::vector<Frame> frames;std::map<std::string,U> symbols;std::vector<uc_hook> hooks;
 bool primitive=false;UiTransform matrix;U realBase=0;std::vector<uint16_t> canvas;
 U reg(int n)const {U v=0;static const int regs[]={UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_R4,UC_ARM_REG_R5,UC_ARM_REG_R6,UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_R10,UC_ARM_REG_R11,UC_ARM_REG_R12,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC};uc_reg_read(e,regs[n],&v);return v;}
 void set(int n,U v){static const int regs[]={UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_R4,UC_ARM_REG_R5,UC_ARM_REG_R6,UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_R10,UC_ARM_REG_R11,UC_ARM_REG_R12,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC};uc_reg_write(e,regs[n],&v);}
 U get(U p)const {U v=0;if(uc_mem_read(e,p,&v,4)!=UC_ERR_OK)throw std::runtime_error("UI: leitura ARM invalida");return v;}
 void put(U p,U v){if(uc_mem_write(e,p,&v,4)!=UC_ERR_OK)throw std::runtime_error("UI: escrita ARM invalida");}
 U arg(int n)const{return n<4?reg(n):get(reg(13)+4*(n-4));}
 void read(U p,void* data,size_t n){if(n&&uc_mem_read(e,p,data,n)!=UC_ERR_OK)throw std::runtime_error("UI: fonte ARM invalida");}
 U symbol(const char* n)const {auto it=symbols.find(n);if(it==symbols.end())throw std::runtime_error(std::string("UI: simbolo ausente ")+n);return it->second;}
 void hook(U pc){uc_hook h=0;if(uc_hook_add(e,&h,UC_HOOK_CODE,(void*)dispatch,this,pc,pc)!=UC_ERR_OK)throw std::runtime_error("UI: falha instalando hook");hooks.push_back(h);}
 static void dispatch(uc_engine*,uint64_t pc,uint32_t,void* data){auto& self=*static_cast<UiRenderer*>(data);try{self.at(U(pc));}catch(const std::exception& ex){self.error=ex.what();uc_emu_stop(self.e);}}
 static void unhandled(uc_engine*,uc_mem_type,uint64_t pc,int,int64_t,void* data){auto& self=*static_cast<UiRenderer*>(data);self.error="UI: raster ARM sem cobertura, pc="+std::to_string(self.reg(15)-self.base);uc_emu_stop(self.e);}
 UiTransform transform(int x,int y)const {
  UiAnchor anchor=UiAnchor::Center;int fw=0,fh=0;
  if(kind==Kind::Status)anchor=UiAnchor::TopLeft;
  if(kind==Kind::Minimap)anchor=x<width/2?UiAnchor::TopLeft:UiAnchor::TopRight;
  if(kind==Kind::Quick||kind==Kind::Controls){const bool right=x>=width/2,bottom=y>=height/2;anchor=right?(bottom?UiAnchor::BottomRight:UiAnchor::TopRight):(bottom?UiAnchor::BottomLeft:UiAnchor::TopLeft);}
  if(kind==Kind::Panel){fw=232;fh=240;}
  return{width,height,kind==Kind::Status?percent*statusBaseScale:percent,anchor,fw,fh};
 }
 void context(Kind next){if(next==Kind::Status)statusDrawn=true;frames.push_back({reg(14),kind});kind=next;set(14,returnPc|1);}
 void enterPrimitive(){

  if(primitive||kind==Kind::None||(reg(0)!=get(grp)&&reg(0)!=get(grp+4)))return;
  U handle=reg(0),descriptor=get(fb+handle*4);if(!descriptor)return;
  matrix=transform(int32_t(reg(1)),int32_t(reg(2)));realBase=get(descriptor+8);
  Frame f{reg(14),kind,true,descriptor,{get(descriptor),get(descriptor+4),realBase}};
  // Handle0 obtains dimensions from mainScreenWidth/Height, not its descriptor.
  // Route this leaf call through a temporary normal handle instead of changing
  // global screen dimensions used by layout and camera.
  if(handle==0){U slot=reg(3)==4?3:4;f.borrowed=slot;f.previousPointer=get(fb+slot*4);descriptor=returnPc+0x100;put(fb+slot*4,descriptor);put(descriptor,f.saved[0]);put(descriptor+4,f.saved[1]);put(descriptor+8,realBase);f.descriptor=descriptor;set(0,slot);}
  frames.push_back(f);primitive=true;
  put(descriptor,virtualSide);put(descriptor+4,virtualSide);put(descriptor+8,virtualBase);
  set(1,reg(1)+padding);set(2,reg(2)+padding);set(14,returnPc|1);
 }
 void leave(){if(frames.empty())throw std::runtime_error("UI: retorno sem contexto");auto f=frames.back();frames.pop_back();if(f.primitive){put(f.descriptor,f.saved[0]);put(f.descriptor+4,f.saved[1]);put(f.descriptor+8,f.saved[2]);if(f.borrowed)put(fb+f.borrowed*4,f.previousPointer);primitive=false;}kind=f.previous;set(14,f.lr);set(15,f.lr);}
 void begin(){canvas.resize(size_t(width)*height);read(realBase,canvas.data(),canvas.size()*2);}
 void finish(U next){if(uc_mem_write(e,realBase,canvas.data(),canvas.size()*2)!=UC_ERR_OK)throw std::runtime_error("UI: destino ARM invalido");++draws;set(15,next|1);}
 static uint16_t alpha(uint16_t old,uint16_t color,U weight){U mask=0x07e0f81f,s=(U(color)|(U(color)<<16))&mask,b=(U(old)|(U(old)<<16))&mask;U out=(s+(int32_t((b-s)*weight)>>4))&mask;return uint16_t(out|(out>>16));}
 void pixel(U dst,int offset,uint16_t color,int effect=0,U weight=0){
  int64_t address=int64_t(dst)+offset-virtualBase;if(address<0||address>=int64_t(virtualSide)*virtualSide*2||address%2)throw std::runtime_error("UI: destino virtual invalido");
  int n=int(address/2),x=n%virtualSide-padding,y=n/virtualSide-padding;auto area=matrix.rect(x,y,1,1);
  for(int yy=std::max(0,area.y);yy<std::min(height,area.y+area.h);++yy)for(int xx=std::max(0,area.x);xx<std::min(width,area.x+area.w);++xx){auto& old=canvas[size_t(yy)*width+xx];if(effect==1)old=alpha(old,color,weight);else if(effect==2)old=uint16_t(std::min((old&0xf800)+(color&0xf800),0xf800)|std::min((old&0x7e0)+(color&0x7e0),0x7e0)|std::min((old&31)+(color&31),31));else if(effect==3||effect==4){auto multiplied=uint16_t((((old>>11)*(color>>11))>>5)<<11|((((old>>5&63)*(color>>5&63))>>6)<<5)|((old&31)*(color&31)>>5));old=effect==4?alpha(old,multiplied,weight):multiplied;}else old=color;}
 }
 void indexed(U dst,U source,U palette,int w,int h,int sourceStride,int step,int stride,int effect,U next){
  if(w<=0||h<=0){set(15,next|1);return;}if(w>2048||h>2048||sourceStride<w)throw std::runtime_error("UI: dimensoes de imagem invalidas");
  std::vector<uint8_t> indices(size_t(h-1)*sourceStride+w);std::array<uint16_t,256> colors;read(source,indices.data(),indices.size());read(palette,colors.data(),512);U key=get(grp+0x50),weight=get(grp+0x68);begin();
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto c=colors[indices[size_t(y)*sourceStride+x]];if(c!=key)pixel(dst,y*stride+x*step,c,effect,weight);}finish(next);
 }
 void directCopy(){int w=reg(4),h=reg(8),srcStride=w+int32_t(reg(12))/2,dstStride=w*2+int32_t(reg(10));if(w<=0||h<=0)return;std::vector<uint16_t> source(size_t(h-1)*srcStride+w);read(reg(0),source.data(),source.size()*2);auto key=get(reg(5)+0x50);begin();for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto c=source[size_t(y)*srcStride+x];if(c!=key)pixel(reg(6),y*dstStride+x*2,c);}finish(base+0xa2ea);}
 void fill(){int w=reg(10),h=reg(11),stride=reg(8)*2;if(w<=0||h<=0)return;auto color=uint16_t(reg(5));int effect=get(reg(4)+0x64)==1?1:0;U weight=get(reg(4)+0x68);begin();for(int y=0;y<h;++y)for(int x=0;x<w;++x)pixel(reg(0),y*stride+x*2,color,effect,weight);finish(base+0xabc4);}
 void rawAlpha(U dst,U source,U masks,int w,int h,int sourceStride,int step,int stride,U shade,U common,U next){
  if(w<1||h<1){set(15,next|1);return;}if(w>2048||h>2048||sourceStride<w)throw std::runtime_error("UI: imagem alpha invalida");
  std::vector<uint16_t> colors(size_t(h-1)*sourceStride+w);std::vector<uint8_t> weights(colors.size());read(source,colors.data(),colors.size()*2);read(masks,weights.data(),weights.size());begin();
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){int i=y*sourceStride+x;U weight=weights[i]?uint16_t(weights[i]+shade+common):uint16_t(common);if(weight<=14)pixel(dst,y*stride+x*step,colors[i],weight?1:0,weight);}finish(next);
 }
 void expanded(){auto sp=reg(13);auto st=[&](U n){return get(sp+n);};int w=st(0x24),h=st(0x14);uint16_t pitch=0;read(reg(10)+8,&pitch,2);if(w<1||h<1){set(15,base+0xb287);return;}if(w>2048||h>2048||pitch>2048)throw std::runtime_error("UI: expand invalido");
  U sx=st(0x20),sy=st(0),step=reg(7),src=st(8);size_t lastY=(sy+U(h-1)*step)>>10,lastX=(sx+U(w-1)*step)>>10;std::vector<uint8_t> data(lastY*pitch+lastX+1);std::array<uint16_t,256> colors;read(src,data.data(),data.size());read(reg(8),colors.data(),512);U key=get(grp+0x50),weight=get(grp+0x68);int effect=get(grp+0x64)==1?1:0;begin();
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto c=colors[data[((sy+U(y)*step)>>10)*pitch+((sx+U(x)*step)>>10)]];if(c!=key)pixel(reg(5),(y*virtualSide+x)*2,c,effect,weight);}finish(base+0xb286);
 }
 void at(U pc){
  if(pc==returnPc){leave();return;}if(percent==100)return;U off=pc-base;
  switch(off){
   case 0x34684:context(Kind::Status);return;case 0x341d4:context(Kind::Quick);return;case 0xff30:context(Kind::Controls);return;case 0x34454:context(Kind::Minimap);return;
   case 0x21440:case 0x215ec:case 0x216a8:case 0x21854:case 0x31df8:case 0x15bec:case 0x1689c:case 0x16824:case 0x29000:case 0x29344:case 0x2943c:context(Kind::Panel);return;
   case 0xbb90:case 0xb4bc:case 0xb148:case 0xafc0:case 0xb8d4:case 0xae34:case 0xab40:case 0xa27c:case 0xaad4:enterPrimitive();return;
  }
  if(!primitive)return;auto sp=reg(13);auto stack=[&](U n){return get(sp+n);};
  switch(off){
   case 0xac34:fill();return;
   case 0xb32a:expanded();return;
   case 0xab34:begin();pixel(reg(0),reg(6),uint16_t(reg(4)));finish(base+0xab18);return;
   case 0xb0d6:{int w=reg(7),srcStride=w+int32_t(stack(0xc))/2;rawAlpha(reg(5),reg(6),reg(8),w,stack(4),srcStride,2,w*2+int32_t(stack(0x14)),stack(0x10),stack(0x1c),base+0xb066);return;}
   case 0xbb16:{int w=stack(0x44),step=int32_t(reg(9));rawAlpha(reg(5),reg(6),reg(8),w,stack(0x40),w+int32_t(stack(0x20)),step,w*step+int32_t(stack(0x24)),reg(11),reg(7),base+0xb9f0);return;}
   case 0xa346:directCopy();return;
   case 0xbd60:{int w=stack(0x78),step=int32_t(stack(0x34))*2;indexed(reg(5),reg(6),reg(8),w,stack(0x7c),w+int32_t(stack(0x3c)),step,w*step+int32_t(stack(0x38))*2,0,base+0xbc8c);return;}
   case 0xc120:{int w=stack(0x78),step=int32_t(stack(0x34))*2;indexed(reg(5),reg(6),reg(8),w,stack(0x7c),w+int32_t(stack(0x3c)),step,w*step+int32_t(stack(0x38))*2,1,base+0xbc8c);return;}
   case 0xb5e2:{int w=reg(4);indexed(reg(0),reg(1),reg(2),w,stack(0x64),w+int32_t(stack(0x24)),2,w*2+int32_t(stack(0x28))*2,0,base+0xb546);return;}
   case 0x9e18:case 0x9244:case 0x953c:case 0x93e0:case 0x9480:{int w=reg(3),step=int32_t(arg(6))*2;indexed(reg(0),reg(1),reg(2),w,arg(4),w+int32_t(arg(5)),step,w*step+int32_t(arg(7))*2,off==0x9244?1:off==0x953c?2:off==0x93e0?3:off==0x9480?4:0,reg(14));return;}
   default:return;
  }
 }
public:
 int statusBaseScale=1;bool statusDrawn=false;void beginFrame(){statusDrawn=false;}
 std::string error;uint64_t draws=0;
 void install(uc_engine* engine,U soBase,const std::map<std::string,U>& exports,int w,int h,int scale,bool detectUnsupported=false,const std::string& library=""){e=engine;base=soBase;symbols=exports;width=w;height=h;percent=scale;grp=symbol("Grp");fb=symbol("frameBuffer");if(percent==100)return;
  if(!library.empty()){std::ifstream file(library,std::ios::binary);uint64_t hash=14695981039346656037ull;char byte;while(file.get(byte))hash=(hash^static_cast<unsigned char>(byte))*1099511628211ull;if(!file.eof()||hash!=0x74b2547839b88777ull)throw std::runtime_error("UI: biblioteca ARM incompativel");}
  if(uc_mem_map(e,virtualBase,0x800000,UC_PROT_NONE)!=UC_ERR_OK||uc_mem_map(e,returnPc,0x1000,UC_PROT_READ|UC_PROT_EXEC)!=UC_ERR_OK)throw std::runtime_error("UI: viewport virtual indisponivel");
  for(U off:{0x34684,0x341d4,0xff30,0x34454,0x21440,0x215ec,0x216a8,0x21854,0x31df8,0x15bec,0x1689c,0x16824,0x29000,0x29344,0x2943c,0xbb90,0xb4bc,0xb148,0xafc0,0xb8d4,0xae34,0xab40,0xa27c,0xaad4,0xab34,0xb0d6,0xbb16,0xb32a,0xac34,0xa346,0xbd60,0xc120,0xb5e2,0x9e18,0x9244,0x953c,0x93e0,0x9480})hook(base+off);hook(returnPc);
  if(detectUnsupported){uc_hook detector;if(uc_hook_add(e,&detector,UC_HOOK_MEM_WRITE,(void*)unhandled,this,virtualBase,returnPc-1)!=UC_ERR_OK)throw std::runtime_error("UI: detector indisponivel");hooks.push_back(detector);}
 }
 std::pair<int,int> touchPoint(int x,int y,int state)const {
  if(percent==100||state==0||state==8)return{x,y};
  auto controls=[&](int a,int b){return UiTransform{width,height,percent,a>=width/2?(b>=height/2?UiAnchor::BottomRight:UiAnchor::TopRight):(b>=height/2?UiAnchor::BottomLeft:UiAnchor::TopLeft)};};
  auto pointIn=[&](int rx,int ry,int rw,int rh,const UiTransform& t,std::pair<int,int>& result){if(rw<=0||rh<=0||rw>width*2||rh>height*2)return false;auto box=t.rect(rx,ry,rw+1,rh+1);if(x<box.x||y<box.y||x>=box.x+box.w||y>=box.y+box.h)return false;result=t.inverse(x,y);return true;};
  auto main=symbol("MMain");std::pair<int,int> result;
  auto rectangle=[&](U at){std::array<int16_t,4> r{};if(uc_mem_read(e,at,r.data(),8)!=UC_ERR_OK)throw std::runtime_error("UI: retangulo de toque invalido");return r;};
  if(state==2){auto r=rectangle(main+0xe2);for(int i=0;i<2;++i){int rx=r[0]+i*r[2]/2,rw=r[2]/2;if(pointIn(rx,r[1],rw,r[3],controls(rx,r[1]),result))return result;}}
  for(int i=0;i<14;++i){auto r=rectangle(main+0xea+8*i);if(pointIn(r[0],r[1],r[2],r[3],controls(r[0],r[1]),result))return result;}
  if(state==1||state>=4){UiTransform panel{width,height,percent,UiAnchor::Center,232,240};return panel.inverse(x,y);}
  return{-4096,-4096};
 }
 bool ok()const{return error.empty();}
};
}
