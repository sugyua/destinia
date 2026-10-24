import {UiTransform} from './ui-transform.mjs';
import {overlayGlyph} from './overlay-font.mjs';
export function combatPower(s){const n=i=>Math.max(0,s[i]||0);return 4*n(7)+3*n(8)+Math.floor(n(5)/10)+Math.floor(n(6)/10)+Math.floor(n(9)/10)+Math.floor(n(10)/10)+Math.floor(n(11)/5)+Math.floor(n(12)/10);}
const signed=n=>(n>0?'+':'')+n;
export class CombatPower {
 constructor(r){this.r=r;this.beginFrame();}
 beginFrame(){this.status=null;this.candidates=[];}
 install(){const r=this.r;for(const name of ['_drawMenuStatus','_drawEquipDesc']){const at=r.exports.get(name)&~1;r.e.hook_add(r.uc.HOOK_CODE,()=>{if(name==='_drawMenuStatus')this.status={x:r.r(1)|0,y:r.r(2)|0,w:r.r(3)};else if(r.r(3)&&this.candidates.length<4)this.candidates.push(r.r(3));},null,at,at);}}
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
 paint(pixels){const r=this.r,game=r.exports.get('Game'),ui=r.exports.get('GameUI');if(!this.status&&!this.candidates.length||!r.get16(game+0x48fa)||r.get16(game+0x4854,true)!==16)return;
 const t=new UiTransform(400,240,r.uiScale,'Center',232,240),rect=(x,y,w,h,c)=>{const b=t.rect(x,y,w,h);for(let yy=Math.max(0,b.y);yy<Math.min(240,b.y+b.h);yy++)for(let xx=Math.max(0,b.x);xx<Math.min(400,b.x+b.w);xx++){const at=2*(yy*400+xx);pixels[at]=c&255;pixels[at+1]=c>>>8;}},text=(x,y,str,c)=>{for(const ch of str){const bits=overlayGlyph(ch);for(let i=0;i<5;i++)for(let j=0;j<11;j++)if(bits[i]&(1<<j))rect(x+i,y+j-2,1,1,c);x+=6;}};
 if(this.status){const info=this.calculate();if(info.valid){const{x,y}=this.status;rect(x+98,y+22,114,13,0x1946);text(x+101,y+25,'FC '+info.current,65535);}}
 for(let i=0;i<this.candidates.length;i++){const p=this.candidates[i],equipped=Array.from({length:6},(_,n)=>r.get32(ui+0x2148+4*n)).includes(p);if(equipped&&i+1<this.candidates.length)continue;const info=this.calculate(p),x=2,y=54;rect(x,y,80,96,0x1946);text(x+4,y+5,r.language==='pt'?'COMPARAR':'COMPARE',0xe5b2);text(x+4,y+17,'FC '+info.current,65535);if(!info.valid){text(x+4,y+31,(r.language==='pt'?info.reason:({'OUTRA CLASSE':'OTHER CLASS','NIVEL BAIXO':'LOW LEVEL','IDENTIFIQUE':'IDENTIFY'}[info.reason]||'UNVERIFIED')).slice(0,12),0xf800);break;}
 const delta=info.projected-info.current;text(x+4,y+29,(r.language==='pt'?'NOVO ':'NEW ')+info.projected,65535);text(x+4,y+41,signed(delta),color(delta));let row=0;for(const[index,label]of[[7,'ATK'],[8,'DEF'],[5,'PV'],[6,'PM'],[11,'CRI']]){const d=info.after[index]-info.before[index];text(x+4,y+51+9*row++,(r.language!=='pt'&&index===5?'HP':r.language!=='pt'&&index===6?'MP':label)+' '+(index===11?(d>0?'+':'')+(d/10).toFixed(1):signed(d)),color(d));}break;
 }
 function color(n){return n>0?0x07e0:n<0?0xf800:0x8c92;}
 }
}
