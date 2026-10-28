import test from 'node:test';
import assert from 'node:assert/strict';
import {Features0130} from '../dist/features-0130.mjs';
import {BossRushUI} from '../dist/boss-rush.mjs';
test('failed or incomplete spawn aborts with no rewards',()=>{
 const f=new Features0130({},{spawnWave:()=>[{instance:1,species:1}]});
 assert.equal(f.start(0,4),false);assert.equal(f.arena.active,false);assert.equal(f.arena.ledger.slots[0].pendingGold,0);
});
test('registered kills advance once; context changes abort',()=>{
 let next=0;const f=new Features0130({},{spawnWave:w=>Array.from({length:w.count},()=>({instance:++next,species:1}))});
 assert.equal(f.start(0,4),true);const run=f.arena.runId;
 assert.equal(f.kill(run,1,4,0),true);assert.equal(f.kill(run,1,4,0),false);
 for(let p=2;p<=4;p++)f.kill(run,p,4,0);
 assert.equal(f.arena.ledger.slots[0].pendingGold,500);assert.equal(f.next(),true);assert.equal(f.arena.currentWave,2);
 f.observeContext(1,4,true);assert.equal(f.arena.active,false);
});
test('sidecars survive reload but live run never resumes',()=>{
 const f=new Features0130({});f.refinement.addCrystals(0,8);f.arena.ledger.slots[0].pendingGold=42;
 const g=new Features0130({}, {}, f.snapshot());assert.equal(g.refinement.crystals(0),8);assert.equal(g.arena.ledger.slots[0].pendingGold,42);assert.equal(g.arena.active,false);
});
test('active time ignores paused frames and records survive abort/reload',()=>{
 let next=0;const f=new Features0130({},{spawnWave:w=>Array.from({length:w.count},()=>({instance:++next,species:1}))});
 assert.equal(f.start(0,1),true);f.tick(60000,true);assert.equal(f.arena.ledger.slots[0].bestTimeSeconds,0);f.tick(2500,false);assert.equal(f.arena.ledger.slots[0].bestTimeSeconds,2);
 const run=f.arena.runId,map=f.arena.arenaMap;for(let i=1;i<=4;i++)assert.equal(f.kill(run,i,map,0),true);f.abort();const g=new Features0130({}, {},f.snapshot());assert.equal(g.arena.ledger.slots[0].totalKills,4);assert.equal(g.arena.ledger.slots[0].bestTimeSeconds,2);
});

