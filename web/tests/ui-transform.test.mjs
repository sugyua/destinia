import test from 'node:test';import assert from 'node:assert/strict';
import {UiTransform} from '../dist/ui-transform.mjs';
test('transformacao da UI casa pixels e inversa nas tres escalas e cinco ancoras',()=>{
 for(const percent of [75,100,125])for(const anchor of ['TopLeft','TopRight','BottomLeft','BottomRight','Center']){
  const t=new UiTransform(800,480,percent,anchor);
  for(let x=-100;x<900;x++){const a=t.point(x,0).x,b=t.point(x+1,0).x;for(let dx=a;dx<b;dx++)assert.equal(t.inverse(dx,0).x,x);}
  for(let y=-100;y<600;y++){const a=t.point(0,y).y,b=t.point(0,y+1).y;for(let dy=a;dy<b;dy++)assert.equal(t.inverse(0,dy).y,y);}
 }
});
test('paineis grandes respeitam encaixe na tela sem mudar a escala do mundo',()=>{
 const t=new UiTransform(400,240,125,'Center',232,240);assert.equal(t.scale(),1);assert.deepEqual(t.rect(84,0,232,240),{x:84,y:0,w:232,h:240});
 const codex=new UiTransform(400,240,125,'Center',360,215,20,25);assert.ok(codex.scale()<1.25);const p=codex.rect(20,25,360,215);assert.ok(p.x>=0&&p.y>=0&&p.x+p.w<=400&&p.y+p.h<=240);
 const normal=new UiTransform(400,240,100);assert.deepEqual(normal.point(19,27),{x:19,y:27});
});
