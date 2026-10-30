import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {createRequire} from 'node:module';
import {DestiniaWeb} from '../dist/engine.mjs';
import {OriginalCompanions} from '../dist/original-companions.mjs';
const root=process.env.DESTINIA_GAME_ROOT||'D:/IA/Meus Aplicativos/destinia/game';
const temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-companions-'));
fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));
const uc=await createRequire(import.meta.url)(path.join(temp,'unicorn.cjs'))();
process.on('exit',()=>fs.rmSync(temp,{recursive:true,force:true}));
function make(){const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))])),r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en',fastGraphics:false});r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();return r;}
test('four original cosmetic companions preserve native state and retain actual animation',async()=>{
 const r=make();try{const art=new OriginalCompanions(r),g=r.exports.get('Game'),before=r.read(g,32056),grp=r.read(r.exports.get('Grp'),124),stack=r.read(r.stackBase,r.stackSize),pixels=new Uint8Array(192000);
  assert.equal(art.paint(pixels,0,20,20),false);await art.prepare();assert.deepEqual(r.read(g,32056),before);assert.deepEqual(r.read(r.exports.get('Grp'),124),grp);assert.deepEqual(r.read(r.stackBase,r.stackSize),stack);
  const signatures=[];for(let id=0;id<4;id++){pixels.fill(0);assert.equal(art.paint(pixels,id,40,40,0,1),true);assert.ok(pixels.some(v=>v));signatures.push(Buffer.from(pixels).toString('base64'));assert.equal(art.sprites[id].actions.length,id<3?2:1);assert.ok(art.sprites[id].actions.every(a=>a.length>0));for(const a of art.sprites[id].actions)for(const f of a)assert.ok(f.width<=24&&f.height<=24);pixels.fill(0);art.paint(pixels,id,1,12,999,90);assert.ok(pixels.some(v=>v));}
  assert.equal(new Set(signatures).size,4);assert.deepEqual(r.read(g,32056),before);
 }finally{r.close();}
});
test('corrupt original resource reports failure and never draws replacement art',async()=>{const r=make();try{r.assets.set('pet3.wmb',Uint8Array.of(1,2,3));const art=new OriginalCompanions(r),before=r.read(r.exports.get('Game'),32056),pixels=new Uint8Array(192000);await assert.rejects(art.prepare(),/pet3.wmb/);assert.equal(art.paint(pixels,0,20,20),false);assert.equal(pixels.some(v=>v),false);assert.deepEqual(r.read(r.exports.get('Game'),32056),before);}finally{r.close();}});

test('optional companion art failure does not prevent the game reaching its first frame',async()=>{const r=make();try{r.assets.set('pet3.wmb',Uint8Array.of(1,2,3));const before=r.read(r.exports.get('Game'),32056);assert.equal(await r.prepareOriginalCompanions(),false);assert.deepEqual(r.read(r.exports.get('Game'),32056),before);r.step();const pixels=r.framebuffer();assert.equal(pixels.length,192000);assert.ok(pixels.some(v=>v));}finally{r.close();}});
