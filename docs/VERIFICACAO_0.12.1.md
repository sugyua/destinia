# Verificação 0.12.1 Beta — 27/10/2026

Escopo: Issue #5, coerência da distribuição e atalhos Web F4/F5/F8. Base c0c1566, branch fix/0121-issue5. Worktree isolada; saves pessoais e main preservados.

## Executado

- Node `node --test web/tests/*.test.mjs`: 134 casos, 133 aprovados, 1 ignorado, zero falhas. Inclui testes ARM/WASM existentes e seis regressões de teclado.
- Python `test_package*.py`: nove aprovados; seleção das notas pela versão, ausência sem artefato parcial, exclusão de DLL plantada, allowlist, caminhos proibidos, symlinks, reprodução e rejeição de PE incompatível.
- Python `test_release_coherence.py`: versão das entradas, manifesto e cobertura recursiva dos includes C++ nos fontes; biblioteca externa fora dos pacotes.
- `test_runtime.py`: um aprovado (backup/lock), 12 ignorados por flag opcional nativa; não alegados como executados.
- Builds CMake/Ninja Release de host e host_zoom: GCC 16.2.0, SDL2 2.32.10, SDL2_mixer 2.8.2, Unicorn 2.1.4, zlib 1.3.2, Windows UCRT64. Ambos `--version`: 0.12.1 Beta. Ambos executaram dez quadros ARM com SDL dummy, exit 0, diretórios temporários e `--no-save-preferences`.
- Chromium/Playwright `browser_keyboard_events_qa.py`: aplicativo real com **worker fixture**, F4/F5/F8 press/release/repetição, botões, F5 ativo/pausado com foco nos campos de loot/idioma e Tab aprovados. O teste verifica também `defaultPrevented`. Isso verifica o contrato de entrada, não gameplay.
- Regressões observadas antes das correções: pacote sem notas atuais; manifesto sem includes como common/titles.h; versões de host/banner divergentes.
- Pacotes candidatos: ZIP fontes com 297 entradas e Windows com 47; CRC/checksums conferidos, sem game/saves/DLLs/biblioteca ARM/Unicorn.js/chaves/logs. Ambos inicializadores GUI extraídos do ZIP executaram os hosts recompilados por dez quadros, em diretórios temporários. O teste fornece somente em cópia local a biblioteca original exigida pelo launcher; ela não foi adicionada aos pacotes.

## Bloqueios e limites

- `browser_keyboard_qa.py`, app + worker/ARM real, não concluiu: `net::ERR_CONNECTION_RESET` ao importar vendor/unicorn_arm.js no servidor local. Edge e Chromium apresentaram a falha antes de iniciar a partida. Não foi alterada a biblioteca nem contornada proteção do PC. Testes Node ARM não substituem esta prova de navegador.
- Deployment: chamada `Sites list_sites` devolveu ferramenta indisponível. Site privado existente preservado; versão publicada não confirmada: https://destinia-web-experimental.leafyheron257.chatgpt.site/.
- Gameplay físico/campanha, Safari/iPhone, joystick USB e CI remoto desta versão não executados. Não houve mudança de lógica nativa além dos textos de versão.
- Launchers GUI reutilizados da release 0.9.9; `launcher/main.go` sem diferenças desde aquela tag. Checksum do ZIP local confirmado contra checksum baixado do GitHub: 62984787c12dcd7ae87586d5023abf9265f8be0faae7ca8d6645dff5483fa903. Os dois hosts foram recompilados nesta entrega.
- DLLs e Unicorn.js não acompanham o novo pacote curado/export da tag. Para desenvolver a Web, fornecer a dependência Unicorn.js/ponte indicada em web/dist/vendor/ORIGEM.txt e BRIDGE-PATCH.txt; testes que dependem dela exigem esse arquivo local. Dados originais sempre fornecidos pelo proprietário, fora dos pacotes.

Revisão independente: encontrou falta da proteção F5 em campos editáveis; regressão Chromium falhou antes, correção por listener capture aplicada e revisão confirmou seis testes Node e teste Chromium aprovados, sem novos bloqueadores. Revisão não equivale a gameplay físico.

Releases anteriores preservadas. Os demais tópicos da Issue #5 permanecem abertos. Confirmação de pacote/download fica registrada no checkpoint da central e na release, sem transformar falhas/pendências em aprovação.
