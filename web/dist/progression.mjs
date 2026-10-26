// Canonical progression: no engine access, native save access, or automatic hooks.
export const MAX_LEVEL=999, NATIVE_MAX_LEVEL=99, BASIS=10000, U64_MAX=(1n<<64n)-1n;
export const ARM_FIELDS=Object.freeze({level:0x48fa,xp:0x48fc,stats:0x48ba});
export const SIDECAR_NAME='progression-v1.txt';
export const Rank=Object.freeze({Aprendiz:0,Aventureiro:1,Guerreiro:2,Cavaleiro:3,Campeao:4,Mestre:5,GraoMestre:6,Lenda:7,Mitico:8,Semideus:9,Divino:10,Transcendente:11,Celestial:12,Supremo:13});
export const Difficulty=Object.freeze({Normal:0,Dificil:1,Pesadelo:2,Inferno:3,Abissal:4});
const names=['Aprendiz','Aventureiro','Guerreiro','Cavaleiro','Campeão','Mestre','Grão-Mestre','Lenda','Mítico','Semideus','Divino','Transcendente','Celestial','Supremo'];
function require(condition,message){if(!condition)throw new RangeError(message);}
function u32(value){require(Number.isInteger(value)&&value>=0&&value<=0xffffffff,'invalid uint32');return value;}
function u64(value){require(typeof value==='bigint'&&value>=0n&&value<=U64_MAX,'expected uint64 BigInt');return value;}
export function checkedAdd(a,b){u64(a);u64(b);return u64(a+b);}
function checkedMul(a,b){u64(a);u64(b);return u64(a*b);}
function level(value){u32(value);require(value>=1&&value<=MAX_LEVEL,'level outside 1..999');return value;}
export function rankForLevel(value){level(value);return value<=20?0:value<=40?1:value<=60?2:value<=80?3:value<=99?4:5+Math.floor((value-100)/100);}
export function rankName(rank){u32(rank);require(rank<14,'invalid rank');return names[rank];}
export function xpForNextLevel(value){level(value);if(value===MAX_LEVEL)return 0n;const n=BigInt(value),adjusted=n+2n+n*n/10n;return adjusted*adjusted+20n;}
const xpTable=Array(MAX_LEVEL+1).fill(0n);for(let n=2;n<=MAX_LEVEL;++n)xpTable[n]=checkedAdd(xpTable[n-1],xpForNextLevel(n-1));
export function xpToReachLevel(value){return xpTable[level(value)];}
export function query(totalXp){
 u64(totalXp);require(totalXp<=xpTable[MAX_LEVEL],'XP exceeds level cap');let low=1,high=MAX_LEVEL;
 while(low<high){const middle=Math.floor((low+high+1)/2);if(xpTable[middle]<=totalXp)low=middle;else high=middle-1;}
 return {level:low,rank:rankForLevel(low),totalXp,currentXp:totalXp-xpTable[low],requiredXp:xpForNextLevel(low)};
}
export function awardXp(totalXp,amount){query(totalXp);u64(amount);const remaining=xpTable[MAX_LEVEL]-totalXp,accepted=amount<remaining?amount:remaining;return {progress:query(totalXp+accepted),accepted,discarded:amount-accepted};}
export function scale(value,basisPoints){u64(value);u32(basisPoints);return u64(value*BigInt(basisPoints)/10000n);}
export function statAtLevel(value,{base=0,linear=0,quadratic=0,divisor=1}={}){
 level(value);[base,linear,quadratic,divisor].forEach(u32);require(divisor!==0,'zero stat divisor');const n=BigInt(value-1);
 const result=checkedAdd(checkedAdd(BigInt(base),checkedMul(BigInt(linear),n)),checkedMul(BigInt(quadratic),n*n)/BigInt(divisor));require(result<=0xffffffffn,'stat uint32 overflow');return Number(result);
}
const fields=['hp','attack','xp','gold'];
export function defaultRules(){return {profiles:[[10000,10000],[15000,13000],[25000,18000],[40000,25000],[60000,40000]].map(([hp,xp])=>({hp,attack:hp,xp,gold:xp})),ngGrowth:{hp:2500,attack:2500,xp:1000,gold:1000}};}
function validateRules(rules){
 require(rules&&Array.isArray(rules.profiles)&&rules.profiles.length===5&&rules.ngGrowth,'invalid rules');
 for(const m of rules.profiles){require(m,'invalid profile');for(const f of fields)require(u32(m[f])>0,'zero difficulty multiplier');}
 for(const f of fields){require(rules.profiles[0][f]===BASIS,'Normal must remain 1x');u32(rules.ngGrowth[f]);}
}
export function multipliers(difficulty,ngCycle,rules=defaultRules()){
 validateRules(rules);u32(difficulty);require(difficulty<5,'invalid difficulty');u32(ngCycle);const result={};
 for(const f of fields){const factor=checkedAdd(10000n,checkedMul(BigInt(ngCycle),BigInt(rules.ngGrowth[f]))),value=scale(factor,rules.profiles[difficulty][f]);require(value<=0xffffffffn,'multiplier uint32 overflow');result[f]=Number(value);}return result;
}
export function checkedArmU16(value){u32(value);require(value<=65535,'native uint16 truncation');return value;}
export function checkedArmStat(value){u32(value);require(value<=32767,'native signed stat truncation');return value;}
export function checkedArmXp(value){u64(value);require(value<=0xffffffffn,'native XP truncation');return Number(value);}
export function nativeLevelCompatible(value){return Number.isInteger(value)&&value>=1&&value<=NATIVE_MAX_LEVEL;}
export function checkedArmLevel(value){require(nativeLevelCompatible(value),'native engine still capped at 99');return checkedArmU16(value);}
export function importNative(value,currentXp){require(nativeLevelCompatible(value),'unsupported native level');u64(currentXp);require(value===NATIVE_MAX_LEVEL?currentXp===0n:currentXp<xpForNextLevel(value),'invalid native current XP');return xpToReachLevel(value)+currentXp;}
function modeName(mode){require(mode==='classic'||mode==='expanded','invalid mode');return mode;}
function validate(book){
 modeName(book.mode);validateRules(book.rules);require(Array.isArray(book.characters)&&book.characters.length<=10,'invalid characters');const slots=new Set();
 for(const c of book.characters){u32(c.slot);require(c.slot<10&&!slots.has(c.slot),'invalid or duplicate slot');slots.add(c.slot);query(c.totalXp);multipliers(c.difficulty,c.ngCycle,book.rules);}
}
export function encode(book){
 validate(book);const values=m=>fields.map(f=>m[f]).join(' ');
 const lines=['DESTINIA_PROGRESSION_V1',`mode ${book.mode}`,'curve legacy-extended-v1',`ng ${values(book.rules.ngGrowth)}`];
 book.rules.profiles.forEach((m,i)=>lines.push(`profile ${i} ${values(m)}`));lines.push(`characters ${book.characters.length}`);
 for(const c of [...book.characters].sort((a,b)=>a.slot-b.slot))lines.push(`character ${c.slot} ${c.totalXp} ${c.ngCycle} ${c.difficulty}`);lines.push('end');return lines.join('\n')+'\n';
}
export function decode(text,expectedMode){
 modeName(expectedMode);require(typeof text==='string'&&text.length<=4096&&/^[\x20-\x7e\t\r\n]*$/.test(text),'invalid sidecar text');
 const tokens=text.trim().split(/[ \t\r\n]+/);let index=0;const token=()=>{require(index<tokens.length,'truncated sidecar');return tokens[index++];};
 const expect=value=>require(token()===value,'invalid sidecar schema');
 const number=()=>{const value=token();require(/^(0|[1-9][0-9]*)$/.test(value),'invalid decimal');return u64(BigInt(value));};
 const small=()=>{const value=number();require(value<=0xffffffffn,'uint32 overflow');return Number(value);};
 const read=()=>Object.fromEntries(fields.map(f=>[f,small()]));
 expect('DESTINIA_PROGRESSION_V1');expect('mode');expect(expectedMode);expect('curve');expect('legacy-extended-v1');expect('ng');
 const rules={ngGrowth:read(),profiles:[]};for(let i=0;i<5;++i){expect('profile');require(small()===i,'profile order');rules.profiles.push(read());}
 expect('characters');const count=small();require(count<=10,'too many characters');const characters=[];
 for(let i=0;i<count;++i){expect('character');characters.push({slot:small(),totalXp:number(),ngCycle:small(),difficulty:small()});}
 expect('end');require(index===tokens.length,'trailing sidecar data');const book={mode:expectedMode,rules,characters};validate(book);return book;
}
// Inject Window Storage or another synchronous adapter. Web Locks are mandatory
// for commits: callers must not acknowledge persistence before this promise resolves.
export function createSidecarStore({storage,mode,locks}){
 modeName(mode);require(storage&&typeof storage.getItem==='function'&&typeof storage.setItem==='function','missing storage');
 const key=`destinia.progression-v1.${mode}`;
 return {key,load(){const text=storage.getItem(key);return text===null?null:{text,ledger:decode(text,mode)};},async commit(book,expected=null){
  require(book.mode===mode,'mode mismatch');const text=encode(book);require(locks&&typeof locks.request==='function','Web Locks required for progression commits');
  await locks.request(key,()=>{const previous=storage.getItem(key);if(previous!==null)decode(previous,mode);require(previous===expected,'stale progression snapshot');storage.setItem(key,text);require(storage.getItem(key)===text,'progression write not acknowledged');});return text;
 }};
}
