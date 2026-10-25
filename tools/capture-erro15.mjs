import fs from 'node:fs';
import Module from 'node:module';
import assert from 'node:assert/strict';
import {DestiniaWeb} from '../web/dist/engine.mjs';

const root = process.env.DESTINIA_GAME_ROOT;
assert.ok(root, 'DESTINIA_GAME_ROOT obrigatorio');

const vendor = new URL('../web/dist/vendor/unicorn_arm.js', import.meta.url);
const mod = new Module(vendor.pathname);
mod.filename = vendor.pathname;
mod.paths = [];
mod._compile(fs.readFileSync(vendor, 'utf8'), vendor.pathname);
const uc = await mod.exports();

const assets = new Map(
  fs.readdirSync(root + '/assets').map(n => [n, new Uint8Array(fs.readFileSync(root + '/assets/' + n))])
);

const r = new DestiniaWeb(uc, assets, {language: 'pt'});
const B = 0x10000000;
const call = (n, a = []) => r.call(r.exports.get(n), a);
const hex = (a, n) => Buffer.from(r.read(B + a, n)).toString('hex').match(/.{1,2}/g).join(' ');

const lines = [
  '# Relatorio Vanguard: Erro 15 e bytes ARM',
  '',
  'Execucao Node/Unicorn com engine.mjs e ELF original libdestinia_jni.so. Fixture exclusivamente em RAM; nenhum save de usuario foi lido ou escrito no disco.',
  ''
];

const dump = (name, a, n) => {
  lines.push(`## ${name} (offset 0x${a.toString(16)}, ${n} bytes)`, '```text', hex(a, n), '```', '');
};

