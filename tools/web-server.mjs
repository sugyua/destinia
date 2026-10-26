import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const PORT = Number(process.env.PORT || 8000);
const WEB_ROOT = path.resolve(__dirname, '../web/dist');
const SAVE_DIR = process.env.DESTINIA_SAVE_DIR || 'C:/Users/Sugya/Google Drive/DestiniaSaves';
const MOCK_URL = 'http://127.0.0.1:3000';

const MIME_TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.mjs': 'application/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.txt': 'text/plain; charset=utf-8',
  '.bin': 'application/octet-stream',
  '.so': 'application/octet-stream',
  '.wasm': 'application/wasm',
  '.png': 'image/png'
};

function safeFileName(name) {
  const base = path.basename(name || 'save2.dat');
  return /^[a-zA-Z0-9._-]+$/.test(base) ? base : 'save2.dat';
}

function ensureSaveDir() {
  if (!fs.existsSync(SAVE_DIR)) {
    fs.mkdirSync(SAVE_DIR, { recursive: true });
  }
}

export function createWebServer() {
  ensureSaveDir();

  const server = http.createServer(async (req, res) => {
    res.setHeader('Cache-Control', 'no-store');
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin || '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization, X-Save-Name');

    if (req.method === 'OPTIONS') {
      res.writeHead(204);
      res.end();
      return;
    }

    const parsedUrl = new URL(req.url || '/', `http://${req.headers.host || 'localhost'}`);
    const pathname = parsedUrl.pathname;

    // --- ROTA DE SAVE: GET /api/save ---
    if (req.method === 'GET' && pathname === '/api/save') {
      ensureSaveDir();
      const format = parsedUrl.searchParams.get('format');
      const reqFile = parsedUrl.searchParams.get('file');

      // Formato JSON com todos os saves disponíveis
      if (format === 'json' || parsedUrl.searchParams.get('all') === '1') {
        const files = ['save2.dat', 'save0.dat', 'save1.dat', 'msave.dat', 'codex-v1.txt'];
        const saves = {};
        for (const file of files) {
          const filePath = path.join(SAVE_DIR, file);
          if (fs.existsSync(filePath)) {
            saves[file] = fs.readFileSync(filePath).toString('base64');
          }
        }
        res.writeHead(200, { 'Content-Type': 'application/json; charset=utf-8' });
        res.end(JSON.stringify({ ok: true, saves }));
        return;
      }

      // Download de um arquivo específico em formato binário
      const fileName = safeFileName(reqFile || 'save2.dat');
      const targetPath = path.join(SAVE_DIR, fileName);

      if (!fs.existsSync(targetPath)) {
        res.writeHead(404, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ ok: false, error: `Arquivo ${fileName} nao encontrado no Drive.` }));
        return;
      }

      const fileBytes = fs.readFileSync(targetPath);
      res.writeHead(200, {
        'Content-Type': 'application/octet-stream',
        'Content-Length': fileBytes.length,
        'X-Save-Name': fileName
      });
      res.end(fileBytes);
      return;
    }

    // --- ROTA DE SAVE: POST /api/save ---
    if (req.method === 'POST' && pathname === '/api/save') {
      ensureSaveDir();
      const chunks = [];
      let totalSize = 0;

      for await (const chunk of req) {
        totalSize += chunk.length;
        if (totalSize > 50 * 1024 * 1024) {
          res.writeHead(413, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify({ ok: false, error: 'Payload de save muito grande.' }));
          return;
        }
        chunks.push(chunk);
      }

      const bodyBuffer = Buffer.concat(chunks);
      const contentType = req.headers['content-type'] || '';

      try {
        let fileName = safeFileName(req.headers['x-save-name'] || 'save2.dat');
        let fileBytes = bodyBuffer;

        // Se for JSON { name: 'save2.dat', data: '<base64>' }
        if (contentType.includes('application/json')) {
          const json = JSON.parse(bodyBuffer.toString('utf8'));
          if (json.name) fileName = safeFileName(json.name);
          if (json.data) fileBytes = Buffer.from(json.data, 'base64');
        }

        const targetPath = path.join(SAVE_DIR, fileName);
        const tempPath = targetPath + '.tmp';

        // Escrita atômica segura
        fs.writeFileSync(tempPath, fileBytes);
        fs.renameSync(tempPath, targetPath);

        console.log(`[Web Server] Save gravado no Google Drive: ${fileName} (${fileBytes.length} bytes)`);
        res.writeHead(200, { 'Content-Type': 'application/json; charset=utf-8' });
        res.end(JSON.stringify({ ok: true, file: fileName, bytes: fileBytes.length }));
        return;
      } catch (err) {
        console.error('[Web Server] Erro ao salvar:', err);
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ ok: false, error: err.message }));
        return;
      }
    }

    // --- PROXY DE REDE: /api/* para o servidor mock (porta 3000) ---
    if (pathname.startsWith('/api/')) {
      const proxyReq = http.request(
        MOCK_URL + req.url,
        {
          method: req.method,
          headers: {
            ...req.headers,
            host: '127.0.0.1:3000'
          }
        },
        proxyRes => {
          res.writeHead(proxyRes.statusCode, proxyRes.headers);
          proxyRes.pipe(res);
        }
      );

      proxyReq.on('error', err => {
        res.writeHead(502, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ ok: false, error: 'Mock server na porta 3000 indisponivel: ' + err.message }));
      });

      req.pipe(proxyReq);
      return;
    }

    // --- SERVIDOR DE ARQUIVOS ESTÁTICOS (web/dist) ---
    const rel = pathname === '/' ? 'index.html' : pathname.replace(/^\/+/, '');
    const full = path.resolve(WEB_ROOT, rel);

    if (!full.startsWith(path.resolve(WEB_ROOT)) || !fs.existsSync(full) || !fs.statSync(full).isFile()) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      res.end('Arquivo nao encontrado: ' + pathname);
      return;
    }

    const ext = path.extname(full).toLowerCase();
    res.writeHead(200, {
      'Content-Type': MIME_TYPES[ext] || 'application/octet-stream',
      'Content-Length': fs.statSync(full).size
    });
    fs.createReadStream(full).pipe(res);
  });

  return server;
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const server = createWebServer();
  server.listen(PORT, '0.0.0.0', () => {
    console.log(`[Web Server] Destinia Web + Sync Google Drive ativo em http://localhost:${PORT}/`);
    console.log(`[Web Server] Pasta do Drive: ${SAVE_DIR}`);
  });
}
