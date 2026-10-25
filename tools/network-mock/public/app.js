'use strict';
let token = null, state = null, polling = false;
const el = id => document.getElementById(id), status = text => { el('status').textContent = text; };
async function api(endpoint, data) {
  const response = await fetch('/api/' + endpoint, { method: data === undefined ? 'GET' : 'POST', headers: { 'Content-Type': 'application/json', ...(token ? { Authorization: 'Bearer ' + token } : {}) }, ...(data === undefined ? {} : { body: JSON.stringify(data) }), signal: AbortSignal.timeout(10000) });
  const result = await response.json();
  if (!response.ok) { if (response.status === 401) { token = null; el('session').hidden = true; el('join').hidden = false; } throw Error(result.error || 'Falha no servidor.'); }
  return result;
}
function render(value) {
  state = value; el('session').hidden = false; el('join').hidden = true;
  el('room').textContent = 'Sala: ' + state.room;
  el('players').replaceChildren(...state.players.map(p => { const li = document.createElement('li'); li.textContent = `${p.name}${p.id === state.me ? ' (você)' : ''} — HP ${p.hp}/100 — Gold fictício ${p.gold}`; return li; }));
  const a = state.auction, winner = state.players.find(p => p.id === a.bidder);
  el('auction').textContent = `${a.item} — Lance: ${a.amount} — ${winner?.name || 'sem lance'} — ${a.closed ? 'ENCERRADO' : 'ABERTO'}`;
  el('events').replaceChildren(...[...state.events].reverse().map(e => { const li = document.createElement('li'); li.textContent = e.text; return li; }));
  el('attack').disabled = state.players.length < 2; el('close').disabled = a.closed; el('bid').querySelector('button').disabled = a.closed;
}
async function run(action) { try { await action(); } catch (e) { status(e.message); } }
el('join').onsubmit = event => { event.preventDefault(); run(async () => { const data = Object.fromEntries(new FormData(event.target)); const value = await api('onJoin', data); token = value.token; render(value); status('Conectado ao simulador.'); }); };
async function send(endpoint, data = {}) { render(await api(endpoint, data)); status('Ação confirmada pelo servidor.'); }
el('attack').onclick = () => run(() => send('onPlayerAttack', { targetId: state.players.find(p => p.id !== state.me)?.id }));
el('hp').onclick = () => run(() => send('resetHp'));
el('bid').onsubmit = event => { event.preventDefault(); run(() => send('onAuctionBid', { amount: Number(new FormData(event.target).get('amount')) })); };
el('close').onclick = () => run(() => send('onAuctionClose'));
el('leave').onclick = () => run(async () => { await api('leave', {}); token = null; state = null; el('session').hidden = true; el('join').hidden = false; status('Desconectado.'); });
setInterval(async () => { if (!token || polling) return; polling = true; try { render(await api('state')); } catch (e) { status('Conexão: ' + e.message); } finally { polling = false; } }, 1000);
// Token somente na memória desta aba; recarregar inicia nova sessão.
