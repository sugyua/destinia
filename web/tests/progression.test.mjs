import test from 'node:test';
import assert from 'node:assert/strict';
import * as p from '../dist/progression.mjs';
import {execFileSync} from 'node:child_process';
test('canonical XP boundaries, ranks and capped awards',()=>{
 assert.equal(p.xpForNextLevel(1),29n);assert.equal(p.xpForNextLevel(99),1168581n);
 const starts=[1,21,41,61,81,100,200,300,400,500,600,700,800,900];
 starts.forEach((start,i)=>{assert.equal(p.rankForLevel(start),i);assert.equal(p.rankForLevel(i===13?999:starts[i+1]-1),i);});
 let total=0n;for(let level=1;level<=999;++level){assert.equal(p.xpToReachLevel(level),total);assert.equal(p.query(total).level,level);if(level<999){assert.equal(p.query(total+p.xpForNextLevel(level)-1n).level,level);total+=p.xpForNextLevel(level);}}
 assert(total>0xffffffffn);const gain=p.awardXp(0n,p.U64_MAX);assert.equal(gain.progress.level,999);assert.equal(gain.accepted,total);assert.equal(gain.discarded,p.U64_MAX-total);
 for(const invalid of [0,1000,1.5,NaN])assert.throws(()=>p.rankForLevel(invalid));assert.throws(()=>p.query(p.U64_MAX));assert.throws(()=>p.awardXp(0n,9007199254740992));
});
test('integer multipliers, status and native narrowing guards',()=>{
 assert.equal(p.scale(p.U64_MAX,10000),p.U64_MAX);assert.equal(p.scale(p.U64_MAX,1),p.U64_MAX/10000n);assert.throws(()=>p.scale(p.U64_MAX,10001));
 assert.equal(p.statAtLevel(999,{base:100,linear:3,quadratic:1,divisor:10}),102694);assert.throws(()=>p.statAtLevel(999,{base:0xffffffff,linear:1,quadratic:0,divisor:1}));
 const rules=p.defaultRules();assert.deepEqual(rules.profiles.map(m=>m.hp),[10000,15000,25000,40000,60000]);assert.deepEqual(rules.profiles.map(m=>m.xp),[10000,13000,18000,25000,40000]);
 assert.deepEqual(p.multipliers(p.Difficulty.Normal,0,rules),{hp:10000,attack:10000,xp:10000,gold:10000});
 rules.ngGrowth={hp:2500,attack:2500,xp:1000,gold:1000};assert.equal(p.multipliers(p.Difficulty.Dificil,2,rules).hp,22500);assert.equal(p.multipliers(p.Difficulty.Dificil,2,rules).xp,15600);
 assert.throws(()=>p.multipliers(5,0,rules));assert.throws(()=>p.multipliers(4,0xffffffff,rules));
 assert.equal(p.checkedArmU16(65535),65535);assert.throws(()=>p.checkedArmU16(65536));assert.throws(()=>p.checkedArmStat(32768));assert.throws(()=>p.checkedArmXp(0x100000000n));assert.equal(p.nativeLevelCompatible(100),false);
 assert.equal(p.importNative(2,1n),p.xpToReachLevel(2)+1n);assert.throws(()=>p.importNative(99,1n));
});
test('versioned sidecar codec: exact decimal integers, strict schema, mode isolation',()=>{
 const book={mode:'expanded',rules:p.defaultRules(),characters:[{slot:3,totalXp:p.xpToReachLevel(999),ngCycle:2,difficulty:4}]};
 const text=p.encode(book);assert.equal(p.encode(p.decode(text,'expanded')),text);
 assert.throws(()=>p.decode(text,'classic'));assert.throws(()=>p.decode(text+'garbage\n','expanded'));assert.throws(()=>p.decode(text.slice(0,-4),'expanded'));
 assert.throws(()=>p.encode({...book,characters:[book.characters[0],book.characters[0]]}));
 assert.throws(()=>p.decode(text.replace(/character 3 /,'character -1 '),'expanded'));
});
test('storage commits require lock, reject stale snapshots and propagate quota errors',async()=>{
 const data=new Map(),storage={getItem:k=>data.get(k)??null,setItem:(k,v)=>data.set(k,v)};
 let queue=Promise.resolve();const locks={request(_key,fn){const result=queue.then(fn);queue=result.catch(()=>{});return result;}};
 const book={mode:'expanded',rules:p.defaultRules(),characters:[]},store=p.createSidecarStore({storage,mode:'expanded',locks});
 assert.equal(store.load(),null);const text=await store.commit(book);assert.equal(store.load().text,text);
 await assert.rejects(store.commit(book),/stale/);
 const updated={...book,characters:[{slot:0,totalXp:29n,ngCycle:1,difficulty:1}]};
 const results=await Promise.allSettled([store.commit(updated,text),store.commit(book,text)]);assert.deepEqual(results.map(r=>r.status),['fulfilled','rejected']);
 const before=store.load().text;storage.setItem=()=>{throw Error('quota');};await assert.rejects(store.commit(book,before),/quota/);assert.equal(store.load().text,before);
 await assert.rejects(p.createSidecarStore({storage,mode:'expanded'}).commit(book,before),/Web Locks/);
 assert.equal(p.createSidecarStore({storage,mode:'classic',locks}).load(),null);
 data.set(store.key,'corrupt');assert.throws(()=>store.load());await assert.rejects(store.commit(book,'corrupt'));assert.equal(data.get(store.key),'corrupt');
});
test('reject malformed sidecars without permissive integer coercion',()=>{
 const text=p.encode({mode:'expanded',rules:p.defaultRules(),characters:[{slot:0,totalXp:0n,ngCycle:0,difficulty:0}]});
 for(const invalid of [text.replace('character 0 0 0 0','character 0 18446744073709551616 0 0'),text.replace('characters 1','characters 11'),text.replace('character 0','character 00'),text.replace('profile 1','profile 2'),text.replace('ng 2500','ng -1'),text+'\0',text.repeat(100)])assert.throws(()=>p.decode(invalid,'expanded'));
 assert.throws(()=>p.checkedArmLevel(999));
});
test('C++ and web match all levels, profiles, large integer scaling and serialized bytes',{skip:!process.env.PROGRESSION_CPP},()=>{
 const lines=[];for(let n=1;n<=999;++n)lines.push(`${n} ${p.rankForLevel(n)} ${p.xpForNextLevel(n)} ${p.xpToReachLevel(n)} ${p.statAtLevel(n,{base:100,linear:3,quadratic:1,divisor:10})}`);
 for(let d=0;d<5;++d)for(const cycle of [0,1,2,1000]){const m=p.multipliers(d,cycle);lines.push(`${d} ${cycle} ${m.hp} ${m.attack} ${m.xp} ${m.gold}`);}
 for(const value of [0n,1n,0xffffffffn,p.U64_MAX])for(const bps of [0,1,9999,10000])lines.push(`${value} ${bps} ${p.scale(value,bps)}`);
 const book={mode:'expanded',rules:p.defaultRules(),characters:[{slot:3,totalXp:p.xpToReachLevel(999),ngCycle:2,difficulty:4}]};
 const expected=lines.join('\n')+'\n'+p.encode(book),actual=execFileSync(process.env.PROGRESSION_CPP,['--parity'],{encoding:'utf8'}).replaceAll('\r\n','\n');assert.equal(actual,expected);
});
