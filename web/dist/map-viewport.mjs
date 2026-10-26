// APK1.0.6 viewport guards, matching host_zoom. No global memory hooks.
const BASE=0x10000000,GAME=BASE+0x77ee0,MAP=BASE+0x80b18,FRAMEBUFFERS=BASE+0x4ef5c;
export const MAP_BACKGROUND=0x8410;
export function centeredCamera(mw,mh,w,h,x,y){return {x:mw*16<w?Math.trunc((mw*16-w)/2):x,y:mh*16<h?Math.trunc((mh*16-h)/2):y};}
export function mapViewportBounds(mw,mh,w,h,x,y){const clamp=(v,max)=>Math.max(0,Math.min(max,v));return {x0:clamp(-x,w),x1:clamp(-x+mw*16,w),y0:clamp(-y,h),y1:clamp(-y+mh*16,h)};}
export function tileInsideMap(x,y,mw,mh,cx,cy){return x+cx>=0&&x+cx<mw*16&&y+cy>=0&&y+cy<mh*16;}
export function clearMapMargins(pixels,w,h,{x0,x1,y0,y1}){
 if(!Number.isInteger(w)||!Number.isInteger(h)||w<1||h<1||w>4096||h>4096||pixels.length!==w*h*2||![x0,x1,y0,y1].every(Number.isInteger)||x0<0||x1<x0||x1>w||y0<0||y1<y0||y1>h)throw Error('Viewport: raster invalido');
 if(x0===0&&x1===w&&y0===0&&y1===h)return;
 const row=new Uint8Array(w*2);for(let x=0;x<w;x++){row[x*2]=MAP_BACKGROUND&255;row[x*2+1]=MAP_BACKGROUND>>>8;}
 for(let y=0;y<h;y++){const at=y*w*2;if(y<y0||y>=y1)pixels.set(row,at);else{if(x0>0)pixels.set(row.subarray(0,x0*2),at);if(x1<w)pixels.set(row.subarray(x1*2),at+x1*2);}}
}
const cameraSites=[
 [0x11b96,[68,70,34,4]],[0x11bc8,[12,188,144,70]],[0x14e64,[97,89,98,75]],[0x14eb8,[81,73,96,89]],
 [0x14ee4,[2,32,9,94]],[0x14f0c,[60,188,144,70]],[0x14f7a,[115,231,98,70]],[0x14f8e,[147,231,82,66]],
 [0x19666,[2,50,153,90]],[0x1966e,[10,58,155,90]],[0x19d32,[2,51,242,90]],[0x19d3c,[241,91,9,4]],
 [0x1e3aa,[224,82,59,231]],[0x1e3ac,[59,231,68,70]],[0x1f92e,[154,90,0,32]],[0x1f94c,[99,136,1,34]]
];
export class MapViewport{
 constructor(r){this.r=r;this.enabled=r.width===800&&r.height===480;this.cacheTiles=true;this.fastReject=true;this.context=null;this.state=null;this.stats={centered:0,cleared:0,skipped:0,sourceReads:0,paletteReads:0,batched:0};}
 releasePixels(){const c=this.context;if(c?.pixels){this.r.write(c.address,c.pixels);c.pixels=null;}}
 pixelWindow(address,size,source,sourceSize,palette){
  const c=this.context;if(!c?.pixels)return null;
  const overlaps=(at,n)=>at<c.address+c.size&&c.address<at+n;
  if(address<c.address||address+size>c.address+c.size||overlaps(source,sourceSize)||overlaps(palette,512)){this.releasePixels();return null;}
  this.stats.batched++;return c.pixels.subarray(address-c.address,address-c.address+size);
 }
 tileBytes(address,size,palette=false){
  const c=this.context;if(!c||address<c.address+c.size&&c.address<address+size)return this.r.read(address,size);
  const cache=palette?c.palettes:c.sources,key=address+':'+size;let bytes=cache.get(key);
  if(!bytes){bytes=this.r.read(address,size);cache.set(key,bytes);this.stats[palette?'paletteReads':'sourceReads']++;}return bytes;
 }
 centerSmallMapCamera(){
  const r=this.r,mw=r.get16(MAP,true),mh=r.get16(MAP+2,true);if(mw<=0||mh<=0)return null;
  const oldX=r.get16(GAME+18552,true),oldY=r.get16(GAME+18554,true),p=centeredCamera(mw,mh,r.width,r.height,oldX,oldY);
  for(const [axis,value,old] of [[0,p.x,oldX],[1,p.y,oldY]])if(value!==old){r.write(GAME+18552+axis*2,new Uint8Array([value&255,value>>8&255]));this.stats.centered++;}
  return {mw,mh,x:p.x,y:p.y};
 }
 mapEdgeClear(){
  const r=this.r;this.state=this.centerSmallMapCamera();if(!this.state)return;const index=r.r(0);if(index>=5)return;
  const descriptor=r.get32(FRAMEBUFFERS+4*index);if(!descriptor)return;const w=r.get32(descriptor),h=r.get32(descriptor+4),address=r.get32(descriptor+8);
  if(!w||!h||w>4096||h>4096||!address)return;
  if(this.cacheTiles&&r.fastGraphics&&r.graphicsStats?.supported&&!this.context){
   const grp=r.exports.get('Grp'),plain=r.get32(grp+0x64)===0&&r.get32(grp+0x48)===16;
   this.context={caller:r.e.reg_read_i32(r.uc.ARM_REG_LR)>>>0,address,size:w*h*2,sources:new Map(),palettes:new Map(),pixels:plain?r.read(address,w*h*2):null};r.e.reg_write_i32(r.uc.ARM_REG_LR,0x60903001);
  }
  const {mw,mh,x,y}=this.state,b=mapViewportBounds(mw,mh,w,h,x,y);
  if(b.x0===0&&b.x1===w&&b.y0===0&&b.y1===h)return;
  // One bounded read/write, rather than a WASM round trip for every border row.
  const pixels=this.context?.pixels||r.read(address,w*h*2);clearMapMargins(pixels,w,h,b);if(!this.context?.pixels)r.write(address,pixels);this.stats.cleared++;
 }
 mapEdge(){
  const r=this.r,lr=r.e.reg_read_i32(r.uc.ARM_REG_LR)>>>0,caller=lr&~1;if(caller<BASE+0x19308||caller>=BASE+0x19308+660)return;
  // Map_drawBuff does not write the camera; retain its entry snapshot for tiles.
  const s=this.state||this.centerSmallMapCamera();if(s&&!tileInsideMap(r.r(1)|0,r.r(2)|0,s.mw,s.mh,s.x,s.y)){r.w(1,0x4000);if(this.fastReject)r.e.reg_write_i32(r.uc.ARM_REG_PC,lr);this.stats.skipped++;}
 }
 install(){
  if(!this.enabled)return;const r=this.r,sites=[...cameraSites,[0xbb90,[240,181,95,70]],[0xb4bc,[240,181,95,70]],[0x19308,[240,181,95,70]]];
  if(r.exports.get('Game')!==GAME||r.exports.get('curMapData')!==MAP||r.exports.get('frameBuffer')!==FRAMEBUFFERS)throw Error('Viewport: layout ARM incompativel');
  for(const [pc,bytes] of sites)if(!r.read(BASE+pc,bytes.length).every((b,i)=>b===bytes[i]))throw Error('Viewport: instrucao ARM incompativel');
  // Scope immutable resource caching to this single original renderer call.
  r.e.mem_map(0x60903000,4096,r.uc.PROT_READ|r.uc.PROT_EXEC);
  r.e.hook_add(r.uc.HOOK_CODE,()=>{try{const context=this.context;if(!context)throw Error('Viewport: retorno sem contexto');this.releasePixels();this.context=null;r.e.reg_write_i32(r.uc.ARM_REG_LR,context.caller);r.e.reg_write_i32(r.uc.ARM_REG_PC,context.caller);}catch(error){r.error=error;r.e.emu_stop();}},null,0x60903000,0x60903000);
  const hook=(pc,fn)=>r.e.hook_add(r.uc.HOOK_CODE,()=>{if(!this.enabled)return;try{fn();}catch(error){r.error=error;r.e.emu_stop();}},null,BASE+pc,BASE+pc);
  for(const [pc] of cameraSites)hook(pc,()=>this.centerSmallMapCamera());
  hook(0x19308,()=>this.mapEdgeClear());hook(0xbb90,()=>this.mapEdge());hook(0xb4bc,()=>this.mapEdge());
 }
}
