import test from 'node:test';import assert from 'node:assert/strict';
import {nativePanel,nativeCell,nativeTabHit,nativeTransform,nativeDetailsButton} from '../dist/codex-native.mjs';
test('Códex cabe na moldura original e décima aba preserva nove posições distintas',()=>{
 const p=nativePanel();assert.equal(p.w,232);assert.equal(p.h,215);
 for(let i=0;i<32;i++){const c=nativeCell(i);assert.ok(c.x>=p.x&&c.x+c.w<=p.x+p.w);assert.ok(c.y+c.h<160);}
 for(let i=0;i<10;i++)assert.equal(nativeTabHit(96+i*23,12),i);
 assert.equal(nativeTabHit(350,12),-1);assert.ok(nativeDetailsButton().w>0);
 for(const scale of [75,100,125]){const t=nativeTransform(scale),p=t.point(303,12),q=t.inverse(p.x,p.y);assert.equal(nativeTabHit(q.x,q.y),9);const b=t.rect(84,0,255,240);assert.ok(b.y>=0&&b.y+b.h<=240);}
});
