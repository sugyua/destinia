import test from 'node:test';
import assert from 'node:assert/strict';
import { wave, Arena, encode, decode } from '../dist/survival.mjs';
import { tag } from '../dist/refinement.mjs';

// Acceptance contract from the Maestro's 26/10/2026 assignment.
// Keep separate from prototype tests so incompatible limits remain visible.
test('0.13 contract: refinement labels support every level through +15', () => {
  assert.equal(tag(0), '');
  for (let n = 1; n <= 15; n++) assert.equal(tag(n), `[+${n}]`);
  for (const bad of [-1, 16, NaN, 1.5]) assert.throws(() => tag(bad));
});

test('0.13 contract: canonical bosses at each fifth wave, repeat after 50', () => {
  for (let n = 1; n <= 55; n++) {
    const w = wave(n);
    assert.equal(w.number, n);
    assert.equal(w.bossSpecies, n % 5 === 0 ? 100 + ((n / 5 - 1) % 10) : 0);
    assert.ok(w.count > 0 && w.count <= 12, 'arena limits concurrent monsters');
    assert.equal(w.elite, n % 3 === 0, 'every third wave is elite');
    if (n > 1) {
      assert.ok(w.hpBps >= wave(n - 1).hpBps);
      assert.ok(w.attackBps >= wave(n - 1).attackBps);
    }
  }
});

test('0.13 contract: title unlocks at 20 and arena continues beyond 50', () => {
  const arena = new Arena();
  assert.equal(arena.start(0, 1, 7), true);
  let instance = 0;
  for (let n = 1; n <= 51; n++) {
    const w = wave(n);
    const ids = [];
    for (let j = 0; j < w.count; j++) {
      ids.push(++instance);
      assert.equal(arena.registerMonster(instance, j === 0 && w.bossSpecies ? w.bossSpecies : 1), true);
    }
    for (const id of ids) assert.equal(arena.observeKill(1, id, 7, 0), true);
    assert.equal(arena.observeKill(1, ids[0], 7, 0), false, 'no duplicate rewards');
    assert.equal(arena.ledger.slots[0].titleUnlocked, n >= 20);
    assert.equal(arena.ledger.slots[0].bestWave, n);
    assert.equal(arena.active, true, 'endless arena must not finish at 15/30/50');
    assert.equal(encode(decode(encode(arena.ledger))), encode(arena.ledger));
    assert.equal(arena.beginNextWave(), true);
  }
});
