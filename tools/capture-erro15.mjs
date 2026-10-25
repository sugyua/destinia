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
r.write(menu+184,new Uint8Array([10,0]));r.write(menu+190,new Uint8Array([0,0]));
r.e.hook_add(uc.HOOK_CODE,()=>console.log('ARM slot check',Array.from({length:8},(_,i)=>r.r(i).toString(16))),null,B+0x22a04,B+0x22a12);call('MenuCanvas_mKeyReleased',[17]);call('MenuCanvas_mKeyPressed',[17]);
console.log('diagnostico',Object.fromEntries(['Menu','Comm','outPacket','netData','MMain'].map(n=>[n,(r.exports.get(n)-B).toString(16)])), 'state',r.get32(menu+156),'slot',r.get16(menu+190),'levels',hex(menu-B+184,8),'pending',r.get32(B+0x73b2c));if(!r.get32(B+0x73b2c)){lines.push('LIMITACAO: confirmacao via handler nao publicou pacote; captura abaixo usa Comm_init/Comm_write/Comm_flush ARM diretamente com payload zero controlado, nao request de personagem real.');call('Comm_setConType',[0]);call('Comm_init',[0]);for(let i=0;i<3;i++)call('Comm_write2Bytes',[0]);call('Comm_write4Bytes',[0]);for(let i=0;i<24;i++)call('Comm_write2Bytes',[0]);call('Comm_flush');}assert.equal(r.get32(B+0x73b2c),111);
dump('Comm.sendBuf cmd=0',0x70bd0,111);dump('outPacket completo; layout do contrato ainda nao validado',0x6dcb4,12008);dump('Comm.pendingSendLen LE',0x73b2c,4);
lines.push(`Estado apos confirmacao: Menu=${r.get32(menu+156)}, netState=${r.get16(main+0x3920)}, timer=${r.get32(main+0x3924)}.`);
for(let frame=1;frame<=151;frame++) {
r.step();
if(frame>=150)lines.push(`Frame relativo ${frame}: netState=${r.get16(main+0x3920)}, timer=${r.get32(main+0x3924)}, Menu=${r.get32(menu+156)}.`);
if(frame===150)assert.equal(r.get16(main+0x3920),1);
}
assert.equal(r.get16(main+0x3920),2);assert.match(r.cstr(B+0x81cf8),/15/);
lines.push('',`Mensagem capturada: ${JSON.stringify(r.cstr(B+0x81cf8))}`);dump('netData+260 incluindo terminador',0x81cf8,Buffer.byteLength(r.cstr(B+0x81cf8))+1);
lines.push('Retomada apos reset de cota. Telemetria de percentual atual indisponivel.','');
if(process.argv.includes('--responses')){
const reply=(cmd,payload)=>{const b=Buffer.alloc(8+payload.length);b.writeUInt32BE(b.length);b.writeUInt32BE(cmd,4);payload.copy(b,8);lines.push('Resposta cmd='+cmd+': '+b.toString('hex').match(/.{1,2}/g).join(' '));call('Java_game_destiniaeng_GameThread_procCommMsg',[0x40000000,16,1,0]);call('Java_game_destiniaeng_GameThread_procCommRead',[0x40000000,16,r.array(b),b.length]);console.log('apos procCommRead',cmd,'socket',r.get32(B+0x45108),'comm',hex(0x70bbc,24),'hdr',hex(0x49f4c,32));for(let i=0;i<4;i++){call('Comm_mRun');console.log('comm tick',i,'socket',r.get32(B+0x45108),'comm',hex(0x70bbc,24),'inHead',hex(0x6adc8,32),'inTail',hex(0x6adc8+12000,12));}};
r.e.hook_add(uc.HOOK_CODE,()=>console.log('Menu_netProc',r.r(0),r.r(1).toString(16)),null,B+0x22d6c,B+0x22d6c);console.log('callback',r.get32(B+0x70b9c).toString(16));r.put32(B+0x70bc0,0);r.put32(menu+156,8);call('Comm_setConType',[0]);call('Comm_init',[0]);call('Comm_flush');
const p=Buffer.from('000000010000000844455354494e4941000000024f4b0000','hex');r.put32(B+0x73b2c,0);reply(0,p);assert.equal(r.get32(B+0x73b2c),53);assert.equal(Buffer.from(r.read(B+0x70bd0,8)).readUInt32BE(4),8);assert.equal(r.get32(B+0x45108),0);dump('Request automatico cmd=8',0x70bd0,53);reply(8,Buffer.from('000000010000','hex'));lines.push('Estado apos respostas: Menu='+r.get32(menu+156)+'; GameUI=0x'+r.get32(r.exports.get('GameUI')+124).toString(16));dump('inPacket inicio apos respostas',0x6adc8,64);assert.equal(r.get32(B+0x45108),0);assert.equal(r.get32(menu+156),20);assert.equal(r.get32(r.exports.get('GameUI')+124),0xa0000001);lines.push('PASS: Menu.state=20; GameUI+124=0xa0000001. Respostas sem newNetUserId opcional.');
}
fs.writeFileSync(new URL('../docs/RELATORIO_VANGUARD_ERRO15_BYTES.md',import.meta.url),lines.join('\n'));
console.log(lines.filter(line=>!line.startsWith('00 ')&&line.length<500).join('\n'));
} catch(error) {lines.push('', '## Resultado incompleto', String(error), 'A transicao JNI cmd0/cmd8 NAO foi comprovada. pendingSendLen=53 isolado nao prova que cmd0 foi processado, pois a requisicao de preparacao ja tinha esse tamanho. O contrato estatico exige revisao: prefixo real 0x6f versus 0x6b previsto; layout outPacket nao confirmado. Confirmacao de personagem pelo handler tambem permanece pendente. Nenhuma alteracao em gameplay, saves no disco, release ou site.');fs.writeFileSync(new URL('../docs/RELATORIO_VANGUARD_ERRO15_BYTES.md',import.meta.url),lines.join('\n'));console.error(String(error));process.exitCode=1;} finally {r.close();}











