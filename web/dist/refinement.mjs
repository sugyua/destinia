import {weaponAttack,stoneForTarget} from './refinement-policy.mjs';
// Pure sidecar model. Original equipment bytes and game saves are never mutated.
export const MaxWallet=2147483647;
export const Kind=Object.freeze({Weapon:0,Armor:1,Other:2});
export const Error=Object.freeze({None:0,InvalidIdentity:1,InvalidKind:2,MaxLevel:3,InsufficientGold:4,InsufficientCrystals:5});
const valid=(slot,pool)=>Number.isInteger(slot)&&slot>=0&&slot<10&&Number.isInteger(pool)&&pool>=0&&pool<200;
const fingerprint=bytes=>{if(!bytes||bytes.length!==20||!Array.from(bytes).every(b=>Number.isInteger(b)&&b>=0&&b<=255))throw new RangeError('INVALID FINGERPRINT');return Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');};
export function tag(level){if(!Number.isInteger(level)||level<0||level>15)throw new RangeError('INVALID LEVEL');return level?`[+${level}]`:'';}
export const scaleRelevant=weaponAttack;
export class Store {
 #records=new Map();#wallets=Array.from({length:10},()=>[0,0,0]);
 #peek(slot,pool,bytes){const hex=fingerprint(bytes),r=this.#records.get(slot*200+pool);return valid(slot,pool)&&r?.hex===hex?r.level:0;}
 // Observation retires replaced items; quotes and rejected upgrades remain pure.
 level(slot,pool,bytes){const hex=fingerprint(bytes);if(!valid(slot,pool))return 0;const key=slot*200+pool,r=this.#records.get(key);if(r&&r.hex!==hex){this.#records.delete(key);return 0;}return r?.level??0;}
 crystals(slot,stone=0){return Number.isInteger(slot)&&slot>=0&&slot<10&&Number.isInteger(stone)&&stone>=0&&stone<3?this.#wallets[slot][stone]:0;}
 addCrystals(slot,amount,stone=0){if(!Number.isInteger(slot)||slot<0||slot>=10||!Number.isSafeInteger(amount)||amount<0||!Number.isInteger(stone)||stone<0||stone>2||amount>MaxWallet-this.#wallets[slot][stone])return false;this.#wallets[slot][stone]+=amount;return true;}
 quote(slot,pool,bytes,kind){const level=this.#peek(slot,pool,bytes);const r={error:Error.None,level,goldCost:0,crystalsCost:0,stone:0};if(!valid(slot,pool))r.error=Error.InvalidIdentity;else if(kind!==Kind.Weapon&&kind!==Kind.Armor)r.error=Error.InvalidKind;else if(level===15)r.error=Error.MaxLevel;else{r.goldCost=500*(level+1)**2;r.crystalsCost=level+1;r.stone=stoneForTarget(level+1);}return r;}
 upgrade(slot,pool,bytes,kind,wallet){const r=this.quote(slot,pool,bytes,kind);if(r.error)return r;if(!wallet||!Number.isSafeInteger(wallet.gold)||wallet.gold<r.goldCost){r.error=Error.InsufficientGold;return r;}if(this.#wallets[slot][r.stone]<r.crystalsCost){r.error=Error.InsufficientCrystals;return r;}this.#records.set(slot*200+pool,{hex:fingerprint(bytes),level:r.level+1});wallet.gold-=r.goldCost;this.#wallets[slot][r.stone]-=r.crystalsCost;r.level++;return r;}
 encode(){let out='DESTINIA_REFINEMENT_V1\n';for(let i=0;i<10;i++)out+=`W ${i} ${this.#wallets[i].join(" ")}\n`;for(const [key,r] of [...this.#records].sort((a,b)=>a[0]-b[0]))out+=`R ${Math.floor(key/200)} ${key%200} ${r.hex} ${r.level}\n`;return out;}
 decode(text){
  if(typeof text!=='string'||text.length>120000||!text.endsWith('\n'))throw new RangeError('INVALID REFINEMENT');
  const lines=text.split('\n');if(lines.shift()!=='DESTINIA_REFINEMENT_V1')throw new RangeError('INVALID REFINEMENT');lines.pop();const next=new Store();
  for(let slot=0;slot<10;slot++){const m=/^W ([0-9]) (0|[1-9][0-9]*) (0|[1-9][0-9]*) (0|[1-9][0-9]*)$/.exec(lines.shift()??'');if(!m||Number(m[1])!==slot||![0,1,2].every(t=>next.addCrystals(slot,Number(m[2+t]),t)))throw new RangeError('INVALID WALLET');}
  let previous=-1;for(const line of lines){const m=/^R ([0-9]) (0|[1-9][0-9]*) ([0-9a-f]{40}) ([1-9]|1[0-5])$/.exec(line);if(!m)throw new RangeError('INVALID RECORD');const slot=Number(m[1]),pool=Number(m[2]),key=slot*200+pool;if(!valid(slot,pool)||key<=previous)throw new RangeError('INVALID RECORD');next.#records.set(key,{hex:m[3],level:Number(m[4])});previous=key;}
  if(next.encode()!==text)throw new RangeError('NONCANONICAL REFINEMENT');this.#wallets=next.#wallets;this.#records=next.#records;
 }
}
