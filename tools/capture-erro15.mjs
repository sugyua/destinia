import fs from 'node:fs';
import Module from 'node:module';
import assert from 'node:assert/strict';
import {DestiniaWeb} from '../web/dist/engine.mjs';
const root=process.env.DESTINIA_GAME_ROOT;
assert.ok(root,'DESTINIA_GAME_ROOT obrigatorio');
const vendor=new URL('../web/dist/vendor/unicorn_arm.js',import.meta.url);
const mod=new Module(vendor.pathname);mod.filename=vendor.pathname;mod.paths=[];
mod._compile(fs.readFileSync(vendor,'utf8'),vendor.pathname);
const uc=await mod.exports();
const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
const r=new DestiniaWeb(uc,assets,{language:'pt'}),B=0x10000000;
const call=(n,a=[])=>r.call(r.exports.get(n),a);
const hex=(a,n)=>Buffer.from(r.read(B+a,n)).toString('hex').match(/.{1,2}/g).join(' ');
const lines=['# Relatorio Vanguard: Erro 15 e bytes ARM','', 'Execucao Node/Unicorn com engine.mjs e ELF original. Fixture exclusivamente em RAM; nenhum save de usuario foi lido ou escrito. Slot sem arquivo: nivel da selecao definido em RAM para exercitar confirmacao; isto nao equivale a validar personagem persistido.',''];
const dump=(name,a,n)=>lines.push(`## ${name} (offset 0x${a.toString(16)}, ${n} bytes)`, '```text',hex(a,n),'```','');
try {
r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();
call('MenuCanvas_init',[r.exports.get('MenuCanvas')]);
const menu=r.exports.get('Menu'),main=r.exports.get('MMain');
// Select MULTI MODE through native handler, then allow an ephemeral empty slot.
r.put32(menu+156,1);r.write(menu+204,new Uint8Array([1,0]));
call('MenuCanvas_mKeyPressed',[17]);assert.equal(r.get32(menu+156),8);
r.write(menu+184,new Uint8Array([1,0]));r.write(menu+190,new Uint8Array([0,0]));
call('MenuCanvas_mKeyPressed',[17]);
console.log('diagnostico',Object.fromEntries(['Menu','Comm','outPacket','netData','MMain'].map(n=>[n,(r.exports.get(n)-B).toString(16)])), 'state',r.get32(menu+156),'slot',r.get16(menu+190),'levels',hex(menu-B+184,8),'pending',r.get32(B+0x73b2c));assert.equal(r.get32(B+0x73b2c),111);
dump('Comm.sendBuf cmd=0',0x70bd0,111);dump('outPacket: header LE + payload BE',0x6dcb4,66);dump('Comm.pendingSendLen LE',0x73b2c,4);
lines.push(`Estado apos confirmacao: Menu=${r.get32(menu+156)}, netState=${r.get16(main+0x3920)}, timer=${r.get32(main+0x3924)}.`);
for(let frame=1;frame<=151;frame++) {
call('MMain_run');
if(frame>=150)lines.push(`Frame relativo ${frame}: netState=${r.get16(main+0x3920)}, timer=${r.get32(main+0x3924)}, Menu=${r.get32(menu+156)}.`);
if(frame===150)assert.equal(r.get16(main+0x3920),1);
}
assert.equal(r.get16(main+0x3920),2);assert.match(r.cstr(B+0x81cf8),/15/);
lines.push('',`Mensagem capturada: ${JSON.stringify(r.cstr(B+0x81cf8))}`);dump('netData+260 incluindo terminador',0x81cf8,Buffer.byteLength(r.cstr(B+0x81cf8))+1);
lines.push('Cota de 5h: sem telemetria acessivel; 18% informado pelo usuario no inicio. Nenhuma afirmacao de percentual atual.','');
fs.writeFileSync(new URL('../docs/RELATORIO_VANGUARD_ERRO15_BYTES.md',import.meta.url),lines.join('\n'));
console.log(lines.join('\n'));
} finally {r.close();}

