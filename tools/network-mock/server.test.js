const { test } = require('node:test');
const assert = require('node:assert/strict');
const { createMockServer } = require('./server');
test('Dois clientes: sala, duelo, leilão, isolamento e entradas inválidas', async t => {
  const server = createMockServer(); await new Promise(r => server.listen(0, '127.0.0.1', r));
  t.after(() => server.close()); const base = `http://127.0.0.1:${server.address().port}`;
  async function api(endpoint, data, token, headers = {}) {
    const r = await fetch(base + endpoint, { method: data === undefined ? 'GET' : 'POST', headers: { 'Content-Type': 'application/json', ...(token ? { Authorization: 'Bearer ' + token } : {}), ...headers }, ...(data === undefined ? {} : { body: JSON.stringify(data) }) });
    return { status: r.status, body: await r.json() };
  }
  const join = async (name, room = 'primos') => (await api('/api/onJoin', { name, room })).body;
  const a = await join('Autor'), b = await join('Primo'), c = await join('Outro', 'isolada');
  assert.equal((await api('/api/onJoin', { name: 'Terceiro', room: 'primos' })).status, 409);
  assert.equal((await api('/api/state', undefined, a.token)).body.players.length, 2);
  assert.equal((await api('/api/state', undefined, c.token)).body.players.length, 1);
  assert.equal((await api('/api/state')).status, 401);
  assert.equal((await api('/api/onJoin', { name: 'X', room: '../' })).status, 400);
  assert.equal((await api('/api/onJoin', { name: 'X', room: 'ok' }, null, { Origin: 'https://estranho.example' })).status, 403);
  assert.equal((await api('/api/onPlayerAttack', { targetId: c.me }, a.token)).status, 400);
  await new Promise(r => setTimeout(r, 110));
  const attack = await api('/api/onPlayerAttack', { targetId: b.me, damage: 999 }, a.token);
  assert.equal(attack.body.players.find(p => p.id === b.me).hp, 90); // dano decidido pelo servidor
  assert.equal((await api('/api/onPlayerAttack', { targetId: b.me }, a.token)).status, 429);
  await new Promise(r => setTimeout(r, 110));
  assert.equal((await api('/api/onAuctionBid', { amount: 1001 }, a.token)).status, 400);
  await new Promise(r => setTimeout(r, 110));
  assert.equal((await api('/api/onAuctionBid', { amount: 100 }, a.token)).status, 200);
  assert.equal((await api('/api/state', undefined, b.token)).body.auction.amount, 100);
  assert.equal((await api('/api/onAuctionBid', { amount: 99 }, b.token)).status, 400);
  await new Promise(r => setTimeout(r, 110));
  assert.equal((await api('/api/onAuctionBid', { amount: 150 }, b.token)).status, 200);
  assert.equal((await api('/api/onAuctionClose', {}, a.token)).body.players.find(p => p.id === b.me).gold, 850);
  await new Promise(r => setTimeout(r, 110));
  assert.equal((await api('/api/onAuctionClose', {}, a.token)).status, 409);
  assert.equal((await api('/api/state', undefined, b.token)).body.players.find(p => p.id === b.me).gold, 850);
  const response = await fetch(base + '/api/onJoin', { method: 'POST', body: '{' }); assert.equal(response.status, 400);
  assert.equal((await fetch(base + '/server.js')).status, 401); // não serve arquivos fora da allowlist
  assert.equal((await fetch(base + '/')).status, 200);
  await new Promise(r => setTimeout(r, 110));
  await api('/api/leave', {}, b.token);
  assert.equal((await api('/api/state', undefined, a.token)).body.players.length, 1);
  assert.equal((await api('/api/state', undefined, b.token)).status, 401);
});
