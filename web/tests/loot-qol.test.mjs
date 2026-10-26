import test from 'node:test';import assert from 'node:assert/strict';
const loot=await import('../dist/loot-qol.mjs').catch(()=>({}));
test('loot opt-in, categorias confirmadas, ranks e raio nativo',()=>{
 assert.equal(typeof loot.lootOptions,'function');const options=loot.lootOptions();assert.equal(options.visual,false);assert.equal(options.autoGold,true);assert.equal(options.autoMaterials,false);
 const types=new Map([[79,14],[0,0],[119,22]]);
 const gold=loot.describeDrop(5,50,types),material=loot.describeDrop(1,79,types),potion=loot.describeDrop(1,0,types),gear=loot.describeDrop(0,3<<8,types);
 assert.equal(gold.category,4);assert.equal(material.category,2);assert.equal(potion.category,16);assert.equal(gear.rank,3);
 assert.equal(loot.autoEligible(gold,options,0,0),true);options.autoGold=false;assert.equal(loot.autoEligible(gold,options,0,0),false);options.autoGold=options.autoMaterials=true;
 assert.equal(loot.autoEligible(gold,options,19,-19),true);assert.equal(loot.autoEligible(gold,options,20,0),false);
 assert.equal(loot.autoEligible(material,options,0,0),true);assert.equal(loot.autoEligible(potion,options,0,0),false);assert.equal(loot.autoEligible(gear,options,0,0),false);
 options.visual=true;options.categories=2;assert.equal(loot.visibleDrop(gold,options),false);assert.equal(loot.visibleDrop(material,options),true);options.categories=31;options.minRank=4;assert.equal(loot.visibleDrop(gear,options),false);
 assert.throws(()=>loot.lootOptions({radius:20}));assert.throws(()=>loot.lootOptions({categories:32}));
});
