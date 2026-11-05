import test from 'node:test';
import assert from 'node:assert/strict';
import {BossRushUI} from '../dist/boss-rush.mjs';
import {Features0130} from '../dist/features-0130.mjs';

function fixture(width,height){
 const calls=[];const engine={width,height,originalUi:{panel(p,b){calls.push({kind:'panel',...b});},text(p,x,y,value,color,width,scale){calls.push({kind:'text',x,y,value,width,scale});}}};
 const pixels=new Uint8Array(width*height*2);return {engine,pixels,calls};
}
for(const [w,h] of [[400,240],[800,480]]){
 test(`arena native frames, all six tabs and matching row touch ${w}`,()=>{
  const {engine,pixels,calls}=fixture(w,h),ui=new BossRushUI(engine);engine.features0130=new Features0130(engine);ui.openMenu();
  for(let t=0;t<6;t++){
   ui.currentTab=t;ui.selectedIndex=0;calls.length=0;ui.paint(pixels,w,h);
   assert.ok(calls.some(c=>c.kind==='panel'),'original frame must render');
   assert.ok(calls.filter(c=>c.kind==='text').length>8,'tab rows must render');
   const b=ui.layout(w,h),rect=ui.rowRects(b)[1];
   assert.ok(calls.some(c=>c.kind==='text'&&c.y===rect.y),'touch row must match rendered baseline');
   ui.handleTouch(rect.x+2,rect.y+rect.h-1,0);assert.equal(ui.selectedIndex,1);
   for(const c of calls){assert.ok(c.x>=0&&c.y>=0);if(c.kind==='text')assert.ok(c.x+c.width<=w&&c.y+12*c.scale<=h);}
  }
  const b=ui.layout(w,h);for(let t=0;t<6;t++){const r=b.tabs[t];ui.handleTouch(r.x+r.w/2,r.y+r.h/2,0);assert.equal(ui.currentTab,t);}
  ui.currentTab=0;ui.selectedIndex=6;ui.handleKey(9,true);assert.equal(ui.selectedIndex,7);assert.equal(ui.scrollOffset,1);
  calls.length=0;ui.paint(pixels,w,h);assert.ok(calls.some(c=>c.value?.includes('Baramos')));
  ui.handleTouch(b.close.x+1,b.close.y+1,0);assert.equal(ui.open,false);
 });
 test(`refinement native layout, eight actions and footer fit ${w}`,()=>{
  const {engine,pixels,calls}=fixture(w,h),ui=new Features0130(engine);ui.open=true;ui.message='REFINADO +15';ui.paint(pixels);
  assert.ok(calls.some(c=>c.kind==='panel'));assert.ok(calls.some(c=>c.value===ui.message));
  const actions=[];ui.action=i=>actions.push(i);const b=ui.layout();
  for(let i=0;i<8;i++)ui.handleTouch(b.x+10,b.top+i*b.row+b.row-1,0);
  assert.deepEqual(actions,[0,1,2,3,4,5,6,7]);
  assert.ok(calls.some(c=>c.value?.includes('ARMAS +8%')));
  for(const c of calls){assert.ok(c.x>=0&&c.y>=0);if(c.kind==='text')assert.ok(c.x+c.width<=w&&c.y+12*c.scale<=h);}
 });
 test(`active arena HUD uses original frame while panel closed ${w}`,()=>{
  const {engine,pixels,calls}=fixture(w,h),ui=new Features0130(engine);ui.arena.active=true;ui.arena.currentWave=3;ui.arena.remaining=4;ui.paint(pixels);
  assert.equal(calls.filter(c=>c.kind==='panel').length,1);
  assert.ok(calls.some(c=>c.value==='ONDA 3 RESTAM 4'));
  assert.equal(ui.handleTouch(12,12,0),false);
 });
}
