// Uses original ARM UI routines and resources. No proprietary raster is stored in source.
export class OriginalUi {
 constructor(r){this.r=r;this.cache=new Map();this.descriptor=0;this.draws=0;}
 call(name,args=[]){const address=this.r.exports.get(name);if(!address)throw Error('Original UI routine unavailable: '+name);return this.r.call(address,args);}
 capture(w,h,draw,transparent=false){
  const r=this.r,W=r.width,H=r.height;
  if(!Number.isInteger(w)||!Number.isInteger(h)||w<1||h<1||w>W||h>H)throw Error('Original UI raster outside screen');
  if(!this.descriptor){this.descriptor=r.malloc(12);this.pixels=r.malloc(W*H*2);this.string=r.malloc(1024);r.put32(this.descriptor,W);r.put32(this.descriptor+4,H);r.put32(this.descriptor+8,this.pixels);}
  if(this.capturing||(r.e.reg_read_i32(r.uc.ARM_REG_PC)&~1)!==r.armReturn||r.uiRenderer?.frames?.length)throw Error('Original UI cannot render inside an active ARM drawing hook');
  const table=r.exports.get('frameBuffer')+16,grp=r.exports.get('Grp'),font=r.exports.get('Font'),g=r.exports.get('Game'),u=r.exports.get('GameUI');
  const blocks=[[grp,124],[font,64],[g,32056],[u,12088],[r.stackBase,r.stackSize],[r.exports.get('MMain')+0x3900,64]].map(([p,n])=>[p,r.read(p,n)]),slot=r.get32(table),scale=r.uiRenderer?.percent,error=r.error,drawing=r.drawingNativeUi;
  const registers=[...r.regs,r.uc.ARM_REG_SP,r.uc.ARM_REG_LR,r.uc.ARM_REG_PC,r.uc.ARM_REG_CPSR],values=registers.map(p=>r.e.reg_read_i32(p));
  r.drawingNativeUi=true;this.capturing=true;
  try{
   r.put32(table,this.descriptor);if(r.uiRenderer)r.uiRenderer.percent=100;
   const dimensions=new Uint8Array([W&255,W>>>8,H&255,H>>>8]);r.write(grp+92,dimensions);
   const blank=new Uint8Array(W*H*2);if(transparent)for(let i=0;i<blank.length;i+=2){blank[i]=31;blank[i+1]=248;}
   r.write(this.pixels,blank);this.call('Grp_setDrawOpVal',[0,0xffffffff]);draw();this.draws++;
   const full=r.read(this.pixels,W*H*2),out=new Uint8Array(w*h*2);for(let y=0;y<h;y++)out.set(full.subarray(y*W*2,(y*W+w)*2),y*w*2);return out;
  }finally{
   for(const [p,bytes]of blocks)r.write(p,bytes);r.put32(table,slot);if(r.uiRenderer)r.uiRenderer.percent=scale;
   // Restore CPSR before PC so the original ARM/Thumb mode is retained.
   r.e.reg_write_i32(registers.at(-1),values.at(-1));for(let i=0;i<registers.length-1;i++)r.e.reg_write_i32(registers[i],values[i]);
   r.error=error;r.drawingNativeUi=drawing;this.capturing=false;
  }
 }
 raster(key,w,h,draw,transparent=false){if(!this.cache.has(key)){if(this.cache.size>=256)this.cache.clear();this.cache.set(key,this.capture(w,h,draw,transparent));}return this.cache.get(key);}
 blit(output,data,rect,transparent=false,scale=1){const W=this.r.width,H=this.r.height;for(let y=0;y<rect.h;y++)for(let x=0;x<rect.w;x++){const at=(y*rect.w+x)*2,c=data[at]|data[at+1]<<8;if(transparent&&c===0xf81f)continue;for(let dy=0;dy<scale;dy++)for(let dx=0;dx<scale;dx++){const px=rect.x+x*scale+dx,py=rect.y+y*scale+dy;if(px<0||py<0||px>=W||py>=H)continue;const p=(py*W+px)*2;output[p]=c&255;output[p+1]=c>>>8;}}}
 panel(output,rect){const b={x:Math.round(rect.x),y:Math.round(rect.y),w:Math.min(this.r.width,Math.round(rect.w)),h:Math.min(this.r.height,Math.round(rect.h))};if(b.w<12||b.h<12)return;const menu=this.r.exports.get('GameUI')+0x24fc;const data=b.w>=140&&b.h>=40&&this.r.get16(menu+8)>0&&this.r.get16(menu+10)>0?this.windowRaster(b.w,b.h):this.raster('panel:'+b.w+':'+b.h,b.w,b.h,()=>this.call('_drawDescBox',[4,0,0,b.w,b.h]));this.blit(output,data,b);}
 text(output,x,y,value,color=0xffff,width=this.r.width-x,scale=1){value=String(value).normalize('NFD').replace(/[\u0300-\u036f]/g,'').replace(/[^\x20-\x7e]/g,'?').slice(0,160);const w=Math.min(this.r.width,Math.max(1,Math.floor(width/scale))),h=12,key=JSON.stringify(['text',value,color,w,this.r.language]);const data=this.raster(key,w,h,()=>{this.r.text(this.string,value);this.call('Font_setColorPixel',[color]);this.call('Font_drawStringChar',[4,this.string,0,0,w,12,1,0x11]);},true);this.blit(output,data,{x:Math.round(x),y:Math.round(y),w,h},true,scale);}
 cursor(output,x,y){const r=this.r,p=r.exports.get('MMain')+12,w=r.get16(p+8),h=r.get16(p+10),ax=r.get16(p+12,true),ay=r.get16(p+14,true);if(!w||!h||w>128||h>128)return;const data=this.raster('cursor:'+w+':'+h,w,h,()=>this.call('Grp_drawWImage',[4,ax,ay,p]),true);this.blit(output,data,{x:Math.round(x),y:Math.round(y),w,h},true);} windowRaster(w,h){
  if(!this.menuSkin){const r=this.r,u=r.exports.get('GameUI');this.menuSkin=this.capture(r.width,r.height,()=>{r.put32(u+0x7c,0);r.write(u+0x76,new Uint8Array([16,0]));r.write(r.exports.get('MMain')+0x3920,new Uint8Array([2,0]));this.call('GameUI_drawMainMenu',[4]);});}
  const key='window:'+w+':'+h;if(this.cache.has(key))return this.cache.get(key);
  const W=this.r.width,ox=(W-400)/2,oy=(this.r.height-240)/2,out=new Uint8Array(w*h*2);
  for(let y=0;y<h;y++)for(let x=0;x<w;x++){let sx=x<4?84+x:x>=w-4?316-(w-x):200,sy=y<4?25+y:y>=h-4?240-(h-y):132;if(x>=4&&x<w-4&&y>=4&&y<h-4)sx=306;const src=((oy+sy)*W+ox+sx)*2,dst=(y*w+x)*2;out[dst]=this.menuSkin[src];out[dst+1]=this.menuSkin[src+1];}
  if(this.cache.size>=256)this.cache.clear();this.cache.set(key,out);return out;
 }}
