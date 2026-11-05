import test from 'node:test';import assert from 'node:assert/strict';
import {bossMagePalette,mageFacing,placeBossMage,BossRushMenuEvent} from '../dist/boss-mage.mjs';
test('original palette preserves every color, alpha and source; native facing range is exact',()=>{
 const art={width:2,height:2,pixels:Uint32Array.from([0,0x1ffff,0x1f800,0x18410])},before=art.pixels.slice(),copy=bossMagePalette(art);
 assert.deepEqual(art.pixels,before);assert.equal(copy.pixels[0],0);assert.deepEqual(copy.pixels,art.pixels,'original NPC palette must be retained');for(let i=0;i<4;i++)assert.equal(copy.pixels[i]>>>16,art.pixels[i]>>>16);
 const p={area:0,map:65,x:200,y:200};assert(mageFacing(p,200,216,2));assert(!mageFacing(p,200,216,3));assert(!mageFacing(p,200,229,2));
 assert(mageFacing(p,184,200,1)&&mageFacing(p,216,200,0)&&mageFacing(p,200,184,3));
});
test('map parser rejects truncation and wholly blocked maps',()=>{assert.throws(()=>placeBossMage(0,65,new Uint8Array([40,40,0,0])));});
test('menu event is deferred and acknowledged once by explicit UI opener',()=>{const e=new BossRushMenuEvent();let count=0;e.openBossRushMenu();assert(e.pending);e.opener=()=>count++;e.dispatch();e.dispatch();assert.equal(count,1);assert(!e.pending);});

test('Mago dos Bosses esta presente nas 5 cidades do jogo', () => {
 for (const map of [61, 62, 63, 64, 65]) {
  // dummy MPD bytes: 30x30, p=0, q=0, grid of 30*30*4 bytes, records, 0 spawns
  const w = 30, h = 30, p = 0, q = 0;
  const dummy = new Uint8Array(4 + w * h * 4 + 1 + 2 + 1 + 1 + 1 + 12 + 1);
  dummy[0] = w; dummy[1] = h; dummy[2] = p; dummy[3] = q;
  const site = placeBossMage(0, map, dummy);
  assert.ok(site, 'Cidade ' + map + ' deve ter Mago dos Bosses');
  assert.equal(site.area, 0);
  assert.equal(site.map, map);
  assert.ok(site.x > 0 && site.y > 0);
 }
});
