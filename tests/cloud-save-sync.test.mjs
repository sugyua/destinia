import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import Module from 'node:module';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const root = process.env.DESTINIA_GAME_ROOT || 'D:/IA/Meus Aplicativos/destinia/game';

const { DestiniaWeb } = await import('../web/dist/engine.mjs');
const vendorPath = path.resolve(__dirname, '../web/dist/vendor/unicorn_arm.js');

const m = new Module(vendorPath);
m.filename = vendorPath;
m.paths = [];
m._compile(fs.readFileSync(vendorPath, 'utf8'), vendorPath);
const uc = await m.exports();

const assets = new Map(
  fs.readdirSync(path.join(root, 'assets')).map(n => [n, new Uint8Array(fs.readFileSync(path.join(root, 'assets', n)))])
);
const elf = fs.readFileSync(path.join(root, 'lib/armeabi/libdestinia_jni.so'));

console.log('=== Teste de Integracao: Sync Google Drive <-> Web Engine ===');

// 1. Testa GET /api/save
const getRes = await fetch('http://localhost:8000/api/save?format=json');
assert.equal(getRes.status, 200, 'GET /api/save deve retornar status 200');
const json = await getRes.json();
assert.ok(json.ok);
assert.ok(json.saves['save2.dat'], 'save2.dat deve estar presente no Google Drive');
console.log('[OK] Google Drive possui save2.dat com tamanho base64:', json.saves['save2.dat'].length);

// 2. Instancia a engine e executa loadCloudSaves()
const engine = new DestiniaWeb(uc, assets, { language: 'pt' });
await engine.loadCloudSaves();
const loadedSave = engine.saved.get('/data/data/game.destiniaeng/files/save2.dat');
assert.ok(loadedSave, 'save2.dat deve estar mapeado em engine.saved');
assert.equal(loadedSave.length, 1569, 'Tamanho do save2.dat deve bater com o PC (1569 bytes)');
console.log('[OK] engine.saved carregou save2.dat com 1569 bytes');

// 3. Inicializa ARM e testa GameCanvas_LoadGame(2)
engine.load(elf);
engine.init();

const loadResult = engine.call(engine.exports.get('GameCanvas_LoadGame'), [2]);
assert.equal(loadResult, 1, 'GameCanvas_LoadGame(2) deve carregar o save com sucesso (retorno 1)');
console.log('[OK] GameCanvas_LoadGame(2) carregou o save do PC no Unicorn com retorno 1');

// 4. Testa persistencia com POST /api/save
const dummyName = 'save_sync_verify.dat';
const dummyData = new Uint8Array([0xde, 0xad, 0xbe, 0xef, 0x01, 0x02, 0x03, 0x04]);
engine.saved.set(dummyName, dummyData);
engine.persistCloudSave(dummyName);

// Aguarda 300ms para a requisicao fetch resolver
await new Promise(r => setTimeout(r, 300));

// Le de volta via API para garantir gravacao no Google Drive
const verifyRes = await fetch(`http://localhost:8000/api/save?file=${dummyName}`);
assert.equal(verifyRes.status, 200);
const verifyBuf = Buffer.from(await verifyRes.arrayBuffer());
assert.deepEqual(Array.from(verifyBuf), Array.from(dummyData));
console.log('[OK] persistCloudSave() gravou dummy save no Google Drive e leu de volta com exatidao');

// Limpa o dummy save
const driveFolder = 'C:/Users/Sugya/Google Drive/DestiniaSaves';
const dummyPath = path.join(driveFolder, dummyName);
if (fs.existsSync(dummyPath)) fs.unlinkSync(dummyPath);

engine.close();
console.log('=== Sincronizacao Google Drive <-> Web Engine APROVADA 100%! ===');
