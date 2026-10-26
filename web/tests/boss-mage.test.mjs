import test from 'node:test';import assert from 'node:assert/strict';
import {bossMagePalette,mageFacing,placeBossMage,BossRushMenuEvent} from '../dist/boss-mage.mjs';
test('isolated palette preserves alpha and source; native facing range is exact',()=>{
 const art={width:2,height:2,pixels:Uint32Array.from([0,0x1ffff,0x1f800,0x18410])},before=art.pixels.slice(),copy=bossMagePalette(art);
 assert.deepEqual(art.pixels,before);assert.equal(copy.pixels[0],0);assert.notEqual(copy.pixels[2],art.pixels[2]);for(let i=0;i<4;i++)assert.equal(copy.pixels[i]>>>16,art.pixels[i]>>>16);
 const p={area:0,map:65,x:200,y:200};assert(mageFacing(p,200,216,2));assert(!mageFacing(p,200,216,3));assert(!mageFacing(p,200,217,2));
 assert(mageFacing(p,184,200,1)&&mageFacing(p,216,200,0)&&mageFacing(p,200,184,3));
});
test('map parser rejects truncation and wholly blocked maps',()=>{assert.throws(()=>placeBossMage(0,65,new Uint8Array([40,40,0,0])));});
test('menu event is deferred and acknowledged once by explicit UI opener',()=>{const e=new BossRushMenuEvent();let count=0;e.openBossRushMenu();assert(e.pending);e.opener=()=>count++;e.dispatch();e.dispatch();assert.equal(count,1);assert(!e.pending);});