try {
  r.load(new Uint8Array(fs.readFileSync(root + '/lib/armeabi/libdestinia_jni.so')));
  r.init();
  call('MenuCanvas_init', [r.exports.get('MenuCanvas')]);

  const menu = r.exports.get('Menu');
  const main = r.exports.get('MMain');
  const gameUI = r.exports.get('GameUI');
  const loadGame = r.exports.get('GameCanvas_LoadGame');

  // Selecao de MULTI MODE via handler nativo (Menu.state: 1 -> 8)
  r.put32(menu + 156, 1);
  r.write(menu + 204, new Uint8Array([1, 0])); // cursor = 1 (MULTI MODE)
  call('MenuCanvas_mKeyPressed', [17]);
  assert.equal(r.get32(menu + 156), 8);

  // Configura slot 0 em RAM (nivel 10) e substate > 3
  r.put32(menu + 164, 4);
  r.write(menu + 184, new Uint8Array([10, 0]));
  r.write(menu + 190, new Uint8Array([0, 0]));

  // Mock em RAM de GameCanvas_LoadGame para retornar 1 sem tocar em disco
  r.e.hook_add(
    uc.HOOK_CODE,
    () => {
      r.w(0, 1);
      r.e.reg_write_i32(uc.ARM_REG_PC, r.e.reg_read_i32(uc.ARM_REG_LR));
    },
    null,
    loadGame - 1,
    loadGame - 1
  );

  // Confirma personagem via MenuCanvas_mKeyPressed(17) nativo
  call('MenuCanvas_mKeyPressed', [17]);

  // Se handler nativo concluiu, pendingSendLen = 111 (0x6f)
  console.log('diagnostico', {
    state: r.get32(menu + 156),
    slot: r.get16(menu + 190),
    pending: r.get32(B + 0x73b2c)
  });

  if (!r.get32(B + 0x73b2c)) {
    lines.push('FALLBACK: confirmacao direta Comm_flush em RAM.');
    call('Comm_setConType', [0]);
    call('Comm_init', [0]);
    for (let i = 0; i < 3; i++) call('Comm_write2Bytes', [0]);
    call('Comm_write4Bytes', [0]);
    for (let i = 0; i < 24; i++) call('Comm_write2Bytes', [0]);
    call('Comm_flush');
  } else {
    lines.push('Confirmacao de personagem executada com sucesso via MenuCanvas_mKeyPressed nativo (GameCanvas_LoadGame em RAM).');
  }

  assert.equal(r.get32(B + 0x73b2c), 111);

  dump('Comm.sendBuf cmd=0 (111 bytes)', 0x70bd0, 111);
  dump('outPacket completo (offset 0x6dcb4)', 0x6dcb4, 12008);
  dump('Comm.pendingSendLen LE', 0x73b2c, 4);

  lines.push(`Estado apos confirmacao: Menu=${r.get32(menu + 156)}, netState=${r.get16(main + 0x3920)}, timer=${r.get32(main + 0x3924)}.`);

  // Passagem de frames ate frame 151 para comprovar Erro 15
  for (let frame = 1; frame <= 151; frame++) {
    r.step();
    if (frame >= 150) {
      lines.push(`Frame relativo ${frame}: netState=${r.get16(main + 0x3920)}, timer=${r.get32(main + 0x3924)}, Menu=${r.get32(menu + 156)}.`);
    }
    if (frame === 150) assert.equal(r.get16(main + 0x3920), 1);
  }

  assert.equal(r.get16(main + 0x3920), 2);
  assert.match(r.cstr(B + 0x81cf8), /15/);
  lines.push('', `Mensagem capturada: ${JSON.stringify(r.cstr(B + 0x81cf8))}`);
  dump('netData+260 incluindo terminador', 0x81cf8, Buffer.byteLength(r.cstr(B + 0x81cf8)) + 1);

  if (process.argv.includes('--responses')) {
    const reply = (cmd, payload) => {
      // Quadro de entrada de Comm_netSocketRead: totalLen = 8 + payload.length
      const totalLen = 8 + payload.length;
      const b = Buffer.alloc(totalLen);
      b.writeUInt32BE(totalLen, 0);
      b.writeUInt32BE(cmd, 4);
      payload.copy(b, 8);
      lines.push(`Resposta injetada cmd=${cmd} (totalLen=${totalLen}): ` + b.toString('hex').match(/.{1,2}/g).join(' '));
      call('Java_game_destiniaeng_GameThread_procCommMsg', [0x40000000, 16, 1, 0]);
      call('Java_game_destiniaeng_GameThread_procCommRead', [0x40000000, 16, r.array(b), b.length]);
      for (let i = 0; i < 4; i++) call('Comm_mRun');
    };

    // O loop de logo da fixture pode substituir Comm.processCB; restaura o menu nativo.
    call('MenuCanvas_init', [r.exports.get('MenuCanvas')]);
    r.put32(B + 0x70bc0, 0);
    assert.equal(r.get32(B + 0x70b9c), r.exports.get('Menu_netProc'));
    // Reinicia estado de espera de rede a partir de Menu.state = 8
    r.put32(menu + 156, 8);
    call('Comm_setConType', [0]);
    call('Comm_init', [0]);
    call('Comm_flush');

    const p0 = Buffer.from('000000010000000844455354494e4941000000024f4b0000', 'hex');
    r.put32(B + 0x73b2c, 0);
    reply(0, p0);
    assert.equal(r.get32(B + 0x73b2c), 53);
    assert.equal(Buffer.from(r.read(B + 0x70bd0, 8)).readUInt32BE(4), 8);
    assert.equal(r.get32(B + 0x45108), 0);
    dump('Request automatico cmd=8 (53 bytes)', 0x70bd0, 53);

    const p8 = Buffer.from('000000010000', 'hex');
    reply(8, p8);

    const curMenu = r.get32(menu + 156);
    const curGameUI = r.get32(gameUI + 124) >>> 0;
    lines.push(`Estado apos respostas: Menu=${curMenu}; GameUI+124=0x${curGameUI.toString(16)}`);
    dump('inPacket inicio apos respostas', 0x6adc8, 64);

    assert.equal(curMenu, 20);
    assert.equal(curGameUI, 0xa0000001);
    lines.push(`PASS: Menu.state=20; GameUI+124=0x${curGameUI.toString(16)}. Respostas processadas com sucesso.`);
  }

  fs.writeFileSync(new URL('../docs/RELATORIO_VANGUARD_ERRO15_BYTES.md', import.meta.url), lines.join('\n'));
  console.log(lines.filter(line => line.length < 500).join('\n'));
} catch (error) {
  lines.push('', '## Resultado incompleto', String(error));
  fs.writeFileSync(new URL('../docs/RELATORIO_VANGUARD_ERRO15_BYTES.md', import.meta.url), lines.join('\n'));
  console.error(String(error));
  process.exitCode = 1;
} finally {
  r.close();
}


