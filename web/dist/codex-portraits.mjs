// Original APK 1.0.6 resources; no generated art or changes to the ARM game.
export const rowsPerPage=32;
export function portraitAssets(id){
 if(Number.isInteger(id)&&id>=1&&id<=63){const group=Math.floor((id-1)/3)+1;return [`mon${group}_${(id-1)%3}_head.wmb`,`mon${group}.agd`];}
 if(Number.isInteger(id)&&id>=100&&id<=109){const stem='boss'+(id===103?1:id-100);return [stem+'_head.wmb',stem+'.agd'];}return null;
}
const limit=2*1024*1024;
async function unpack(bytes){
 if(!bytes?.length||bytes.length>limit)return null;
 if(bytes[0]!==90||bytes[1]!==67||bytes[2]!==80||bytes[3]!==0)return bytes;
 if(bytes.length<12)return null;
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.length),size=v.getUint32(8,true);
 if(!size||size>limit||v.getUint32(4,true)!==bytes.length-12)return null;
 // Stream with an output bound: never materialize a decompression bomb.
 const reader=new Blob([bytes.subarray(12)]).stream().pipeThrough(new DecompressionStream('deflate')).getReader();
 const out=new Uint8Array(size);let at=0;
 try{for(;;){const {value,done}=await reader.read();if(done)break;if(at+value.length>size){await reader.cancel();return null;}out.set(value,at);at+=value.length;}return at===size?out:null;}finally{reader.releaseLock();}
}
// Original Monster_setAction: normal down/front idle is action 1; boss idle is 0.
export function decodePortrait(w,a,action=1){
 if(!w||!a||w.length<8||w.length>limit||w[0]!==87||w[1]!==66||w[2]!==73||w[3]!==0||a.length<12||a.length>limit||!Number.isInteger(action)||action<0||action>=a[0])return null;
 // loadAGD stores variable-length actions: 2-byte action header, then
 // 9-byte frame headers followed by 8 bytes per part. Skip complete frames.
 let pose=1;
 for(let n=0;n<=action;n++){
  if(pose+2>a.length)return null;
  const count=a[pose+1];pose+=2;if(!count)return null;
  if(n===action)break;
  for(let frame=0;frame<count;frame++){
   if(pose+9>a.length||a[pose+8]>64)return null;
   const end=pose+9+a[pose+8]*8;if(end>a.length)return null;pose=end;
  }
 }
 if(pose+9>a.length||!a[pose+8]||a[pose+8]>64||pose+9+a[pose+8]*8>a.length)return null;
 const parts=a[pose+8],partStart=pose+9;
 const u16=(b,p)=>b[p]|b[p+1]<<8,signed=n=>n>32767?n-65536:n;
 const frames=[],count=u16(w,4);if(count<1||count>256)return null;
 let pos=8,total=0,palette=[];
 for(let i=0;i<count;i++){
  if(pos+7>w.length)return null;
  const f={x:w[pos]<<24>>24,y:w[pos+1]<<24>>24,w:u16(w,pos+2),h:u16(w,pos+4)};let colors=w[pos+6];pos+=7;
  if(f.w<1||f.h<1||f.w>256||f.h>256||(total+=f.w*f.h)>limit)return null;
  if(colors){colors++;if(pos+colors*3>w.length)return null;palette=[];for(let j=0;j<colors;j++){const r=w[pos++],g=w[pos++],b=w[pos++];palette.push(j?0x10000|r>>3<<11|g>>2<<5|b>>3:0);}}
  if(!palette.length||pos+f.w*f.h>w.length)return null;
  f.pixels=new Uint32Array(f.w*f.h);for(let j=0;j<f.pixels.length;j++){const index=w[pos++];if(index>=palette.length)return null;f.pixels[j]=palette[index];}frames.push(f);
 }
 if(pos!==w.length)return null;
 const side=512,canvas=new Uint32Array(side*side);
 for(let i=0;i<parts;i++){
  const at=partStart+i*8,flags=a[at+4];if(flags&63)continue;
  const f=frames[a[at+5]];if(!f||a[at+7]!==0)return null;
  // Original drawAGDImageT adds part offsets to the WImg anchor.
  const flipX=!!(flags&64),flipY=!!(flags&128),anchorX=f.x+signed(u16(a,at)),anchorY=f.y+signed(u16(a,at+2));
  const x=(flipX?anchorX-f.w+1:-anchorX)+256,y=(flipY?anchorY-f.h+1:-anchorY)+256;
  if(x<0||y<0||x+f.w>side||y+f.h>side)return null;
  for(let yy=0;yy<f.h;yy++)for(let xx=0;xx<f.w;xx++){const c=f.pixels[(flipY?f.h-1-yy:yy)*f.w+(flipX?f.w-1-xx:xx)];if(c)canvas[(y+yy)*side+x+xx]=c;}
 }
 let left=side,top=side,right=-1,bottom=-1;for(let y=0;y<side;y++)for(let x=0;x<side;x++)if(canvas[y*side+x]){left=Math.min(left,x);top=Math.min(top,y);right=Math.max(right,x);bottom=Math.max(bottom,y);}
 if(right<left)return null;
 const width=right-left+1,height=bottom-top+1,pixels=new Uint32Array(width*height);
 for(let y=0;y<height;y++)pixels.set(canvas.subarray((top+y)*side+left,(top+y)*side+left+width),y*width);
 return {width,height,pixels};
}
export async function loadPortraits(assets,names){
 const cache=new Map(),unpacked=new Map();
 const read=async name=>{if(!unpacked.has(name))unpacked.set(name,unpack(assets.get(name)));return unpacked.get(name);};
 // A missing/corrupt picture never disables discovery, counters or the game.
 for(const id of names.keys()){try{const paths=portraitAssets(id);cache.set(id,paths?decodePortrait(await read(paths[0]),await read(paths[1]),id>=100?0:1):null);}catch{cache.set(id,null);}}
 return cache;
}
// Bounded original-resource decoder shared by the read-only Codex metadata.
export {unpack as unpackResource};
