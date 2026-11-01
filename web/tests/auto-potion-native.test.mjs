import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';import os from 'node:os';import path from 'node:path';import {createRequire} from 'node:module';
import {DestiniaWeb} from '../dist/engine.mjs';
const require=createRequire(import.meta.url),temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-auto-potion-'));
fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));
const uc=await require(path.join(temp,'unicorn.cjs'))();
process.on('exit',()=>fs.rmSync(temp,{recursive:true,force:true}));
const root=process.env.DESTINIA_GAME_ROOT||(fs.existsSync('game/assets')?'game':'../game');
const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
const u16=v=>Uint8Array.of(v&255,v>>>8),u32=v=>Uint8Array.of(v&255,(v>>>8)&255,(v>>>16)&255,(v>>>24)&255);
test('auto potion uses original ARM quickslot and preserves the assigned slot',()=>{
 for(const type of [0,1]){
  const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});
  try{
   r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
   const g=r.exports.get('Game'),id=type?5:0,vals=[[0x4a8c,0],[0x4a90,0x6123],[0x433c,1],[0x433e,1],[0x4340,id],[0x4342,3],[0x4854,0],[0x48f2,type?10:189],[0x48f4,type?1000:500],[0x48f6,189],[0x48f8,1000]];
   for(const [a,v] of vals)r.write(g+a,u16(v));r.write(g+0x7cd4,u32(1000));
   const hp=r.get16(g+0x48f4),mp=r.get16(g+0x48f2);
   assert.equal(r.autoPotion.update(),true);assert.equal(r.get16(g+0x4342),2);assert.equal(r.get16(g+0x4a90),0x6123);
   assert.equal(r.get16(g+0x48f4),hp);assert.equal(r.get16(g+0x48f2),mp);assert.equal(r.get16(g+(type?0x7cea:0x7ce8)),type?5:50);
   assert.equal(r.autoPotion.update(),false);assert.equal(r.get16(g+0x4342),2);
   r.autoPotion.cooldownHpTimer=0;r.autoPotion.cooldownMpTimer=0;
   r.write(g+0x4340,u16(15));const prior=r.get16(g+0x4342),slot=r.get16(g+0x4a90);
   assert.equal(r.autoPotion.update(),false);assert.equal(r.get16(g+0x4342),prior);assert.equal(r.get16(g+0x4a90),slot);
   r.write(g+0x4340,u16(id));r.write(g+0x4342,u16(0));
   assert.equal(r.autoPotion.update(),false);assert.equal(r.get16(g+0x4342),0);assert.equal(r.get16(g+0x4a90),slot);
  }finally{r.close();}
 }
});
test('paused boss menu does not consume potion while frames render',()=>{
 const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});
 try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
  const g=r.exports.get('Game');for(const [a,v] of [[0x4a8c,0],[0x4a90,0x6123],[0x433c,1],[0x433e,1],[0x4340,0],[0x4342,3],[0x4854,0],[0x48f2,189],[0x48f4,500],[0x48f6,189],[0x48f8,1000]])r.write(g+a,u16(v));r.write(g+0x7cd4,u32(1000));
  for(const [target,key] of [[r.bossRushUI,"open"],[r,"codexOpen"],[r.inventory,"open"],[r.features0130,"open"]]) {
   target[key]=true;for(let i=0;i<65;i++){r.step();r.framebuffer();}
   assert.equal(r.get16(g+0x4342),3);assert.equal(r.get16(g+0x4a90),0x6123);target[key]=false;
  }
 }finally{r.close();}
});
