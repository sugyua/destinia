// Pixel loops for the verified APK 1.0.6. Native ARM still performs clipping,
// palette selection, effect dispatch, gameplay and the original function epilogue.
const base=0x10000000,limit=4*1024*1024;
function overlap(a,n,b,m){return a<b+m&&b<a+n;}
export function blitIndexed(r,{dst,src,palette,width:w,height:h,sourceStride,destinationStep:step,destinationStride:stride,key=null,effect=null}) {
 if(![dst,src,palette,sourceStride,step,stride].every(Number.isSafeInteger)||src<0||palette<0||palette+512>0x100000000)return false;
 if(!Number.isInteger(w)||!Number.isInteger(h)||w<1||h<1||w>2048||h>2048||w*h>limit||sourceStride<w||!step||Math.abs(step)>8192||Math.abs(stride)>8192)return false;
 const x=(w-1)*step,y=(h-1)*stride,lo=dst+Math.min(0,x)+Math.min(0,y),hi=dst+Math.max(0,x)+Math.max(0,y)+2,n=hi-lo,sourceSize=(h-1)*sourceStride+w;
 if(lo<0||hi>0x100000000||n>limit||sourceSize>limit||src+sourceSize>0x100000000||overlap(lo,n,src,sourceSize)||overlap(lo,n,palette,512))return false;
 const source=r.read(src,sourceSize),colors=r.read(palette,512),pixels=r.read(lo,n);
 for(let yy=0;yy<h;yy++)for(let xx=0;xx<w;xx++){
  const p=source[yy*sourceStride+xx]*2,c=colors[p]|colors[p+1]<<8;
  if(c!==key){const at=dst+yy*stride+xx*step-lo;const result=effect?effect(pixels[at]|pixels[at+1]<<8,c):c;pixels[at]=result&255;pixels[at+1]=result>>>8;}
 }
 r.write(lo,pixels);return true;
}
export function installFastGraphics(r) {
 const stats={draws:0,effects:0,alpha:0,multiply:0,fills:0,supported:false};
 const sites=[
  {at:0xbd60,end:0xbc8c,bytes:[31,154,0,42,0,220,145,231,14,152,13,153],kind:'normal'},
  {at:0xb5e2,end:0xb546,bytes:[10,155,9,157,0,38,95,0,188,70,25,159],kind:'simple'},
  {at:0x957e,end:0x9620,bytes:[73,70,0,41,64,221],kind:'dodge'},
  {at:0x98aa,end:0x99c8,bytes:[5,159,0,47,0,220],kind:'screen'},
  {at:0x941e,end:0x9468,bytes:[66,70,0,42,22,221],kind:'multiply'},
  {at:0x9282,end:0x92ce,bytes:[66,70,0,42,23,221],kind:'alpha'},
  {at:0xc120,end:0xbc8c,bytes:[31,154,0,42,0,220],kind:'alphaClip'}
 ];
 if(r.exports.get('Grp_drawClipWImageP')!==base+0xbb91||r.exports.get('Grp_drawSimpleClipWImageP')!==base+0xb4bd)return stats;
 if(!sites.every(site=>r.read(base+site.at,site.bytes.length).every((b,i)=>b===site.bytes[i])))return stats;
 stats.supported=true;
 for(const site of sites)r.e.hook_add(r.uc.HOOK_CODE,()=>{
  try {
   if(!r.fastGraphics||r.uiRenderer?.primitive)return;
   const sp=r.e.reg_read_i32(r.uc.ARM_REG_SP)>>>0,get=at=>r.get32(sp+at),simple=site.kind==='simple',light=!['normal','simple'].includes(site.kind);
   let args,commit=()=>{};
   if(light){
    const dodge=site.kind==='dodge',screen=site.kind==='screen',short=['alpha','multiply'].includes(site.kind),w=dodge?r.r(9):short?r.r(8):get(0x14),step=short?(r.r(9)|0):(get(dodge?4:0x18)|0);
    const value=name=>r.get32(r.exports.get(name));
    let effect;
    if(dodge){const red=value('maxR'),green=value('maxG'),blue=value('maxB');effect=(a,b)=>Math.min((a&red)+(b&red),red)|Math.min((a&green)+(b&green),green)|Math.min((a&blue)+(b&blue),blue);}
    else if(site.kind.startsWith('alpha')){
     const weight=r.get32(r.exports.get('Grp')+0x68),mask=0x07e0f81f;let lastB,lastS,lastR;
     effect=(a,b)=>{lastB=((a|a<<16)&mask)>>>0;lastS=((b|b<<16)&mask)>>>0;lastR=(lastS+(Math.imul(lastB-lastS,weight)>>4))&mask;return(lastR|lastR>>>16)&65535;};
     commit=()=>{if(lastB!==undefined){r.put32(r.exports.get('gProcVarB'),lastB);r.put32(r.exports.get('gProcVarS'),lastS);r.put32(r.exports.get('gProcVarR'),lastR);}};
    }else {
     const sr=value('shiftR'),sg=value('shiftG'),mr=value('maskR'),mg=value('maskG'),mb=value('maskB'),lr=value('lenR'),lg=value('lenG'),lb=value('lenB');
     effect=screen?(a,b)=>((mr-(Math.imul(mr&~(a>>sr),mr&~(b>>sr))>>lr))&mr)<<sr|((mg-(Math.imul(mg&~(a>>sg),mg&~(b>>sg))>>lg))&mg)<<sg|((mb-(Math.imul(mb&~a,mb&~b)>>lb))&mr)
      :(a,b)=>(Math.imul(a>>sr&mr,b>>sr&mr)>>lr)<<sr|(Math.imul(a>>sg&mg,b>>sg&mg)>>lg)<<sg|Math.imul(a&mb,b&mb)>>lb;
    }
    args={dst:short?r.r(11):get(dodge?0x18:0x3c),src:short?r.r(7):dodge?r.r(8):get(0x10),palette:short?r.r(10):get(dodge?8:0x1c),width:w,height:get(short?0x40:dodge?0x50:0x70),sourceStride:w+(get(short?0x44:dodge?0x54:0x74)|0),destinationStep:step,destinationStride:w*step+(get(short?0xc:dodge?0x20:0x44)|0),key:r.get32(r.exports.get('Grp')+0x50),effect};
    if(site.kind==='alphaClip'){const w=get(0x78),step=(get(0x34)|0)*2;args={dst:r.r(5),src:r.r(6),palette:r.r(8),width:w,height:get(0x7c),sourceStride:w+(get(0x3c)|0),destinationStep:step,destinationStride:w*step+(get(0x38)|0)*2,key:r.get32(r.r(9)+0x50),effect};}
   }else {
    const w=simple?r.r(4):get(0x78),step=simple?2:(get(0x34)|0)*2,gap=get(simple?0x28:0x38)|0;
    args={dst:r.r(simple?0:5),src:r.r(simple?1:6),palette:r.r(simple?2:8),width:w,height:get(simple?0x64:0x7c),sourceStride:w+(get(simple?0x24:0x3c)|0),destinationStep:step,destinationStride:w*step+gap*2,key:simple?null:r.get32(r.r(9)+0x50)};
   }
   if(blitIndexed(r,args)){commit();stats.draws++;if(light)stats.effects++;if(site.kind.startsWith('alpha'))stats.alpha++;if(site.kind==='multiply')stats.multiply++;r.e.reg_write_i32(r.uc.ARM_REG_PC,(base+site.end)|1);}

  }catch(error){r.error=error;r.e.emu_stop();}
 },null,base+site.at,base+site.at);
 // Rectangle clipping and color conversion have already run in the APK.
 const fillAt=base+0xac34,fillBytes=[99,110,129,70,1,43];
 if(r.read(fillAt,fillBytes.length).every((b,i)=>b===fillBytes[i]))r.e.hook_add(r.uc.HOOK_CODE,()=>{
  try{
   if(!r.fastGraphics||r.uiRenderer?.primitive)return;
   const dst=r.r(0),w=r.e.reg_read_i32(r.uc.ARM_REG_R10)|0,h=r.e.reg_read_i32(r.uc.ARM_REG_R11)|0,stride=r.r(8)*2,color=r.r(5)&65535,grp=r.r(4);
   if(w<=0||h<=0||w>2048||h>2048||stride<w*2||stride>8192)return;
   const n=(h-1)*stride+w*2;if(n>limit||dst+n>0x100000000)return;
   const pixels=r.read(dst,n),alpha=r.get32(grp+0x64)===1,weight=r.get32(grp+0x68),mask=0x07e0f81f,source=((color|color<<16)&mask)>>>0;
   let lastB=0,lastR=0;
   for(let y=0;y<h;y++)for(let x=0;x<w;x++){
    const at=y*stride+x*2;let out=color;
    if(alpha){const old=pixels[at]|pixels[at+1]<<8;lastB=((old|old<<16)&mask)>>>0;lastR=(source+(Math.imul(lastB-source,weight)>>4))&mask;out=(lastR|lastR>>>16)&65535;}
    pixels[at]=out&255;pixels[at+1]=out>>>8;
   }
   r.write(dst,pixels);
   if(alpha){r.put32(r.exports.get('gProcVarB'),lastB);r.put32(r.exports.get('gProcVarS'),source);r.put32(r.exports.get('gProcVarR'),lastR);}
   stats.fills++;r.e.reg_write_i32(r.uc.ARM_REG_PC,base+0xabc5);
  }catch(error){r.error=error;r.e.emu_stop();}
 },null,fillAt,fillAt);
 return stats;
}
