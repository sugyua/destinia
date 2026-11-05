import test from 'node:test';
import assert from 'node:assert/strict';
import {CombatState} from '../dist/survival-state.mjs';

test('mitigation, evasion, damage cap, last stand, immunity, cooldown and elixir are deterministic',()=>{
 const c=new CombatState();c.enabled=true;
 assert.equal(c.hit(100,100,1000,9999),60);
 assert.equal(c.hit(20,100,1000,9999),19);assert.equal(c.immuneMs,2000);assert.equal(c.cooldownMs,120000);
 assert.equal(c.hit(1,100,10,9999),0);c.tick(2000);assert.equal(c.hit(1,100,10,9999),10);
 c.master=true;assert.equal(c.hit(100,100,50,1499),0);assert.equal(c.hit(100,100,50,1500),40);
 assert.equal(c.activateElixir(),true);assert.equal(c.elixirMs,300000);assert.equal(c.hit(100,100,50,1500),30);
 c.tick(300000);assert.equal(c.elixirMs,0);assert.equal(c.hit(100,100,50,1500),40);
});