import {Arena,wave,encode,decode} from '../dist/survival.mjs';
import {Store,Kind,Error as RefineError,tag,scaleRelevant} from '../dist/refinement.mjs';
test('30 waves, six bosses, replay rejection, exact title and reward ledger',()=>{
 const a=new Arena();assert.equal(a.start(0,1,7),true);let id=0,gold=0,crystals=0;
 for(let n=1;n<=30;n++){
  const w=wave(n);assert.equal(w.count,Math.min(12,3+n));assert.equal(w.hpBps,10000+(n-1)*1500);assert.equal(w.attackBps,10000+(n-1)*1000);
  const ids=[];for(let j=0;j<w.count;j++){ids.push(++id);assert.equal(a.registerMonster(id,j===0&&w.bossSpecies?w.bossSpecies:1),true);}
  assert.equal(a.beginNextWave(),false);assert.equal(a.observeKill(999,ids[0],7,0),false);assert.equal(a.observeKill(1,ids[0],8,0),false);assert.equal(a.observeKill(1,ids[0],7,1),false);
  for(const p of ids)assert.equal(a.observeKill(1,p,7,0),true);assert.equal(a.observeKill(1,ids[0],7,0),false);
  gold+=w.gold;crystals+=n<=10?w.crystals:0;if(n<30)assert.equal(a.beginNextWave(),true);
 }
 assert.equal(a.active,true);assert.equal(a.ledger.slots[0].titleUnlocked,true);assert.equal(a.ledger.slots[0].completions,0);assert.equal(a.ledger.slots[0].totalKills,324);assert.ok(a.ledger.slots[0].survivalScore>0);
 const reward=a.takePending(0);assert.equal(reward.gold,gold);assert.equal(reward.crystals,crystals);assert.ok(reward.pure>0&&reward.abyssal>0&&reward.cash>0);assert.deepEqual(a.takePending(0),{gold:0,crystals:0,cash:0,pure:0,abyssal:0});assert.equal(a.start(0,1,7),false);
 assert.equal(encode(decode(encode(a.ledger))),encode(a.ledger));assert.throws(()=>decode(encode(a.ledger)+'x'));
});
test('refinement prices, supported kinds, wallet atomicity, identity and strict codec',()=>{
 const s=new Store(),bytes=Uint8Array.from({length:20},(_,i)=>i),wallet={gold:10000000};for(let stone=0;stone<3;stone++)s.addCrystals(0,1000,stone);
 for(let n=1;n<=15;n++){const q=s.quote(0,2,bytes,Kind.Weapon);assert.equal(q.goldCost,500*n*n);assert.equal(q.crystalsCost,n);const old=wallet.gold;assert.equal(s.upgrade(0,2,bytes,Kind.Weapon,wallet).error,RefineError.None);assert.equal(wallet.gold,old-500*n*n);assert.equal(s.level(0,2,bytes),n);}
 const old=wallet.gold;assert.equal(s.upgrade(0,2,bytes,Kind.Weapon,wallet).error,RefineError.MaxLevel);assert.equal(wallet.gold,old);
 assert.equal(tag(10),'[+10]');assert.equal(scaleRelevant(100,10),180);assert.equal(s.level(1,2,bytes),0);
 const t=new Store();t.decode(s.encode());assert.equal(t.encode(),s.encode());assert.throws(()=>t.decode(s.encode()+'x'));assert.equal(t.level(0,2,bytes),15);
 const changed=bytes.slice();changed[0]++;assert.equal(t.level(0,2,changed),0);assert.equal(t.level(0,2,bytes),0);
 const poor={gold:0};assert.equal(t.upgrade(0,3,bytes,Kind.Armor,poor).error,RefineError.InsufficientGold);assert.equal(poor.gold,0);assert.equal(t.upgrade(0,3,bytes,Kind.Other,wallet).error,RefineError.InvalidKind);
});
test('F9 panel consumes modal input and draws readable framebuffer pixels',()=>{
 const f=new Features0130({width:800,height:480});assert.equal(f.handleKey('F9',true),true);assert.equal(f.open,true);assert.equal(f.handleKey(17,true),true);
  const pixels=new Uint8Array(800*480*2);f.paint(pixels);assert.ok(pixels.some(x=>x!==0));assert.equal(f.handleTouch(f.layout().x+20,f.layout().y+44+7*f.layout().row+2,0),true);assert.equal(f.open,false);
});
import {Features0130Stats,refinementBonus,boundedStat} from '../dist/features0130-stats.mjs';
import {findTitle,TitleId} from '../dist/titles.mjs';
test('stat hook continuation scales item attributes once and selected title after base rebuild',()=>{
 const g=0x10077ee0,p=g+0x2ed6,sp=0x7f000000,idxPtr=0x20002000,valPtr=0x20003000,hooks=new Map(),words=new Map(),memory=new Map();
 let idx=7;const bytes=new Uint8Array(20);bytes[4]=5;bytes[5]=40;let type=0;
 const r={exports:new Map([['Game',g]]),uc:{HOOK_CODE:1,ARM_REG_SP:13},e:{hook_add:(t,fn,a,b)=>hooks.set(b,fn),reg_read_i32:()=>sp},r:n=>n===5?p:valPtr,get32:()=>idxPtr,get16:addr=>addr===g+0x4886?0:addr===p?type<<5:addr===idxPtr?idx:addr===valPtr?100:words.get(addr)||0,read:(addr,n)=>addr===p?bytes:Uint8Array.of(1),write:(addr,b)=>words.set(addr,b[0]|b[1]<<8),features0130:{refinement:{level:()=>10},arena:{ledger:{slots:[{titleUnlocked:true}]}},masterEquipped:[true]}};
 const s=new Features0130Stats(r);s.install();hooks.get(0x10024fb0)();hooks.get(0x10025064)();assert.equal(s.totals[7],80);
 type=3;idx=8;hooks.get(0x10025064)();assert.equal(s.totals[8],60);assert.equal(s.totals[5],0);
 for(const i of [5,6,7,8])words.set(g+0x48ba+2*i,100);hooks.get(0x10025392)();assert.equal(words.get(g+0x48ba+14),396);assert.equal(words.get(g+0x48ba+10),220);
 hooks.get(0x10024fb0)();assert.deepEqual(s.totals,Array(13).fill(0));assert.equal(boundedStat(100000),32767);assert.equal(refinementBonus(99,1),7);assert.equal(findTitle(TitleId.MestreDaSobrevivencia).atk,120);
});
test('pending reward is cleared only after backend confirms credit; title equip is explicit and per slot',()=>{
 let credit=false;const f=new Features0130({},{context:()=>({slot:0,map:1}),creditRewards:()=>credit});f.arena.ledger.slots[0].pendingGold=500;f.arena.ledger.slots[0].pendingCrystals=1;f.settle();assert.equal(f.arena.ledger.slots[0].pendingGold,500);credit=true;f.settle();assert.equal(f.arena.ledger.slots[0].pendingGold,0);
 assert.equal(f.equipMaster(0),false);Object.assign(f.arena.ledger.slots[0],{bestWave:20,completions:1,titleUnlocked:true});assert.equal(f.masterEquipped[0],false);assert.equal(f.equipMaster(0),true);const g=new Features0130({}, {},f.snapshot());assert.equal(g.masterEquipped[0],true);assert.equal(g.masterEquipped[1],false);
});

