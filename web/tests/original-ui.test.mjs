import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {createRequire} from 'node:module';
import {DestiniaWeb} from '../dist/engine.mjs';
const {OriginalUi}=await import('../dist/original-ui.mjs').catch(()=>({}));
test('original UI adapter is available for authentic frames and text',()=>{assert.equal(typeof OriginalUi,'function');assert.equal(typeof OriginalUi.prototype.cursor,'function');});
const require=createRequire(import.meta.url),temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-original-ui-'));
fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));
const uc=await require(path.join(temp,'unicorn.cjs'))();
process.on('exit',()=>fs.rmSync(temp,{recursive:true,force:true}));
const root=process.env.DESTINIA_GAME_ROOT||(fs.existsSync('game/assets')?'game':'../game');
const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
test('native frame and font reproduce original ARM pixels without changing game state',()=>{
 if(!OriginalUi)return;
 const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});
 try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();for(let i=0;i<10;i++)r.step();
 const ui=new OriginalUi(r),g=r.exports.get('Game'),before=r.read(g,32056),grp=r.read(r.exports.get('Grp'),124),output=new Uint8Array(192000);
 const stack=r.read(0x7f1ff000,4096);
 ui.panel(output,{x:20,y:30,w:100,h:30});
 assert.deepEqual(r.read(0x7f1ff000,4096),stack,'drawing must preserve ARM stack');
 const direct=ui.capture(100,30,()=>ui.call('_drawDescBox',[4,0,0,100,30]));
 for(let y=0;y<30;y++)assert.deepEqual(output.subarray(((y+30)*400+20)*2,((y+30)*400+120)*2),direct.subarray(y*100*2,(y+1)*100*2));
 ui.text(output,28,38,'QUEST 1 / 15',0xffff,190);
 assert.deepEqual(r.read(g,32056),before);assert.deepEqual(r.read(r.exports.get('Grp'),124),grp);
 assert.throws(()=>ui.capture(80,30,()=>{r.put32(g,123);throw Error('injected');}),/injected/);
 assert.deepEqual(r.read(g,32056),before);assert.deepEqual(r.read(r.exports.get('Grp'),124),grp);
 assert.ok(output.some(v=>v));
 }finally{r.close();}
});

test('large windows reuse the original main-menu bevel and background',()=>{
 const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});
 try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);const u=r.exports.get('GameUI');r.put32(u+0x7c,0);r.write(u+0x76,new Uint8Array([16,0]));
 const ui=new OriginalUi(r),reference=ui.capture(400,240,()=>ui.call('GameUI_drawMainMenu',[4])),pixels=new Uint8Array(192000);
 ui.panel(pixels,{x:10,y:20,w:300,h:180});
 for(const [dx,dy,sx,sy]of [[0,0,84,25],[296,0,312,25],[0,176,84,236],[296,176,312,236]])for(let y=0;y<4;y++)for(let x=0;x<4;x++){
 const a=((20+dy+y)*400+10+dx+x)*2,b=((sy+y)*400+sx+x)*2;assert.equal(pixels[a]|pixels[a+1]<<8,reference[b]|reference[b+1]<<8,'original bevel corner');}
 const b=(132*400+306)*2,a=(100*400+150)*2;assert.equal(pixels[a]|pixels[a+1]<<8,reference[b]|reference[b+1]<<8,'original window background');
 }finally{r.close();}
});

test('panels opened during intro do not access unloaded menu resources or poison their cache',()=>{
 const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});
 try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();const ui=new OriginalUi(r),pixels=new Uint8Array(192000);
 assert.equal(r.get16(r.exports.get('GameUI')+0x24fc+8),0);ui.panel(pixels,{x:10,y:20,w:180,h:100});const intro=pixels.slice();
 r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);ui.panel(pixels,{x:10,y:20,w:180,h:100});assert.notDeepEqual(pixels,intro,'loaded menu replaces temporary original description frame');
 }finally{r.close();}
});

test('selection cursor uses the original image anchor',()=>{const r=new DestiniaWeb(uc,assets,{width:400,height:240,language:'en'});try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();const ui=new OriginalUi(r),p=r.exports.get('MMain')+12,anchors=r.read(p+12,4);try{r.write(p+12,Uint8Array.of(3,0,4,0));const output=new Uint8Array(192000);ui.cursor(output,50,50);const reference=ui.capture(400,240,()=>ui.call('Grp_drawWImage',[4,50,50,p]),true);for(let n=0;n<output.length;n+=2){const color=reference[n]|reference[n+1]<<8;assert.equal(output[n]|output[n+1]<<8,color===0xf81f?0:color);}}finally{r.write(p+12,anchors);}}finally{r.close();}});
