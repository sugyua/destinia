import {weaponAttack,armorDefense,armorHealth} from './refinement-policy.mjs';
﻿// ARM offsets verified against the pinned 1.0.6 ELF, not general engine ABI.
export function refinementBonus(value,level){return weaponAttack(Math.max(0,value),level)-Math.max(0,value);}
export function boundedStat(value){return Math.max(0,Math.min(32767,Math.floor(value)));}
export class Features0130Stats {
 constructor(engine){this.r=engine;this.totals=Array(13).fill(0);this.random=0x13579bdf;}
 install(){const r=this.r,base=0x10000000,g=r.exports.get('Game');
  const hook=(offset,fn)=>r.e.hook_add(r.uc.HOOK_CODE,()=>{try{fn();}catch(error){r.error=error;r.e.emu_stop();}},null,base+offset,base+offset);
  hook(0x24fb0,()=>this.totals.fill(0));
  hook(0x25064,()=>{const p=r.r(5),pool=(p-g-0x2ed6)/20;if(!Number.isInteger(pool)||pool<0||pool>=200||!r.read(g+0x2e0c+pool,1)[0])return;
   const slot=r.get16(g+0x4886),bytes=r.read(p,20),type=r.get16(p)>>>5&7,sp=r.e.reg_read_i32(r.uc.ARM_REG_SP)>>>0,idx=r.get16(r.get32(sp)),value=r.get16(r.r(11)),level=r.features0130.refinement.level(slot,pool,bytes);
   if(type<=2&&idx===7)this.totals[idx]+=weaponAttack(value,level)-value;
   if(type>=3&&type<=6&&idx===8)this.totals[idx]+=armorDefense(value,level)-value;
   if(type>=3&&type<=6&&idx===5)this.totals[idx]+=armorHealth(value,level)-value;
  });
  hook(0x25392,()=>{const slot=r.get16(g+0x4886),f=r.features0130,title=!!(f.arena.ledger.slots[slot]?.titleUnlocked&&f.masterEquipped[slot]);for(let idx=0;idx<13;idx++){if(idx===4)continue;let value=r.get16(g+0x48ba+idx*2)+this.totals[idx];const unbuffed=value;if(title)value+=Math.floor(value*120/100);value=boundedStat(value);if(title&&idx<4){const delta=r.get16(g+0x48d4+idx*2)+value-unbuffed;r.write(g+0x48d4+idx*2,Uint8Array.of(delta&255,delta>>>8));}r.write(g+0x48ba+idx*2,Uint8Array.of(value&255,value>>>8));}});
  hook(0x133b2,()=>{const p=r.r(6),count=r.get32(g+0x1f9c),slot=r.get16(g+0x4886);if(p<g+0x14fc||p>=g+0x14fc+20*136||(p-g-0x14fc)%136||count>20||(p-g-0x14fc)/136>=count||r.get16(p+8)>109||r.get16(p+0xa)>=200)return;const f=r.features0130,combat=f.combat;combat.master=!!(f.arena.ledger.slots[slot]?.titleUnlocked&&f.masterEquipped[slot]);if(!combat.enabled&&!combat.master&&!combat.elixirMs&&!combat.immuneMs)return;this.random^=this.random<<13;this.random^=this.random>>>17;this.random^=this.random<<5;this.random>>>=0;const damage=combat.hit(r.get16(g+0x48f4),r.get16(g+0x48c4),r.r(8),this.random%10000);r.w(8,damage);});
 }
}
