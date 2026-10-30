import {unpackResource} from './codex-portraits.mjs';

// Explicit cosmetic choices. These resource numbers are not companion species IDs.
const resources=['pet3','pet14','pet21','boss8_head'];
const animations=['pet3','pet14','pet21','boss8'];
const need=(ok,message)=>{if(!ok)throw Error('Original companion: '+message);};
function crop(raster,w,h){
 let left=w,top=h,right=-1,bottom=-1;
 for(let y=0;y<h;y++)for(let x=0;x<w;x++){const p=(y*w+x)*2;if((raster[p]|raster[p+1]<<8)!==0xf81f){left=Math.min(left,x);right=Math.max(right,x);top=Math.min(top,y);bottom=Math.max(bottom,y);}}
 if(right<left)return null;
 const sw=right-left+1,sh=bottom-top+1,ratio=Math.min(1,24/sw,24/sh),width=Math.max(1,Math.round(sw*ratio)),height=Math.max(1,Math.round(sh*ratio)),pixels=new Uint32Array(width*height);
 for(let y=0;y<height;y++)for(let x=0;x<width;x++){const px=left+Math.min(sw-1,Math.floor(x/ratio)),py=top+Math.min(sh-1,Math.floor(y/ratio)),at=(py*w+px)*2,c=raster[at]|raster[at+1]<<8;if(c!==0xf81f)pixels[y*width+x]=0x10000|c;}
 return {width,height,pixels};
}

export class OriginalCompanions {
 constructor(r){this.r=r;this.sprites=[];this.ready=false;this.pending=null;this.error=null;}
 prepare(){if(this.ready)return Promise.resolve(this);if(this.pending)return this.pending;this.pending=this.load().catch(error=>{this.error=error;this.sprites=[];this.ready=false;throw error;});return this.pending;}
 async load(){
  const r=this.r,ui=r.originalUi;need(ui&&r.exports?.get('Game'),'runtime must be initialized');
  const definitions=[];
  for(let id=0;id<4;id++){
   const wmb=resources[id]+'.wmb',agd=animations[id]+'.agd';
   let w,a;try{w=await unpackResource(r.assets.get(wmb));a=await unpackResource(r.assets.get(agd));}catch{throw Error('Original companion: corrupt '+wmb+' / '+agd);}
   need(w?.length>=8&&w[0]===87&&w[1]===66&&w[2]===73&&w[3]===0,'corrupt '+wmb);
   need(a?.length>=12&&a[0]>0&&a[0]<=64,'corrupt '+agd);
   const count=w[4]|w[5]<<8;need(count>0&&count<=256,'invalid WMB count '+wmb);definitions.push({wmb,agd,count,actions:a[0]});
  }
  const loaded=[];
  for(const definition of definitions){
   const name=r.malloc(128),agd=r.malloc(8),groups=r.malloc(12);let actions;
   // Loading and capture both run outside ARM hooks and restore all game/graphics state.
   ui.capture(r.width,r.height,()=>{
    r.text(name,definition.wmb);const images=ui.call('WImg_loadNewWMBs',[name,definition.count]);need(images,'loading '+definition.wmb);
    r.text(name,definition.agd);r.write(agd,new Uint8Array(8));ui.call('loadAGD',[agd,name]);need(r.get16(agd)===definition.actions,'loading '+definition.agd);
    r.write(groups,new Uint8Array(12));r.put32(groups,images);actions=[];
    const table=r.get32(agd+4);need(table,'missing action table '+definition.agd);
    // Only the Diablo head idle action is verified for this cosmetic resource.
    // Pets retain both original actions and their native frame counts.
    for(let action=0;action<(definition.wmb==='boss8_head.wmb'?1:definition.actions);action++){
     const count=r.get16(table+action*8),frames=r.get32(table+action*8+4);need(count>0&&count<=128&&frames,'invalid frames '+definition.agd);const valid=[];
     for(let frame=0;frame<count;frame++){
      const p=frames+frame*16,partCount=r.read(p+8,1)[0];need(partCount<=64,'invalid parts '+definition.agd);if(!partCount)continue;
      const parts=ui.call('MC_GETDPTR',[r.get32(p+12)]),body=[];need(parts,'missing parts '+definition.agd);
      for(let i=0;i<partCount;i++){const part=parts+i*20;if((r.read(part+12,1)[0]&63)===0)body.push(r.read(part,20));}
      body.forEach((part,i)=>r.write(parts+i*20,part));r.write(p+8,Uint8Array.of(body.length));if(body.length)valid.push(frame);
     }
     actions.push(valid);
    }
   });
   const rasters=actions.map((frames,action)=>frames.map(frame=>crop(ui.capture(r.width,r.height,()=>ui.call('drawAGDImageT',[4,Math.floor(r.width/2),Math.floor(r.height/2),groups,agd,action,frame,0,0]),true),r.width,r.height)).filter(Boolean));
   need(rasters.some(a=>a.length),'empty sprite '+definition.wmb);loaded.push({resource:definition.wmb,actions:rasters});
  }
  this.sprites=loaded;this.ready=true;return this;
 }
 paint(pixels,id,x,y,frame=0,level=1){
  if(!this.ready)return false;
  const sprite=this.sprites[id];if(!sprite)return false;
  const frames=sprite.actions[0],image=frames[((Math.floor(frame/6)%frames.length)+frames.length)%frames.length];if(!image)return false;
  const r=this.r;need(pixels?.length>=r.width*r.height*2,'invalid framebuffer');
  const left=Math.round(x)-Math.floor(image.width/2),top=Math.round(y)-image.height;
  for(let yy=0;yy<image.height;yy++)for(let xx=0;xx<image.width;xx++){const c=image.pixels[yy*image.width+xx],px=left+xx,py=top+yy;if((c&0x10000)&&px>=0&&py>=0&&px<r.width&&py<r.height){const at=(py*r.width+px)*2;pixels[at]=c&255;pixels[at+1]=c>>>8&255;}}
  return true;
 }
}
