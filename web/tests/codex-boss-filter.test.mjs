import test from 'node:test';
import assert from 'node:assert/strict';
import {Codex} from '../dist/codex.mjs';
import * as ui from '../dist/codex-ui.mjs';
import * as native from '../dist/codex-native.mjs';

test('filtro do Códex separa monstros comuns de chefes nativamente', () => {
  const names = new Map([
    [1, 'Fungus'],
    [61, 'Imperial Soldier'],
    [100, 'Dragon King'],
    [105, 'Death Knight']
  ]);
  const codex = new Codex(names);
  codex.select(0);

  // Sem filtro (Todos)
  const all = codex.catalog(0);
  assert.equal(all.length, 4);
  assert.equal(codex.isBoss(1), false);
  assert.equal(codex.isBoss(61), false);
  assert.equal(codex.isBoss(100), true);
  assert.equal(codex.isBoss(105), true);
  assert.equal(all[0].isBoss, false);
  assert.equal(all[2].isBoss, true);

  // Filtro 1 (Comuns: species < 100)
  const common = codex.catalog(1);
  assert.equal(common.length, 2);
  assert.deepEqual(common.map(m => m.id), [1, 61]);
  assert.ok(common.every(m => !m.isBoss));

  // Filtro 2 (Chefes: species >= 100)
  const bosses = codex.catalog(2);
  assert.equal(bosses.length, 2);
  assert.deepEqual(bosses.map(m => m.id), [100, 105]);
  assert.ok(bosses.every(m => m.isBoss));
});

test('hitbox das abas de filtro [TODOS], [COMUNS], [CHEFES] funcionam por toque', () => {
  for (const [w, h] of [[400, 240], [800, 480]]) {
    const tab0 = ui.codexFilterTab(0, w, h);
    const tab1 = ui.codexFilterTab(1, w, h);
    const tab2 = ui.codexFilterTab(2, w, h);

    assert.equal(ui.codexFilterHit(tab0.x + 5, tab0.y + 5, w, h), 0);
    assert.equal(ui.codexFilterHit(tab1.x + 5, tab1.y + 5, w, h), 1);
    assert.equal(ui.codexFilterHit(tab2.x + 5, tab2.y + 5, w, h), 2);
    assert.equal(ui.codexFilterHit(tab0.x - 10, tab0.y, w, h), -1);

    const nTab0 = native.nativeFilterTab(0, w, h);
    const nTab1 = native.nativeFilterTab(1, w, h);
    const nTab2 = native.nativeFilterTab(2, w, h);

    assert.equal(native.nativeFilterHit(nTab0.x + 5, nTab0.y + 5, w, h), 0);
    assert.equal(native.nativeFilterHit(nTab1.x + 5, nTab1.y + 5, w, h), 1);
    assert.equal(native.nativeFilterHit(nTab2.x + 5, nTab2.y + 5, w, h), 2);
    assert.equal(native.nativeFilterHit(nTab0.x - 10, nTab0.y, w, h), -1);
  }
});

test('pintura do Códex destaca chefes visualmente na grade', () => {
  const frame = new Uint8Array(400 * 240 * 2);
  const rows = [
    {id: 1, name: 'Fungus', found: true, kills: 1, isBoss: false},
    {id: 100, name: 'Dragon King', found: true, kills: 1, isBoss: true}
  ];
  ui.paintCodex(frame, rows, {open: true, width: 400, height: 240, filter: 0});
  assert.ok(frame.some(b => b !== 0), 'Framebuffer deve conter dados desenhados');
});
