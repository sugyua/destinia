import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import os from 'node:os';import path from 'node:path';import {createRequire} from 'node:module';
import {DestiniaWeb} from '../dist/engine.mjs';
const require=createRequire(import.meta.url),temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-viewport-'));
fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));const uc=await require(path.join(temp,'unicorn.cjs'))();
process.on('exit',()=>fs.rmSync(temp,{recursive:true,force:true}));
const root=process.env.DESTINIA_GAME_ROOT||(fs.existsSync('game/assets') ? 'game' : '../game'),assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
const BASE=0x10000000,GAME=BASE+0x77ee0,MAP=BASE+0x80b18,CAMERA=GAME+18552;
const sh=(r,p,n)=>r.write(p,new Uint8Array([n&255,n>>8&255]));
function make(width=800){const r=new DestiniaWeb(uc,assets,{width,height:width===800?480:240,language:'en'});r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();return r;}
test('original ARM camera function and all sixteen writers preserve/center axes',()=>{
 const r=make();try{
  const writers=[[0x11b94,0,7,4,0],[0x11bc6,1,0,3,1],[0x14e62,6,3,7,0],[0x14eb6,0,1,7,1],[0x14ee2,2,0,6,0],[0x14f0a,3,2,1,1],[0x14f78,6,3,7,0],[0x14f8c,0,1,7,1],[0x19664,1,3,2,0],[0x1966c,1,3,2,1],[0x19d30,2,6,3,0],[0x19d3a,2,6,3,1],[0x1e3a8,2,4,1,0],[0x1e3aa,0,4,3,1],[0x1f92c,0,3,1,0],[0x1f94a,0,3,1,1]];
  for(const [mw,mh] of [[30,30],[60,60],[20,10]]){
   sh(r,MAP,mw);sh(r,MAP+2,mh);for(const position of [0,240,700]){r.call(r.exports.get('Map_setMapView'),[position,position]);assert.equal(r.get16(CAMERA,true),mw*16<800?(mw*16-800)/2:Math.max(0,Math.min(mw*16-800,position-400)));assert.equal(r.get16(CAMERA+2,true),mh*16<480?(mh*16-480)/2:Math.max(0,Math.min(mh*16-480,position-240)));}
   for(const [pc,source,base,index,axis] of writers){sh(r,CAMERA,37);sh(r,CAMERA+2,51);r.w(source,123);r.w(base,CAMERA+axis*2);r.w(index,0);
    const hook=r.e.hook_add(r.uc.HOOK_CODE,()=>r.e.emu_stop(),null,BASE+pc+2,BASE+pc+2);try{r.e.emu_start((BASE+pc)|1,BASE+pc+4,0,0);}finally{r.e.hook_del(hook);}
    assert.equal(r.get16(CAMERA,true),mw*16<800?(mw*16-800)/2:axis===0?123:37,pc.toString(16));assert.equal(r.get16(CAMERA+2,true),mh*16<480?(mh*16-480)/2:axis===1?123:51,pc.toString(16));
   }
  }
 }finally{r.close();}
 const classic=make(400);try{assert.equal(classic.mapViewport.enabled,false);}finally{classic.close();}
});
test('real Map_drawBuff removes duplicate margins, preserves other Game bytes and stays within frame budget',()=>{
 const r=make();try{
  r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[0,61,1]);r.call(r.exports.get('Map_loadTile'),[0]);r.call(r.exports.get('Map_setMapView'),[240,240]);
  const handle=r.get32(r.exports.get('Grp')+4),pointer=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[handle]),sentinel=new Uint8Array(800*480*2).fill(0x35),before=r.read(GAME,32056),saved=[...r.saved];
  r.mapViewport.enabled=false;r.write(pointer,sentinel);r.call(r.exports.get('Map_drawBuff'),[handle,1]);const broken=r.read(pointer,sentinel.length);
  r.mapViewport.enabled=true;r.mapViewport.cacheTiles=false;r.mapViewport.fastReject=false;r.write(pointer,sentinel);r.call(r.exports.get('Map_drawBuff'),[handle,1]);const reference=r.read(pointer,sentinel.length),graphicsReference=r.read(r.exports.get('Grp'),124);r.mapViewport.cacheTiles=true;r.mapViewport.fastReject=true;r.write(pointer,sentinel);r.call(r.exports.get('Map_drawBuff'),[handle,1]);const fixed=r.read(pointer,sentinel.length);assert(r.mapViewport.stats.skipped>0);assert.notDeepEqual(broken,fixed);assert.deepEqual(fixed,reference,'Batched/rejected tiles must match native clipping pixel for pixel');assert.deepEqual(r.read(r.exports.get('Grp'),124),graphicsReference);assert.equal(r.mapViewport.context,null);
  for(let y=0;y<480;y++)for(const x of [0,159,640,799]){const at=(y*800+x)*2;assert.equal(fixed[at]|fixed[at+1]<<8,0x8410,`margin ${x},${y}`);}
  assert(fixed.some((c,i)=>{const x=Math.floor(i/2)%800;return x>=160&&x<640&&c!==0x35;}));assert.deepEqual(r.read(GAME,32056),before);assert.deepEqual([...r.saved],saved);
  for(let i=0;i<3;i++)r.call(r.exports.get('Map_drawBuff'),[handle,1]);const start=performance.now();for(let i=0;i<8;i++)r.call(r.exports.get('Map_drawBuff'),[handle,1]);const ms=(performance.now()-start)/8;
  console.log('MAP_VIEWPORT_RENDER_MS',ms.toFixed(2),'tilesSkipped',r.mapViewport.stats.skipped);if(process.env.DESTINIA_PERF_CHECK==='1')assert(ms<62.5,`Map render ${ms.toFixed(1)}ms exceeds 16 FPS budget`);
 }finally{r.close();}
});
