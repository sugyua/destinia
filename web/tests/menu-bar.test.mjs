import test from 'node:test';import assert from 'node:assert/strict';
import {nativeTabRect,nativeBarRect,nativeTabHit,nativeTransform} from '../dist/codex-native.mjs';
test('ten menu cells and frame are centered and hits share raster geometry',()=>{
 for(const [w,h] of [[400,240],[800,480]])for(const scale of [75,100,125]){
  const bar=nativeBarRect(w,h),first=nativeTabRect(0,w,h),last=nativeTabRect(9,w,h),t=nativeTransform(scale,w,h);
  assert.equal(bar.x*2+bar.w,w);assert.equal(first.x+last.x+last.w,w);
  const transformed=t.rect(first.x,first.y,last.x+last.w-first.x,24);assert(Math.abs(transformed.x*2+transformed.w-w)<=1);
  for(let i=0;i<10;i++){const b=nativeTabRect(i,w,h);assert.equal(b.w,23);const raster=t.rect(b.x,b.y,b.w,b.h);for(const x of [raster.x,raster.x+raster.w-1]){const q=t.inverse(x,raster.y+Math.floor(raster.h/2));assert.equal(nativeTabHit(q.x,q.y,w,h),i);}}
  assert.equal(nativeTabHit(first.x-1,first.y+12,w,h),-1);assert.equal(nativeTabHit(last.x+last.w,last.y+12,w,h),-1);
 }
});
