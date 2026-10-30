import test from 'node:test';
import assert from 'node:assert/strict';
import {WeaponsCatalogWeb} from '../dist/weapons-catalog.mjs';
import {SharedStashWeb} from '../dist/shared-stash.mjs';
import {BountiesWeb} from '../dist/bounties.mjs';
function fixture(width){const calls=[];return {width,height:width*0.6,calls,originalUi:{panel(p,b){calls.push({b});},text(p,x,y,value,c,w,s){calls.push({x,y,value:String(value),w,s});}}};}
for(const width of [400,800]){
 test(`catalog authentic text and touch ${width}`,()=>{const e=fixture(width),v=new WeaponsCatalogWeb(e);v.open=true;v.paint(new Uint8Array(width*width*1.2));assert.ok(e.calls.some(c=>c.value?.includes('Espada de Madeira')));const tab=e.calls.find(c=>c.value==='Atirador');assert.ok(tab);v.handleTouch(tab.x+2,tab.y+2,0);assert.equal(v.selectedJob,2);const category=e.calls.find(c=>c.value==='Armas');v.handleTouch(category.x+2,category.y+2,0);assert.equal(v.selectedCategory,1);});
 test(`stash selected item and aligned grid ${width}`,()=>{const e=fixture(width),v=new SharedStashWeb(e);v.open=true;v.items=[{name:'Espada real',count:3,rank:2,level:7}];v.paint(new Uint8Array(width*width*1.2));assert.ok(e.calls.some(c=>c.value?.includes('Espada real')));const page=e.calls.find(c=>c.value==='Pagina 2');v.handleTouch(page.x+2,page.y+2,0);assert.equal(v.page,1);e.calls.length=0;v.paint(new Uint8Array(width*width*1.2));const slot=e.calls.find(c=>c.value==='25');assert.ok(slot);v.handleTouch(slot.x+2,slot.y+2,0);assert.equal(v.selectedIndex,24);});
 test(`bounties objectives rewards and aligned claim ${width}`,()=>{const e=fixture(width),v=new BountiesWeb(e);v.open=true;const list=v.getContracts(0);list[0].completed=true;v.paint(new Uint8Array(width*width*1.2));assert.ok(e.calls.some(c=>c.value?.includes(list[0].title)));assert.ok(e.calls.some(c=>c.value?.includes('15000')));const claim=e.calls.find(c=>c.value==='Resgatar');assert.ok(claim);v.handleTouch(claim.x+2,claim.y+2,0);assert.equal(list[0].claimed,true);});
}

test('feature panels render original ELF resources and preserve game state',{skip:!process.env.DESTINIA_GAME_ROOT},async()=>{
 const fs=await import('node:fs'),os=await import('node:os'),path=await import('node:path');
 const {createRequire}=await import('node:module');
 const {DestiniaWeb}=await import('../dist/engine.mjs');
 const {OriginalUi}=await import('../dist/original-ui.mjs');
 const temp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-feature-panels-'));
 try{
  fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(temp,'unicorn.cjs'));
  const uc=await createRequire(import.meta.url)(path.join(temp,'unicorn.cjs'))();
  const root=process.env.DESTINIA_GAME_ROOT;
  const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
  for(const width of [400,800]){
   const r=new DestiniaWeb(uc,assets,{width,height:width*0.6,language:'en'});
   try{
    r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
    r.originalUi??=new OriginalUi(r);
    const g=r.exports.get('Game'),before=r.read(g,32056);
    for(const Feature of [WeaponsCatalogWeb,SharedStashWeb,BountiesWeb]){
     const v=new Feature(r);v.open=true;const pixels=new Uint8Array(width*width*1.2);v.paint(pixels);
     assert.ok(pixels.some(Boolean),Feature.name+' produces native pixels');assert.deepEqual(r.read(g,32056),before);
    }
   }finally{r.close();}
  }
 }finally{fs.rmSync(temp,{recursive:true,force:true});}
});
