import {decodePortrait} from './codex-portraits.mjs';
export const MAGE_ASSETS=Object.freeze(['mon6_0_head.wmb','mon6.agd','shadow.wmb']);
const need=(yes,message)=>{if(!yes)throw Error(message);};
export function bossMageColor(c){const r=(c>>>11)*8,g=(c>>>5&63)*4,b=(c&31)*8,luma=Math.floor((3*r+6*g+b)/10),i=Math.min(7,Math.floor(luma/32));return [0x0823,0x1048,0x18ee,0x22b6,0x449d,0x8e7f,0xd7bf,0xffff][r>g+40&&r>b+40?Math.min(2,i):i];}
function validateArt(art){need(art&&Number.isInteger(art.width)&&Number.isInteger(art.height)&&art.width>0&&art.height>0&&art.width<=256&&art.height<=256&&art.pixels?.length===art.width*art.height,'Mago: arte invalida');}
export function bossMagePalette(source){validateArt(source);return {...source,pixels:source.pixels.slice()};}
export async function unpackMageResource(bytes){
 need(bytes?.length&&bytes.length<=2097152,'Mago: recurso invalido');if(bytes[0]!==90||bytes[1]!==67||bytes[2]!==80||bytes[3]!==0)return bytes;
 need(bytes.length>=12,'Mago: ZCP truncado');const view=new DataView(bytes.buffer,bytes.byteOffset,bytes.length),size=view.getUint32(8,true);need(size>0&&size<=2097152&&view.getUint32(4,true)===bytes.length-12,'Mago: ZCP invalido');
 const reader=new Blob([bytes.subarray(12)]).stream().pipeThrough(new DecompressionStream('deflate')).getReader(),out=new Uint8Array(size);let at=0;
 try{for(;;){const {value,done}=await reader.read();if(done)break;if(at+value.length>size){await reader.cancel();throw Error('Mago: ZCP excessivo');}out.set(value,at);at+=value.length;}}finally{reader.releaseLock();}need(at===size,'Mago: ZCP tamanho divergente');return out;
}
export async function loadBossMageArt(assets){const [w,a,s]=await Promise.all(MAGE_ASSETS.map(n=>unpackMageResource(assets.get(n))));const body=decodePortrait(w,a),single=new Uint8Array(20);single[0]=1;single[2]=1;single[11]=1;const shadow=decodePortrait(s,single,0);validateArt(body);validateArt(shadow);return {body:bossMagePalette(body),shadow:{...shadow,pixels:shadow.pixels.slice()}};}
export function mageFacing(npc,x,y,facing){const dx=npc.x-x,dy=npc.y-y;if(Math.abs(dx)>28||Math.abs(dy)>28)return false;return facing===0?dx<0:facing===1?dx>0:facing===2?dy<0:facing===3?dy>0:false;}
export function placeBossMage(area,map,bytes){
 need(Number.isInteger(area)&&area>=0&&area<=2&&Number.isInteger(map)&&map>=0&&map<=255&&bytes?.length<=2097152,'Mago: mapa invalido');let at=0;
 const check=n=>need(at+n<=bytes.length,'Mago: MPD truncado'),take=()=>{check(1);return bytes[at++];},skip=n=>{check(n);at+=n;};
 const w=take(),h=take(),p=take(),q=take();need(w&&h,'Mago: dimensoes invalidas');skip(p+q);check(w*h*4);const grid=at+w*h*3;skip(w*h*4);const occupied=[];
 const records=(stride,offset)=>{const count=take();for(let i=0;i<count;i++){check(stride);occupied.push([bytes[at+offset]*16+8,bytes[at+offset+1]*16+8]);skip(stride);}};
 records(3,1);skip(2);records(7,0);records(5,0);records(6,0);skip(12);const spawns=take();skip(spawns*4);need(at===bytes.length,'Mago: dados MPD extras');if(spawns)return null;
 if(area===0){
  if(map===61)return {area,map,x:280,y:232};
  if(map===62)return {area,map,x:504,y:280};
  if(map===63)return {area,map,x:280,y:296};
  if(map===64)return {area,map,x:344,y:424};
  if(map===65)return {area,map,x:328,y:344};
 }
 let result=null,score=Infinity;for(let y=4;y<h-2;y++)for(let x=2;x<w-2;x++){let clear=true;for(let yy=y-3;yy<=y+1;yy++)for(let xx=x-1;xx<=x+1;xx++)if(bytes[grid+yy*w+xx]!==0)clear=false;
 const px=x*16+8,py=y*16+8;for(const [ox,oy] of occupied)if(Math.abs(px-ox)<64&&Math.abs(py-oy)<64)clear=false;const distance=(2*x-w)**2+(2*y-h)**2;if(clear&&distance<score){score=distance;result={area,map,x:px,y:py};}}
 return result;
}
export function paintBossMage(pixels,width,height,body,shadow,x,y){
 need(Number.isInteger(width)&&Number.isInteger(height)&&width>0&&height>0&&width<=4096&&height<=4096&&pixels?.length>=width*height*2,'Mago: raster invalido');validateArt(body);validateArt(shadow);
 const paint=(art,left,top)=>{for(let yy=0;yy<art.height;yy++)for(let xx=0;xx<art.width;xx++){const c=art.pixels[yy*art.width+xx],dx=left+xx,dy=top+yy;if(c&0x10000&&dx>=0&&dy>=0&&dx<width&&dy<height){const at=(dy*width+dx)*2;pixels[at]=c&255;pixels[at+1]=c>>>8&255;}}};
 paint(shadow,x-Math.floor(shadow.width/2),y-Math.floor(shadow.height/2));paint(body,x-Math.floor(body.width/2),y-body.height);
}
export class BossRushMenuEvent{constructor(){this.pending=false;this.dispatching=false;this.opener=null;}openBossRushMenu(){this.pending=true;}dispatch(){if(!this.pending||!this.opener||this.dispatching)return;this.dispatching=true;try{this.opener();this.pending=false;}finally{this.dispatching=false;}}}
const BASE=0x10000000;
export class BossMage{
 constructor(r,opener=null){this.r=r;this.menu=new BossRushMenuEvent();this.menu.opener=opener;this.ready=false;this.held=false;this.sites=new Map();this.request=null;}
 async addCity(area,map,bytes){const site=placeBossMage(area,map,await unpackMageResource(bytes));if(site)this.sites.set(`${area}:${map}`,site);else this.sites.delete(`${area}:${map}`);}
 install(){
  const r=this.r;need(r.exports.get('Game')===BASE+0x77ee0&&(r.exports.get('GameCanvas_mKeyPressed')&~1)===BASE+0x20158,'Mago: layout incompativel');
  for(const [name,down] of [['GameCanvas_mKeyPressed',true],['GameCanvas_mKeyReleased',false]]){const at=r.exports.get(name)&~1;r.e.hook_add(r.uc.HOOK_CODE,()=>{try{if(this.input(r.r(0),down))r.e.reg_write_i32(r.uc.ARM_REG_PC,r.e.reg_read_i32(r.uc.ARM_REG_LR));}catch(error){r.error=error;r.e.emu_stop();}},null,at,at);}
  this.prepared=(async()=>{this.art=await loadBossMageArt(r.assets);for(const map of [61,62,63,64,65]){const f=r.assets.get(`m0_${map}.mpd`);if(f)await this.addCity(0,map,f);}this.ready=true;})().catch(error=>{this.ready=false;r.log('Mago desativado: '+error.message);});
 }
 enforceCollision(site){
  const r=this.r,curMap=r.exports.get('curMapData');if(!curMap)return;
  const p3=r.get32(curMap+0x68),w=r.get16(curMap),h=r.get16(curMap+2);
  if(p3&&w>0&&h>0){
   const tx=Math.floor(site.x/16),ty=Math.floor(site.y/16);
   if(tx>=0&&tx<w&&ty>=0&&ty<h)r.write(p3+ty*w+tx,new Uint8Array([1]));
  }
 }
 visibleSite(){
  if(!this.ready)return null;const r=this.r,g=r.exports.get('Game'),active=r.get32(BASE+0x44240);if(!active||r.get32(active)!==r.exports.get('GameCanvas')||r.get16(g+0x4854)!==0)return null;
  const site=this.sites.get(`${r.get16(g+0x4870)}:${r.get16(g+0x4872)}`);if(!site)return null;const count=r.get32(g+0x29f0);if(count>20)return null;
  for(let i=0;i<count;i++){const npc=g+0x1fa8+132*i;if(r.get32(npc+108)&&Math.abs(r.get16(npc+4,true)-site.x)<64&&Math.abs(r.get16(npc+6,true)-site.y)<64)return null;}
  this.enforceCollision(site);
  return site;
 }
 canInteract(site){
  const r=this.r,g=r.exports.get('Game'),x=r.get16(g+0x4890,true),y=r.get16(g+0x4892,true);if(!r.lootQol.focused||r.get16(g+0x4866)!==0||!mageFacing(site,x,y,r.get16(g+0x4898,true)))return false;
  for(let i=0;i<20;i++){const flag=r.read(g+0x3e76+i,1)[0],p=g+0x3e90+60*i;if(flag>0&&flag<128&&Math.abs(r.get16(p+40,true)-x)<=19&&Math.abs(r.get16(p+42,true)-y)<=19)return false;}return true;
 }
 input(key,down){
  if((key!==6&&key!==17)||this.r.lootQol.autoTarget>=0)return false;
  if(!down){if(!this.held)return false;this.held=false;}else if(!this.held){const site=this.visibleSite();if(!site||!this.canInteract(site))return false;this.held=true;this.request=site;this.menu.openBossRushMenu();}return true;
 }
 dispatch(){if(!this.menu.pending)return;const site=this.visibleSite();if(!site||!this.request||site.area!==this.request.area||site.map!==this.request.map||!this.canInteract(site)){this.menu.pending=false;this.request=null;return;}this.menu.dispatch();}
 paint(pixels){
  const site=this.visibleSite();if(!site)return;const r=this.r,g=r.exports.get('Game'),x=site.x-r.get16(g+0x4878,true),y=site.y-r.get16(g+0x487a,true);paintBossMage(pixels,r.width,r.height,this.art.body,this.art.shadow,x,y);
  const hx=r.get16(g+0x4890,true),hy=r.get16(g+0x4892,true);
  if(Math.abs(hx-site.x)>48||Math.abs(hy-site.y)>48)return;
  if(!r.originalUi)return;
  const label='MAGO DOS BOSSES',bw=Math.min(r.width-8,112),bh=22,bx=Math.max(4,Math.min(r.width-bw-4,x-Math.floor(bw/2))),by=Math.max(4,Math.min(r.height-bh-4,y-this.art.body.height-bh-2));
  r.originalUi.panel(pixels,{x:bx,y:by,w:bw,h:bh});
  r.originalUi.text(pixels,bx+6,by+5,label,0xd7bf,bw-12,1);
 }
}
