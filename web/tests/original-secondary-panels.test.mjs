import test from 'node:test';
import assert from 'node:assert/strict';
import {DifficultyWeb} from '../dist/difficulty.mjs';
import {InventoryPanel,paintInventory,inventoryHit,inventoryTransform} from '../dist/inventory.mjs';
import {BossMage} from '../dist/boss-mage.mjs';
import {AutoPotionController} from '../dist/auto-potion.mjs';
function fixture(width,height){const calls=[],r={width,height,exports:new Map([['Game',100]]),get16:()=>100};r.originalUi={panel(p,b){calls.push({kind:'panel',...b});},text(p,x,y,value,color,width,scale=1){calls.push({kind:'text',x,y,value,color,width,scale});}};return {r,calls,pixels:new Uint8Array(width*height*2)};}
for(const [width,height]of [[400,240],[800,480]])test(`difficulty native paint and touch ${width}`,()=>{
 const {r,calls,pixels}=fixture(width,height),ui=new DifficultyWeb(r);ui.open=true;ui.unlockedTier=2;ui.paint(pixels);
 assert.ok(calls.some(c=>c.kind==='panel'));assert.ok(calls.some(c=>c.value?.includes('Pesadelo')));
 const rows=ui.layout().rows;ui.handleTouch(rows[4].x+1,rows[4].y+1,0);assert.equal(ui.open,true);assert.equal(ui.currentTier,0);assert.match(ui.statusMessage,/BLOQUEADA/);
 ui.handleTouch(rows[2].x+1,rows[2].y+1,0);assert.equal(ui.currentTier,2);assert.equal(ui.open,false);
 for(const c of calls)if(c.kind==='text')assert.ok(c.x>=0&&c.y>=0&&c.x+c.width<=width&&c.y+12*c.scale<=height);
});
test('inventory uses original provider without changing transformed controls',()=>{
 const {r,calls,pixels}=fixture(400,240),panel=new InventoryPanel();
 for(const scale of [75,100,125]){calls.length=0;paintInventory(pixels,panel,true,true,scale,r.originalUi);assert.ok(calls.some(c=>c.kind==='panel'));assert.ok(calls.some(c=>c.value?.includes('EQUIPAMENTOS')));const t=inventoryTransform(400,240,scale,true),p=t.point(28,46);assert.equal(inventoryHit(p.x,p.y,scale,true),0);assert.ok(calls.some(c=>c.kind==='panel'&&p.x>=c.x&&p.y>=c.y&&p.x<c.x+c.w&&p.y<c.y+c.h));}
});
test('inventory standalone callers keep framebuffer fallback',()=>{
 const panel=new InventoryPanel(),pixels=new Uint8Array(400*240*2);paintInventory(pixels,panel,true,true,100);assert.ok(pixels.some(v=>v));
});
test('mage preserves sprite and draws original prompt; potion notice uses native font',()=>{
 const {r,calls,pixels}=fixture(400,240),mage=new BossMage(r);mage.visibleSite=()=>({x:100,y:100});r.get16=a=>a===100+0x4878||a===100+0x487a?0:100;mage.art={body:{width:1,height:1,pixels:Uint32Array.of(0x1ffff)},shadow:{width:1,height:1,pixels:Uint32Array.of(0)}};mage.paint(pixels);assert.equal(pixels[(99*400+100)*2],255);assert.ok(calls.some(c=>c.value==='MAGO DOS BOSSES'));
 calls.length=0;const potion=new AutoPotionController(r);potion.toggle();potion.paintNotice(pixels,400,240);assert.ok(calls.some(c=>c.kind==='panel'));assert.ok(calls.some(c=>c.value===potion.notice));assert.equal(potion.noticeTimer,90);
});
