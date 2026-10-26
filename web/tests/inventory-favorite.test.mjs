import test from 'node:test';
import assert from 'node:assert/strict';
import * as inventory from '../dist/inventory.mjs';
const {InventoryPanel}=inventory;
test('protecao acompanha pool e fingerprint, sem confundir gemeos ou pool reciclado',()=>{
 assert.equal(typeof inventory.InventoryProtection,'function');const p=new inventory.InventoryProtection(), bytes=new Uint8Array(20).fill(3), twin=bytes.slice();
 assert.equal(p.toggle(7,bytes),true); bytes[0]=9;
 assert.equal(p.has(7,twin),true);assert.equal(p.has(8,twin),false);
 assert.equal(p.toggle(7,twin),false);assert.equal(p.has(7,twin),false);
 p.toggle(7,twin);assert.equal(p.has(7,bytes),false);assert.equal(p.has(7,twin),false);
 p.toggle(7,twin);p.clear();assert.equal(p.has(7,twin),false);
});
test('venda bloqueia protegido, permite desproteger e nao escolhe vizinho',()=>{
 const p=new InventoryPanel();p.items=[{bag:0,slot:0,pool:7,type:0,rank:0,level:1,price:10,protectedItem:true}];
 p.selectedPool=7;p.focus=7;p.sale=true;p.rebuild();
 assert.equal(p.canChoose(),false);assert.equal(p.key(6,20),'');
 assert.equal(p.hit(12,20),'protect');assert.equal(p.key(12,20),'protect');
 p.sale=false;assert.equal(p.canChoose(),true);
 p.page=1;assert.equal(p.hit(12,20),'');
});
test('sidecar persiste por personagem e rejeita arquivo invalido sem perder estado',()=>{
 const p=new inventory.InventoryProtection(),bytes=new Uint8Array(20).fill(7);
 assert.equal(typeof p.encode,'function');p.select(2);p.toggle(7,bytes);const saved=p.encode();
 const reopened=new inventory.InventoryProtection();reopened.decode(saved);reopened.select(2);assert.equal(reopened.has(7,bytes),true);
 reopened.select(1);assert.equal(reopened.has(7,bytes),false);reopened.select(2);
 for(const bad of [saved.slice(0,-1),saved+'2 7 '+ '07'.repeat(20)+'\n','UNKNOWN\n',saved.replace('2 7 ','2 200 ')]){assert.throws(()=>reopened.decode(bad));assert.equal(reopened.has(7,bytes),true);}
});

test('botao PROT acompanha hitbox nas tres escalas',()=>{
 for(const scale of [75,100,125]){const t=inventory.inventoryTransform(400,240,scale,true),p=inventory.panelRect(),point=t.point(p.x+280,p.y+197);assert.equal(inventory.inventoryHit(point.x,point.y,scale,true),12);}
});