test('active arena credits rewards automatically and rejects refining',()=>{let claims=0;const f=new Features0130({}, {creditRewards:()=>{claims++;return true;},context:()=>({slot:0,map:1})});f.arena.active=true;f.arena.ledger.slots[0].pendingGold=500;f.settle();assert.equal(claims,1);assert.equal(f.upgradeSelected(),false);f.arena.active=false;f.settle();assert.equal(claims,1);});
test('touch selects both tabs in the second row',()=>{const ui=new BossRushUI({width:800,height:480});ui.openMenu();const pw=360,px=220,py=130,tabW=Math.floor((pw-16)/3);ui.handleTouch(px+8+tabW+5,py+44,0);assert.equal(ui.currentTab,4);ui.handleTouch(px+8+tabW*2+5,py+44,0);assert.equal(ui.currentTab,5);});
test('feature sidecar marks restored native saves as authoritative over cloud copies',()=>{const features=new Features0130({}),payload=JSON.stringify({version:1,mode:'classico',features:features.snapshot(),saves:[['./save0.dat',[9,8]]]});const engine={gameMode:'classico',saved:new Map(),log(){}};const backend=new NativeFeatures0130Backend(engine,{storage:{getItem:()=>payload}});backend.attach(features);assert.deepEqual([...engine.saved.get('./save0.dat')],[9,8]);assert.equal(engine.preservedFeatureSaveNames.has('save0.dat'),true);});
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {createRequire} from 'node:module';
import {DestiniaWeb} from '../dist/engine.mjs';
import {NativeFeatures0130Backend} from '../dist/features0130-native.mjs';
test('original ARM feature pipeline spawns registered arena, defers checkpoint saves and suppresses quest callbacks',async()=>{
 const root=process.env.DESTINIA_GAME_ROOT||(fs.existsSync('game/assets')?'game':'../game');if(!fs.existsSync(root+'/assets'))return;
 const temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-feature-test-'));
 let r;try{fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));const factory=createRequire(import.meta.url)(path.join(temp,'unicorn.cjs')),uc=await factory();const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
 r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});const storage=new Map(),backend=new NativeFeatures0130Backend(r,{storage:{getItem:k=>storage.get(k)??null,setItem:(k,v)=>storage.set(k,v)}}),freed=[];const call=backend.call.bind(backend);backend.call=(name,args)=>{if(name==='Monster_freeMonster')freed.push(args[0]);return call(name,args);};r.featureBackendOption=backend;r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);const g=r.exports.get('Game'),put16=(a,v)=>r.write(a,Uint8Array.of(v&255,v>>>8));put16(g+0x48f4,100);put16(g+0x48fa,10);put16(g+0x4886,0);
 const context=backend.context();assert.equal(r.features0130.start(0,context.map),true);assert.equal(r.features0130.arena.members.size,4);freed.length=0;
 const savedBefore=Array.from(r.saved,([k,v])=>[k,Array.from(v)]),first=g+0x14fc;r.put32(first+0x20,0);r.call(r.exports.get('checkQuestMonster'),[first]);assert.equal(r.features0130.arena.remaining,3,JSON.stringify({context:backend.context(),slot:r.features0130.arena.slot,map:r.features0130.arena.arenaMap,instance:backend.instanceId(first),species:r.get16(first+8),hp:r.get32(first+0x20),active:r.features0130.arena.active}));r.call(r.exports.get('checkQuestMonster'),[first]);assert.equal(r.features0130.arena.remaining,3);
 r.features0130.changed();await backend.flush();assert.deepEqual(Array.from(r.saved,([k,v])=>[k,Array.from(v)]),savedBefore);assert.ok(storage.size);r.features0130.abort();assert.equal(r.features0130.arena.active,false);assert.deepEqual(freed,Array.from({length:21},(_,i)=>i),'arena monsters must be freed before restoring the campaign map');
 }finally{r?.close();fs.rmSync(temp,{recursive:true,force:true});}
});
