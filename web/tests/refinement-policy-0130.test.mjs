import test from 'node:test';
import assert from 'node:assert/strict';
import { weaponAttack, armorDefense, armorHealth, quote, checkpoint, Stone } from '../dist/refinement-policy.mjs';

test('refinement 0..15 uses item base, exact requested percentages and integer rounding', () => {
  for (let level=0; level<=15; level++) {
    for (const base of [0,1,99,100,101,32767,2147483647]) {
      for (const [fn,percent] of [[weaponAttack,8],[armorDefense,6],[armorHealth,4]]) {
        assert.equal(fn(base,level),Number(BigInt(base)+BigInt(base)*BigInt(level*percent)/100n));
      }
    }
    assert.equal(checkpoint(level),level>=10?10:level>=5?5:0);
  }
});
test('refinement boundaries require the correct consumable and guarantee success', () => {
  for (let current=0; current<15; current++) {
    const target=current+1,q=quote(current);
    assert.deepEqual(q,{target,stone:target<=5?Stone.Common:target<=10?Stone.Pure:Stone.Abyssal,stones:target,gold:500*target*target,successPercent:100});
  }
  assert.throws(()=>quote(15));
});
test('invalid levels, nonintegral stats and arithmetic overflow are rejected', () => {
  for (const level of [-1,16,NaN,Infinity,1.1,'1',null]) {
    assert.throws(()=>weaponAttack(100,level));
    assert.throws(()=>checkpoint(level));
    assert.throws(()=>quote(level));
  }
  for (const base of [-1,NaN,Infinity,1.1,'100',null,Number.MAX_SAFE_INTEGER+1]) assert.throws(()=>armorDefense(base,1));
  assert.equal(weaponAttack(Number.MAX_SAFE_INTEGER,0),Number.MAX_SAFE_INTEGER);
  assert.throws(()=>weaponAttack(Number.MAX_SAFE_INTEGER,15));
});
