import {UiTransform} from './ui-transform.mjs';
export function combatPower(s){const n=i=>Math.max(0,s[i]||0);return 4*n(7)+3*n(8)+Math.floor(n(5)/10)+Math.floor(n(6)/10)+Math.floor(n(9)/10)+Math.floor(n(10)/10)+Math.floor(n(11)/5)+Math.floor(n(12)/10);}
const signed=n=>(n>0?'+':'')+n;
export class CombatPower {
 constructor(r){this.r=r;this.comparison=false;this.captured=false;this.descriptor=0;this.nativeCache=new Map();this.beginFrame();}
 beginFrame(){this.status=null;this.candidates=[];this.panels=[];}
 install(){const r=this.r;for(const name of ['_drawMenuStatus','_drawEquipDesc']){const at=r.exports.get(name)&~1;r.e.hook_add(r.uc.HOOK_CODE,()=>{if(name==='_drawMenuStatus')this.status={x:r.r(1)|0,y:r.r(2)|0,w:r.r(3)};else if(r.r(3)&&this.candidates.length<4){this.candidates.push(r.r(3));this.panels.push({x:r.r(1)|0,y:r.r(2)|0,w:218});}},null,at,at);}}
 calculate(candidate=0,menu=true){const r=this.r,game=r.exports.get('Game'),ui=r.exports.get('GameUI'),original=r.read(game,32056),info={valid:false,current:0,projected:0,before:[],after:[],reason:''};
 const owned=p=>p>=game+0x2ed6&&p<game+0x2ed6+4000&&(p-game-0x2ed6)%20===0&&r.read(game+0x2e0c+(p-game-0x2ed6)/20,1)[0]!==0;
 const stats=()=>Array.from({length:13},(_,i)=>r.get16(game+0x48ba+2*i,true));
 try{
  for(let i=0;i<6;i++){const p=r.get32((menu?ui+0x2148:game+0x4584)+4*i);if(p&&!owned(p)){info.reason='NAO VERIFICADO';return info;}r.put32(game+0x4584+4*i,p);}
  r.call(r.exports.get('GameUI_calcHeroStat'));info.before=stats();info.current=combatPower(info.before);info.after=info.before.slice();info.projected=info.current;
  if(!candidate){info.valid=true;return info;}if(!owned(candidate)){info.reason='NAO VERIFICADO';return info;}
  const header=r.get16(candidate),type=header>>>5&7,required=r.call(r.exports.get('GameUI_getEquipMinLv'),[header,1]),identified=r.call(r.exports.get('GameUI_equipDistinguish'),[candidate]);
  if(type<3&&type!==r.get16(game+0x4886,true))info.reason='OUTRA CLASSE';else if(required>r.get16(game+0x48fa))info.reason='NIVEL BAIXO';else if(!identified)info.reason='IDENTIFIQUE';else if((header>>>8&7)>4)info.reason='NAO VERIFICADO';
  if(info.reason)return info;r.put32(game+0x4584+4*(type<3?0:type-2),candidate);r.call(r.exports.get('GameUI_calcHeroStat'));info.after=stats();info.projected=combatPower(info.after);info.valid=true;return info;
 }finally{r.write(game,original);}}
 selection(){const r=this.r,u=r.exports.get('GameUI');for(let i=0;i<this.candidates.length;i++){const pointer=this.candidates[i],equipped=Array.from({length:6},(_,n)=>r.get32(u+0x2148+4*n)).includes(pointer);if(equipped&&i+1<this.candidates.length)continue;return {pointer,panel:this.panels[i]};}return null;}
 visible(){const r=this.r,g=r.exports.get('Game'),u=r.exports.get('GameUI');return !r.inventory?.open&&!r.codexOpen&&r.get16(g+0x48fa)>0&&r.get16(g+0x4854,true)===16&&r.get16(u+0x76)>=2&&[0x10000002,0x20000002,0x91040004].includes(r.get32(u+0x7c))&&this.selection()!==null;}
 key(key,down){if(key===10015){if(down&&this.visible())this.comparison=!this.comparison;return true;}if(!down||!this.visible()||key!==2&&key!==11)return false;this.comparison=key===11;return true;}
 footerAt(x,y){if(!this.visible())return false;const p=new UiTransform(400,240,this.r.uiScale,'Center',232,240).inverse(x,y),b=this.selection().panel;return p.x>=b.x&&p.x<b.x+218&&p.y>=b.y+68&&p.y<b.y+82;}
 touch(x,y,phase){if(phase===0)this.captured=false;if(this.captured&&phase!==0){if(phase===1)this.captured=false;return true;}if(phase!==0||!this.visible())return false;const p=new UiTransform(400,240,this.r.uiScale,'Center',232,240).inverse(x,y),b=this.selection().panel;if(p.x<b.x||p.x>=b.x+218||p.y<b.y+68||p.y>=b.y+82)return false;this.comparison=p.x>=b.x+109;this.captured=true;return true;}
 nativeText(x,y,text,color=65535,width=208){const r=this.r;r.text(this.nativeString,String(text).slice(0,160));r.call(r.exports.get('Grp_setDrawOpVal'),[0,0xffffffff]);r.call(r.exports.get('Font_setColorPixel'),[color]);r.call(r.exports.get('Font_drawStringChar'),[4,this.nativeString,x,y,width,12,1,0x11]);}
 raster(kind,panel,info,pointer=0){const r=this.r,g=r.exports.get('Game'),owned=pointer>=g+0x2ed6&&pointer<g+0x2ed6+4000&&(pointer-g-0x2ed6)%20===0&&r.read(g+0x2e0c+(pointer-g-0x2ed6)/20,1)[0]!==0;
  const area=kind==='status'?{x:panel.x+98,y:panel.y+20,w:114,h:14}:kind==='item'?{x:panel.x,y:panel.y+68,w:218,h:14}:{x:panel.x,y:panel.y,w:218,h:82};
  if(area.x<0||area.y<0||area.x+area.w>400||area.y+area.h>240)return null;
  const key=JSON.stringify([kind,panel,info,kind==='compare'?pointer:0,r.language,owned&&kind==='compare'?Array.from(r.read(pointer,20)):null]);if(this.nativeCache.has(key))return this.nativeCache.get(key);
  if(!this.descriptor){this.descriptor=r.malloc(12);this.nativePixels=r.malloc(192000);this.nativeString=r.malloc(1024);r.put32(this.descriptor,400);r.put32(this.descriptor+4,240);r.put32(this.descriptor+8,this.nativePixels);}
  const table=r.exports.get('frameBuffer')+16,slot=r.get32(table),grp=r.exports.get('Grp'),graphics=r.read(grp,124),font=r.get32(r.exports.get('Font')),scale=r.uiRenderer.percent;let data;
  r.put32(table,this.descriptor);r.uiRenderer.percent=100;
  try{const blank=new Uint16Array(96000);if(kind==='status')blank.fill(0xf81f);r.write(this.nativePixels,new Uint8Array(blank.buffer));
   if(kind==='status'){this.nativeText(panel.x+101,panel.y+22,'FC',0xffc0,22);this.nativeText(panel.x+125,panel.y+22,info.current,65535,86);}
   else{r.call(r.exports.get('_drawDescBox'),[4,panel.x,panel.y,218,82]);
    if(kind==='compare'){let name='ITEM',rank=owned?r.get16(pointer)>>>8&7:5,color=65535;if(owned&&rank<=4&&info.reason!=='NAO VERIFICADO'){r.call(r.exports.get('GameUI_getEquipName'),[this.nativeString,pointer]);name=r.cstr(this.nativeString,512);color=r.call(r.exports.get('GameUI_getEquipColor'),[rank]);}this.nativeText(panel.x+4,panel.y+4,name,color);
     if(!info.valid){this.nativeText(panel.x+4,panel.y+16,'FC '+info.current);this.nativeText(panel.x+4,panel.y+30,r.language==='pt'?info.reason:({'OUTRA CLASSE':'OTHER CLASS','NIVEL BAIXO':'LOW LEVEL','IDENTIFIQUE':'IDENTIFY'}[info.reason]||'UNVERIFIED'),0xf800);}
     else{const d=info.projected-info.current;this.nativeText(panel.x+4,panel.y+16,`FC ${info.current} > ${info.projected}`,65535,150);this.nativeText(panel.x+158,panel.y+16,signed(d),deltaColor(d),56);let i=0;for(const[index,label]of[[7,'ATK'],[8,'DEF'],[5,r.language==='pt'?'PV':'HP'],[6,r.language==='pt'?'PM':'MP'],[11,'CRI']]){const n=info.after[index]-info.before[index],value=index===11?(n>0?'+':n<0?'-':'')+(Math.abs(n)/10).toFixed(1)+'%':signed(n);this.nativeText(panel.x+4+(i%2)*108,panel.y+28+Math.floor(i/2)*12,label+' '+value,deltaColor(n),104);i++;}}
    }
    this.nativeText(panel.x+10,panel.y+71,'ITEM',kind==='item'?0xffc0:0x8c51,60);this.nativeText(panel.x+88,panel.y+71,'F6/RS',0x8c51,40);this.nativeText(panel.x+154,panel.y+71,r.language==='pt'?'COMPARAR':'COMPARE',kind==='compare'?0xffc0:0x8c51,60);
   }
   const full=r.read(this.nativePixels,192000);data=new Uint8Array(area.w*area.h*2);for(let y=0;y<area.h;y++)data.set(full.subarray(((area.y+y)*400+area.x)*2,((area.y+y)*400+area.x+area.w)*2),y*area.w*2);
  }finally{r.uiRenderer.percent=scale;r.write(grp,graphics);r.put32(r.exports.get('Font'),font);r.put32(table,slot);}
  const raster={area,data,transparent:kind==='status'};if(this.nativeCache.size>=8)this.nativeCache.clear();this.nativeCache.set(key,raster);return raster;
 }
 blit(pixels,raster){if(!raster)return;const {area,data,transparent}=raster,t=new UiTransform(400,240,this.r.uiScale,'Center',232,240);if(!transparent&&this.r.uiScale===100){for(let y=0;y<area.h;y++)pixels.set(data.subarray(y*area.w*2,(y+1)*area.w*2),((area.y+y)*400+area.x)*2);return;}for(let y=0;y<area.h;y++)for(let x=0;x<area.w;x++){const i=(y*area.w+x)*2;if(transparent&&(data[i]|data[i+1]<<8)===0xf81f)continue;const b=t.rect(area.x+x,area.y+y,1,1);for(let yy=Math.max(0,b.y);yy<Math.min(240,b.y+b.h);yy++)for(let xx=Math.max(0,b.x);xx<Math.min(400,b.x+b.w);xx++){const at=(yy*400+xx)*2;pixels[at]=data[i];pixels[at+1]=data[i+1];}}}
 paint(pixels){const r=this.r,g=r.exports.get('Game'),u=r.exports.get('GameUI');if(r.inventory?.open||!r.get16(g+0x48fa)||r.get16(g+0x4854,true)!==16||r.get16(u+0x76)<2)return;if(this.status&&this.status.w>=220&&(r.get32(u+0x7c)>>>28)===0){const info=this.calculate();if(info.valid)this.blit(pixels,this.raster('status',this.status,info));}if(this.visible()){const {pointer,panel}=this.selection();this.blit(pixels,this.raster(this.comparison?'compare':'item',panel,this.comparison?this.calculate(pointer):null,pointer));}}
}
const deltaColor=n=>n>0?0x07e0:n<0?0xf800:0x8c51;
