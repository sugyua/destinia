const BASE=0x10000000,RETURN=0x60902000;
export function lootOptions(value={}){
 const out={visual:false,autoGold:false,autoMaterials:false,categories:31,minRank:0,radius:19,...value};
 for(const k of ['visual','autoGold','autoMaterials'])if(typeof out[k]!=='boolean')throw Error('Preferencia de loot invalida');
 for(const [k,lo,hi] of [['categories',0,31],['minRank',0,4],['radius',1,19]])if(!Number.isInteger(out[k])||out[k]<lo||out[k]>hi)throw Error('Preferencia de loot fora dos limites');return out;
}
export function lootMaterialTypes(bytes){if(bytes?.length!==158*166)throw Error('Tabela de materiais incompativel');const view=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),types=new Map();for(let i=0;i<158;i++){const p=i*166,id=view.getUint16(p,true),type=view.getUint16(p+4,true);if(id!==i||type>255)throw Error('Material invalido');types.set(id,type);}return types;}
export function describeDrop(type,id,types){let category=16,rank=0;if(type===0){category=1;rank=id>>>8&7;if(rank>4)category=16;}else if(type===5)category=4;else if(type===2)category=8;else if(type===1&&[14,15,16,17,22].includes(types.get(id)))category=2;return {type,id,category,rank};}
export const visibleDrop=(d,o)=>!o.visual||!!(o.categories&d.category)&&(d.category!==1||d.rank>=o.minRank);
export const autoEligible=(d,o,dx,dy)=>Math.abs(dx)<=o.radius&&Math.abs(dy)<=o.radius&&(o.autoGold&&d.category===4&&d.id>0||o.autoMaterials&&d.category===2);
export const lootColor=d=>d.category===1?[0xe71c,0x07e0,0x5dff,0xd01f,0xffe0][d.rank]:d.category===4?0xffe0:d.category===2?0x07ff:d.category===8?0xfd20:0xe71c;
export function paintLootMarker(pixels,w,h,x,y,color){const put=(px,py,c)=>{if(px>=0&&py>=0&&px<w&&py<h){const at=(py*w+px)*2;pixels[at]=c&255;pixels[at+1]=c>>>8;}};for(let dx=-7;dx<=7;dx++){const dy=7-Math.abs(dx);for(const sign of [-1,1]){put(x+dx,y+sign*dy,0);if(Math.abs(dx)<6)put(x+dx,y+sign*(dy-1),color);}}put(x,y,color);}
export class LootQol {
 constructor(r,options){this.r=r;this.options=lootOptions(options);this.types=lootMaterialTypes(r.lootMaterialTable);this.names=false;this.autoTarget=-1;this.lastTarget=-1;this.focused=true;this.retryAfter=Array(20).fill(0);}
 configure(options){this.options=lootOptions(options);}
 drop(index){const r=this.r,p=r.exports.get('Game')+0x3e90+index*60;return describeDrop(r.get16(p+48),r.get16(p+50),this.types);}
 gameplay(){const r=this.r;return r.get32(r.get32(BASE+0x44240))===r.exports.get('GameCanvas')&&r.get16(r.exports.get('Game')+0x4854)===0;}
 install(){const r=this.r;for(const [at,bytes] of [[0x20158,[240,181,95,70]],[0x209e4,[60,28,79,70]],[0x208d2,[73,70,123,88]],[0x1a0ca,[12,153,96,89]],[0x19a64,[99,89,63,74]],[0x2019c,[9,176,60,188]],[0x208e2,[1,52,60,53]]]){const actual=r.read(BASE+at,bytes.length);if(!bytes.every((v,i)=>v===actual[i]))throw Error('Loot: biblioteca ARM incompativel');}
  r.e.mem_map(RETURN,4096,r.uc.PROT_READ|r.uc.PROT_EXEC);
  for(const pc of [BASE+0x166e0,BASE+0x12444,BASE+0x1a0ca,BASE+0x209e4,BASE+0x208d2,RETURN])r.e.hook_add(r.uc.HOOK_CODE,()=>{try{this.at(pc);}catch(error){r.error=error;r.e.emu_stop();}},null,pc,pc);
 }
 at(pc){const r=this.r,setpc=value=>r.e.reg_write_i32(r.uc.ARM_REG_PC,value),lr=()=>r.e.reg_read_i32(r.uc.ARM_REG_LR)>>>0;
  if(pc===RETURN){this.names=false;r.e.reg_write_i32(r.uc.ARM_REG_LR,this.caller);setpc(this.caller);return;}
  if(pc===BASE+0x166e0){if(this.options.visual&&!this.names){this.names=true;this.caller=lr();r.e.reg_write_i32(r.uc.ARM_REG_LR,RETURN|1);}return;}
  if(pc===BASE+0x1a0ca){const i=r.r(2);if(this.options.visual&&i<20&&!visibleDrop(this.drop(i),this.options))setpc(BASE+0x19a65);return;}
  if(pc===BASE+0x12444){const i=r.r(0);if(this.autoTarget>=0&&(lr()&~1)===(BASE+0x208f4)&&i!==this.autoTarget||this.names&&i<20&&!visibleDrop(this.drop(i),this.options)){r.w(0,0);setpc(lr());}return;}
  if(this.autoTarget<0)return;if(pc===BASE+0x209e4)setpc(BASE+0x2019d);if(pc===BASE+0x208d2)setpc(BASE+0x208e3);
 }
 collect(frame){const r=this.r,g=r.exports.get('Game'),o=this.options;if(!this.focused||frame%4||!o.autoGold&&!o.autoMaterials||!this.gameplay()||r.get16(g+0x4866)!==0)return;
  const x=r.get16(g+0x4890,true),y=r.get16(g+0x4892,true);
  for(let n=1;n<=20;n++){const i=(this.lastTarget+n)%20,flag=r.read(g+0x3e76+i,1)[0];if(frame<this.retryAfter[i]||!flag||flag>=128)continue;const p=g+0x3e90+i*60;if(!autoEligible(this.drop(i),o,r.get16(p+40,true)-x,r.get16(p+42,true)-y))continue;this.lastTarget=i;this.autoTarget=i;try{r.call(r.exports.get('GameCanvas_mKeyPressed'),[6]);}finally{this.autoTarget=-1;}this.retryAfter[i]=r.read(g+0x3e76+i,1)[0]>0?frame+32:0;return;}
 }
 paint(pixels){if(!this.options.visual||!this.gameplay())return;const r=this.r,g=r.exports.get('Game'),cx=r.get16(g+0x4878,true),cy=r.get16(g+0x487a,true);for(let i=0;i<20;i++){const flag=r.read(g+0x3e76+i,1)[0];if(!flag||flag>=128)continue;const p=g+0x3e90+i*60,d=this.drop(i);if(visibleDrop(d,this.options))paintLootMarker(pixels,r.width,r.height,r.get16(p+40,true)-cx,r.get16(p+42,true)-cy,lootColor(d));}}
}
