import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import {inflateSync} from 'node:zlib';
import {portraitAssets,decodePortrait,loadPortraits} from '../dist/codex-portraits.mjs';
const root=(process.env.DESTINIA_GAME_ROOT||'/workspace/destinia-06/game')+'/assets/';
const assets=new Map(fs.readdirSync(root).map(n=>[n,new Uint8Array(fs.readFileSync(root+n))]));
const unpack=b=>b[0]===90?new Uint8Array(inflateSync(b.subarray(12))):b;
test('figuras completas dos 73 monstros e variantes originais',async()=>{
 const names=new Map([...Array.from({length:63},(_,i)=>i+1),...Array.from({length:10},(_,i)=>100+i)].map(id=>[id,String(id)]));
 const cache=await loadPortraits(assets,names);assert.equal(cache.size,73);
 for(const [id,w,h] of [[1,27,28],[4,54,43],[100,76,81],[102,75,69],[109,108,92]]){const p=cache.get(id);assert.equal(p.width,w);assert.equal(p.height,h);assert.equal(p.pixels.length,w*h);assert.ok(p.pixels.some(c=>c&0x10000));}
 assert.equal(new Set(Array.from({length:63},(_,i)=>Array.from(cache.get(i+1).pixels).join(','))).size,63);
 assert.deepEqual(cache.get(101),cache.get(103));assert.equal(portraitAssets(999),null);
});
test('dados ausentes ou corrompidos preservam a consulta sem imagem falsa',async()=>{
 const [wmb,agd]=portraitAssets(1),w=unpack(assets.get(wmb)),a=unpack(assets.get(agd));
 for(const len of [0,4,11,w.length-1])assert.equal(decodePortrait(w.subarray(0,len),a),null);
 const bad=a.slice();bad[11]=255;assert.equal(decodePortrait(w,bad),null);
 const missing=new Map(assets);missing.delete(wmb);const cache=await loadPortraits(missing,new Map([[1,'Fungus'],[4,'Scorpion']]));assert.equal(cache.get(1),null);assert.ok(cache.get(4));
 const wrong=assets.get(wmb).slice();wrong[wrong.length-1]^=1;missing.set(wmb,wrong);assert.equal((await loadPortraits(missing,new Map([[1,'Fungus']]))).get(1),null);
});

test('pinta miniatura na grade, preserva preto opaco e contagem',async()=>{
 const {paintCodex,codexCell}=await import('../dist/codex-ui.mjs');const pixels=new Uint8Array(400*240*2),rows=[{id:1,name:'Fungus',found:true,kills:123}];
 const art=new Map([[1,{width:2,height:1,pixels:new Uint32Array([0x10000,0x1f81f])}]]);
 paintCodex(pixels,rows,{open:true,portraits:art});const p=codexCell(0),at=((p.y+10)*400+p.x+3)*2;
 assert.equal(pixels[at]|pixels[at+1]<<8,0,'Preto do sprite não é transparência');
 const missing=new Uint8Array(pixels.length);paintCodex(missing,rows,{open:true});assert.notDeepEqual(missing,pixels);
 assert.deepEqual(rows,[{id:1,name:'Fungus',found:true,kills:123}]);
});

test('cores RGB, montagem e espelhamento correspondem às referências originais',()=>{
 for(const [id,expected] of [[1,0xcb852f42],[4,0x033b9dc1],[61,0xba844278],[100,0xac4761fc],[102,0x3301b4b9],[109,0x4e053378]]){
  const paths=portraitAssets(id),p=decodePortrait(unpack(assets.get(paths[0])),unpack(assets.get(paths[1])),id<100?1:0);let hash=2166136261;
  for(const c of p.pixels)for(let shift=0;shift<32;shift+=8)hash=Math.imul(hash^(c>>>shift&255),16777619)>>>0;assert.equal(hash,expected);
 }
});

test('Imperial Soldier usa a pose frontal original com corpo e arma unidos',()=>{
 const paths=portraitAssets(61),p=decodePortrait(unpack(assets.get(paths[0])),unpack(assets.get(paths[1])));
 assert.deepEqual([p.width,p.height],[47,46]);
});

// Monster_setAction: idle normal + dir3 (baixo/frente) = ação1; bosses usam ação0.
test('pose frontal percorre todas as molduras anteriores e rejeita AGD incompleto',()=>{
 const [wn,an]=portraitAssets(61),w=unpack(assets.get(wn)),a=unpack(assets.get(an));
 // mon21 action1 header=651, frame0=653, parts=662 (10 parts).
 for(const len of [12,651,653,661,662,741])assert.equal(decodePortrait(w,a.subarray(0,len)),null);
 const absent=a.slice();absent[0]=1;assert.equal(decodePortrait(w,absent),null);
 const empty=a.slice();empty[652]=0;assert.equal(decodePortrait(w,empty),null);
 const bad=a.slice();bad[661]=255;assert.equal(decodePortrait(w,bad),null);
 assert.equal(decodePortrait(w,a,-1),null);assert.equal(decodePortrait(w,a,256),null);
});
