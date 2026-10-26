'use strict';
const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const { randomUUID } = require('node:crypto');

// Estado fictício em memória. Nunca lê biblioteca ARM, inventário ou saves.
function createMockServer() {
  const rooms = new Map(), players = new Map();
  const idleMs = 120_000;
  function fail(status, message) { const e = new Error(message); e.status = status; throw e; }
  function roomFor(player) { return rooms.get(player.room); }
  function event(room, text) { room.events.push({ at: Date.now(), text }); if (room.events.length > 40) room.events.shift(); }
  function leave(player) {
    const room = roomFor(player); players.delete(player.token); room.players.delete(player.id);
    event(room, `${player.name} saiu.`);
    // Lance de participante desconectado é cancelado neste mock, sem dinheiro real.
    if (room.auction.bidder === player.id && !room.auction.closed) {
      room.auction.bidder = null; room.auction.amount = 0; event(room, 'Lance cancelado: participante saiu.');
    }
    if (!room.players.size) rooms.delete(player.room);
  }
  function expire() { const now = Date.now(); for (const p of players.values()) if (now - p.seen > idleMs) leave(p); }
  function snapshot(room, me) {
    return { me: me.id, room: me.room, players: [...room.players.values()].map(({ id, name, hp, gold }) => ({ id, name, hp, gold })),
      auction: { ...room.auction }, events: room.events, simulation: true };
  }
  async function body(req) {
    let size = 0, chunks = [];
    for await (const chunk of req) {
      size += chunk.length; if (size > 4096) fail(413, 'Mensagem grande demais.'); chunks.push(chunk);
    }
    try { const value = JSON.parse(Buffer.concat(chunks).toString()); if (!value || typeof value !== 'object' || Array.isArray(value)) throw Error(); return value; }
    catch { fail(400, 'JSON inválido.'); }
  }
  const server = http.createServer(async (req, res) => {
    res.setHeader('Cache-Control', 'no-store');
    res.setHeader('X-Content-Type-Options', 'nosniff');
    res.setHeader('Referrer-Policy', 'no-referrer');
    res.setHeader('Content-Security-Policy', "default-src 'self'; connect-src 'self'; script-src 'self'; style-src 'self'; frame-ancestors 'none'; base-uri 'none'");
    const json = (status, data) => { res.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' }); res.end(JSON.stringify(data)); };
    try {
      expire();
      const url = new URL(req.url, 'http://localhost');
      const files = { '/': ['index.html','text/html'], '/app.js': ['app.js','text/javascript'], '/style.css': ['style.css','text/css'] };
      if (req.method === 'GET' && files[url.pathname]) {
        const [file, type] = files[url.pathname]; res.writeHead(200, { 'Content-Type': type + '; charset=utf-8' });
        res.end(fs.readFileSync(path.join(__dirname, 'public', file))); return;
      }
      if (req.method === 'GET' && url.pathname === '/health') { json(200, { ok: true, simulation: true }); return; }
      // Mesmo Origin quando presente; permitir também origens locais de teste (localhost e 127.0.0.1)
      if (req.headers.origin) {
        const origin = new URL(req.headers.origin);
        const isLocal = origin.hostname === 'localhost' || origin.hostname === '127.0.0.1';
        if (origin.host !== req.headers.host && !isLocal) fail(403, 'Origem não permitida.');
        res.setHeader('Access-Control-Allow-Origin', req.headers.origin);
        res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
        res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization');
      }
      if (req.method === 'OPTIONS') { res.writeHead(204); res.end(); return; }
      if (req.method === 'POST' && url.pathname === '/api/onJoin') {
        const data = await body(req);
        if (typeof data.name !== 'string' || !data.name.trim() || data.name.trim().length > 24) fail(400, 'Nome: 1 a 24 caracteres.');
        if (typeof data.room !== 'string' || !/^[a-zA-Z0-9_-]{1,40}$/.test(data.room)) fail(400, 'Sala: 1 a 40 letras, números, _ ou -.');
        if (players.size >= 64 || (!rooms.has(data.room) && rooms.size >= 32)) fail(503, 'Limite do simulador atingido.');
        let room = rooms.get(data.room);
        if (room && room.players.size >= 2) fail(409, 'Sala cheia: máximo de dois jogadores.');
        if (!room) { room = { players: new Map(), events: [], auction: { item: 'Espada de teste (fictícia)', amount: 0, bidder: null, closed: false } }; rooms.set(data.room, room); }
        const player = { id: randomUUID(), token: randomUUID(), name: data.name.trim(), room: data.room, hp: 100, gold: 1000, seen: Date.now(), lastAttack: 0, lastWrite: 0 };
        players.set(player.token, player); room.players.set(player.id, player); event(room, `${player.name} entrou.`);
        json(201, { token: player.token, ...snapshot(room, player) }); return;
      }
      const token = (req.headers.authorization || '').replace(/^Bearer /, '');
      const player = players.get(token); if (!player) fail(401, 'Sessão ausente ou expirada. Entre novamente.');
      player.seen = Date.now(); const room = roomFor(player);
      if (req.method === 'GET' && url.pathname === '/api/state') { json(200, snapshot(room, player)); return; }
      if (req.method !== 'POST') fail(404, 'Endpoint inexistente.');
      const data = await body(req), now = Date.now();
      if (now - player.lastWrite < 100) fail(429, 'Aguarde antes da próxima ação.');
      player.lastWrite = now;
      switch (url.pathname) {
        case '/api/onPlayerAttack': {
          const target = room.players.get(data.targetId);
          if (!target || target.id === player.id) fail(400, 'Escolha um oponente nesta sala.');
          if (player.hp <= 0 || target.hp <= 0) fail(409, 'Duelo encerrado. Os dois devem usar Restaurar HP.');
          if (now - player.lastAttack < 500) fail(429, 'Intervalo entre ataques: 500 ms.');
          player.lastAttack = now; target.hp = Math.max(0, target.hp - 10);
          event(room, `${player.name} atacou ${target.name}: -10 HP${target.hp === 0 ? ' — vitória!' : ''}`); break;
        }
        case '/api/onAuctionBid': {
          if (room.auction.closed) fail(409, 'Leilão encerrado nesta sala.');
          if (!Number.isSafeInteger(data.amount) || data.amount <= room.auction.amount || data.amount > player.gold) fail(400, 'Lance inteiro, maior que o atual e dentro do saldo fictício.');
          room.auction.amount = data.amount; room.auction.bidder = player.id;
          event(room, `${player.name} deu lance de ${data.amount} gold fictício.`); break;
        }
        case '/api/onAuctionClose': {
          if (room.auction.closed) fail(409, 'Leilão já encerrado.');
          const winner = room.players.get(room.auction.bidder); if (!winner) fail(409, 'Ainda não há lance válido.');
          winner.gold -= room.auction.amount; room.auction.closed = true;
          event(room, `Leilão finalizado: ${winner.name}, ${room.auction.amount} gold fictício. Nenhum item real entregue.`); break;
        }
        case '/api/resetHp': player.hp = 100; event(room, `${player.name} restaurou seu HP de teste.`); break;
        case '/api/leave': leave(player); json(200, { ok: true }); return;
        default: fail(404, 'Endpoint inexistente.');
      }
      json(200, snapshot(room, player));
    } catch (e) { if (!res.headersSent) json(e.status || 500, { error: e.status ? e.message : 'Erro interno do simulador.' }); else res.end(); }
  });
  server.requestTimeout = 10_000; server.headersTimeout = 10_000;
  return server;
}
if (require.main === module) {
  const port = Number(process.env.PORT || 3000);
  if (!Number.isInteger(port) || port < 1 || port > 65535) throw Error('PORT inválida.');
  const server = createMockServer();
  server.on('error', e => { console.error(`Não foi possível iniciar: ${e.message}`); process.exitCode = 1; });
  // O túnel local acessa loopback; não expõe a rede doméstica diretamente.
  server.listen(port, '127.0.0.1', () => console.log(`SIMULAÇÃO independente: http://127.0.0.1:${port} — Ctrl+C encerra; tudo é temporário.`));
}
module.exports = { createMockServer };
