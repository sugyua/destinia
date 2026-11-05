import test from 'node:test';import assert from 'node:assert/strict';
import {QuestTrackerWeb} from '../dist/quest-tracker.mjs';
for(const [width,height]of [[400,240],[800,480]])test('quest content and touch share original frame at '+width,()=>{
 const calls=[],r={width,height,originalUi:{panel:(_,b)=>calls.push(['panel',b]),text:(_,x,y,t)=>calls.push(['text',x,y,t])}};
 const q=new QuestTrackerWeb(r);q.setQuest('MISSAO REAL','ABATES 1 / 15','CAMPO');q.paint(new Uint8Array(width*height*2));
 const box=calls.find(c=>c[0]==='panel')?.[1];assert.ok(box,'authentic frame must be used');assert.ok(box.x>=0&&box.y>=0&&box.x+box.w<=width&&box.y+box.h<=height);
 for(const text of ['MISSAO REAL','ABATES 1 / 15','CAMPO'])assert.ok(calls.some(c=>c[0]==='text'&&c[3].includes(text)),text+' visible');
 assert.equal(q.handleTouch(box.x+box.w-4,box.y+4,0),true);assert.equal(q.minimized,true);
 assert.equal(q.handleTouch(0,height-1,0),false);
});
