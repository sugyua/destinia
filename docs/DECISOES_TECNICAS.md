# Decisões técnicas confirmadas

Data desta reconstrução: 2026-10-12.

Este documento separa decisões/evidências confirmadas de hipóteses ainda pendentes. As referências apontam para os arquivos existentes em `C:\destinia`.

## 1. Executar a `.so` original em vez de reimplementar o jogo

**Decisão:** o port atual preserva a biblioteca nativa original `libdestinia_jni.so` e executa essa `.so` no PC via emulação ARM.

**Evidências:**

- `HOST_SPEC.md` afirma que toda a lógica do jogo está na `.so` e que o Java original é uma camada de janela/toque/som/vibração/rede/anúncios/compras (`C:\destinia\Claude\HOST_SPEC.md:10`).
- O host inclui Unicorn e implementa carregamento ELF próprio (`C:\destinia\host\main.cpp:13`, `C:\destinia\host\main.cpp:304-373`).
- O caminho padrão da `.so` é `game/lib/armeabi/libdestinia_jni.so` (`C:\destinia\host\main.cpp:442`).

## 2. Host C++ com Unicorn + SDL2

**Decisão:** o host é um executável C++17 que usa Unicorn para ARM e SDL2 para exibição.

**Evidências:**

- `CMakeLists.txt` exige C++17 e usa `pkg_check_modules(SDL2 REQUIRED sdl2)` e `pkg_check_modules(UNICORN REQUIRED unicorn)` (`C:\destinia\host\CMakeLists.txt:1-11`).
- O código inclui `SDL.h` e `unicorn/unicorn.h` (`C:\destinia\host\main.cpp:11-13`).
- O CMakeCache registra SDL2 encontrado (`SDL2_FOUND=1`, versão `2.32.10`) e Unicorn encontrado (`UNICORN_FOUND=1`, versão `2.1.4`).

## 3. Framebuffer em RGB565, resolução 400x240

**Decisão:** o host usa framebuffer `400x240`, `RGB565`, `192000` bytes.

**Evidências:**

- O host define `SCREEN_W = 400`, `SCREEN_H = 240` e `FB_BYTES = SCREEN_W * SCREEN_H * 2` (`C:\destinia\host\main.cpp:32-36`).
- O histórico exportado registra que o Java original passa 400x240 e buffer de 192000 bytes a `initGame`/`jniRun`.
- `HOST_SPEC.md` confirma que `initGame` recebe width/height do host e que `jniRun` usa buffer `width×height×2` (`C:\destinia\Claude\HOST_SPEC.md:88-91`).
- As imagens geradas em `C:\destinia\host\frames` têm metadados BMP `400x240`.

## 4. Entregar assets como bytes crus

**Decisão:** o host não descomprime nem interpreta assets do jogo; ele lê arquivos em `game/assets` e entrega os bytes para a `.so`.

**Evidências:**

- `HOST_SPEC.md` afirma que a `.so` tem decodificador próprio de ZCP/WBI e que o host só precisa entregar bytes crus (`C:\destinia\Claude\HOST_SPEC.md:12`).
- `getAssetResSize` consulta `assetsDir + "/" + nome` e retorna o tamanho (`C:\destinia\host\main.cpp:264-272`).
- `getAssetRes` lê o arquivo em `assetsDir + "/" + nome` e retorna um `jbyteArray` falso (`C:\destinia\host\main.cpp:273-281`).
- O log da execução mostra vários `getAssetResSize` bem-sucedidos, como `gametxt_en.tdt`, `dotum08.fnt`, `gv555.wmb`, `title.wmb` e outros (`C:\destinia\host\log.txt:7-32`, `C:\destinia\host\log.txt:36-70`).

## 5. Redirecionar saves Android para pasta local

**Decisão:** chamadas nativas a `/data/data/game.destiniaeng/files/...` são redirecionadas para a pasta de save local configurada no host.

**Evidências:**

- `HOST_SPEC.md` registra que saves são feitos pela `.so` via stdio nesse prefixo Android (`C:\destinia\Claude\HOST_SPEC.md:13`, `C:\destinia\Claude\HOST_SPEC.md:94`).
- A função `hostPath` remove o prefixo `/data/data/game.destiniaeng/files/` e redireciona para `saveDir` (`C:\destinia\host\main.cpp:147-153`).
- O log mostra tentativas de abrir `save/msave.dat` para leitura, com falha esperada quando não há save (`C:\destinia\host\log.txt:10-11`).

## 6. Idioma inglês por `isKorean = 0`

**Decisão:** o host inicializa a `.so` com `isKorean = 0`, para a versão em inglês.

**Evidências:**

- Comentário do host: `isKorean = 0 -> a .so escolhe os arquivos *_en (ingles)` (`C:\destinia\host\main.cpp:483-484`).
- A chamada a `initGame` passa `0` como último argumento (`C:\destinia\host\main.cpp:490`).
- O log mostra carregamento de assets com sufixo `_en`, por exemplo `gametxt_en.tdt`, `item_en.gdt`, `equip_name_en.tdt`, `skill0_desc_en.tdt`, `nick_en.tdt` (`C:\destinia\host\log.txt:7`, `C:\destinia\host\log.txt:48`, `C:\destinia\host\log.txt:63-69`).

## 7. Eventos de toque no host

**Decisão atual:** o host encaminha mouse para `jniTouch(env,obj,x,y,tipo)`, com `tipo` 0/1/2 para pressionar/soltar/mover e `3` para tecla de menu/ESC.

**Evidências:**

- O host define a lambda `touch(x, y, type)` chamando `fnTouch` com argumentos `{ENV_OBJ, 0x10, x, y, type}` (`C:\destinia\host\main.cpp:493-494`).
- Eventos SDL de mouse são convertidos para coordenadas lógicas 400x240 e encaminhados para `touch` (`C:\destinia\host\main.cpp:509-520`).
- `HOST_SPEC.md` inicialmente registrou ordem provável `(x, y, tipo)` e necessidade de confirmação; o código atual incorpora essa decisão (`C:\destinia\Claude\HOST_SPEC.md:92`).

## 8. Neutralizar anúncios, analytics, vibração e rede por enquanto

**Decisão atual:** callbacks de vibração, URL, analytics e fechamento de socket são ignorados; som e compra são apenas registrados; rede ativa ainda não foi implementada.

**Evidências:**

- `HOST_SPEC.md` recomenda remover/neutralizar anúncios e não abrir conexões reais com servidores Gamevil (`C:\destinia\Claude\HOST_SPEC.md:54-62`).
- O callback `playVib` é ignorado; `gotoURL` é registrado/ignorado; `logEvent` é descartado; `close` não faz nada (`C:\destinia\host\main.cpp:282-294`).
- `playSound` apenas registra que áudio ainda não foi implementado (`C:\destinia\host\main.cpp:288`).
- `buyCash` apenas registra que compra ainda não foi tratada (`C:\destinia\host\main.cpp:291`).
- O log confirma pedidos de som sem implementação (`C:\destinia\host\log.txt:41-42`, `C:\destinia\host\log.txt:71-72`).

## 9. Compras não estão decididas

**Decisão confirmada:** ainda não há decisão de design sobre compras/cash no port.

**Evidências:**

- `HOST_SPEC.md` afirma que o fluxo Google Play é local do ponto de vista nativo e lista alternativas: responder sucesso, erro ou esconder botão (`C:\destinia\Claude\HOST_SPEC.md:101-105`).
- O host atual só registra `buyCash(id)` e marca como ainda não tratado (`C:\destinia\host\main.cpp:291`).

## 10. Headless validado; janela real não validada neste material

**Decisão/estado:** a execução headless de 60 quadros foi validada; não há evidência encontrada nesta etapa de que interação em janela real tenha sido validada.

**Evidências de headless:**

- `log.txt` termina com `60 quadros executados. Imagens em frames` (`C:\destinia\host\log.txt:73`).
- O histórico exportado registra que as imagens mostram logo/animação Gamevil e tela `TAP TO START` no quadro 50.
- Existem BMPs em `C:\destinia\host\frames` com dimensão `400x240`.

**Limite importante:**

- O histórico exportado, após confirmar o headless, pede para testar com janela (`./build/destinia_host.exe --game /c/destinia/game`) e clicar na tela de título. Não foi encontrada, nesta etapa, resposta posterior confirmando que esse teste de janela/interação foi executado com sucesso.

## 11. `Claude/main.cpp` e `host/main.cpp` estão sincronizados

**Decisão/estado:** não há divergência de conteúdo entre a cópia em `Claude` e o host ativo.

**Evidências:**

- `C:\destinia\Claude\main.cpp`: tamanho `31082`, SHA-256 `9c52896ebd2656f9d06ee7f160fc47638f77c0c991c6b33b709682f5164a98b7`, mtime `2026-10-12T00:00:15.711390`.
- `C:\destinia\host\main.cpp`: tamanho `31082`, SHA-256 `9c52896ebd2656f9d06ee7f160fc47638f77c0c991c6b33b709682f5164a98b7`, mtime `2026-10-12T00:00:27.216235`.
- Portanto, são idênticos byte a byte; a diferença é apenas de timestamp.

## 12. Save compartilhado entre versões do host

**Decisão:** as versões normais do host usam `%LOCALAPPDATA%\Destinia\save`, independentemente de serem iniciadas pelo launcher padrão, 30/60 FPS ou host de zoom. Essa também é a pasta padrão ao executar diretamente os binários no Windows.

- O diretório é criado pelo host na inicialização.
- `--save` continua permitindo escolher outro diretório, usado pelos launchers de teste que precisam manter saves isolados.
- Os saves antigos em `host\save`, `host\save_fps30`, `host\save_fps60` e `host_zoom\save_zoom800` não são removidos nem copiados automaticamente.
- A pasta compartilhada começa vazia; nenhum progresso antigo é migrado.

## [2026-10-12] Mapeamento do Mouse e Escala da Janela
- **Problema:** Ao ampliar a janela de 400x240 para 1200x720 usando `SDL_RenderSetLogicalSize`, o viewport interno de renderização do SDL2 mantinha a saída descompassada com a transformação lógica, causando desalinhamento no clique.
- **Solução adotada:**
  1. Remoção da abstração `SDL_RenderSetLogicalSize` / `SDL_RenderWindowToLogical`.
  2. Apresentação da textura esticada cobrindo explicitamente o retângulo real retornado por `SDL_GetRendererOutputSize` (`dst = {0, 0, ow, oh}`).
  3. Mapeamento direto e proporcional de coordenadas do mouse: `x = (wx * 400) / ow` e `y = (wy * 240) / oh`.
- **Status:** [C] Confirmado em teste interativo. Clique no botão 1 gerou `tecla_toque=28` e mudou `menu.estado=2`.

## [2026-10-18] Ritmo de Quadro e Aquecimento de CPU
- **Problema:** o port esquentava o processador mais que qualquer outro programa. Causa medida com o novo profiler (`--profile`): os launchers `--fps 30/60` forçavam o host a tentar mais quadros do que o jogo pede. O Destinia e de passo fixo e se auto-regula (medido: `wait` medio ~60-75ms no menu = ~16 FPS; a emulacao custa ~14ms/quadro em 400x240 e ~30ms em 800x480). Forçar 60fps fazia a emulacao rodar ~4x mais que o ritmo natural, prendendo ~84% de um núcleo so no menu.
- **Pior caso:** quando a emulacao de uma cena ultrapassa o `wait`, `sleepMs = max(1, wait - elapsed)` colapsava para 1ms -> loop praticamente sem limite, núcleo a 100%.
- **Decisão final (pedido do usuário, 2026-10-18):** o jogo deve rodar como o original, a ~16 FPS, sem modo turbo. Consequências:
  - O flag `--fps` (30/60) foi **REMOVIDO** de `host/main.cpp` e `host_zoom/main.cpp`; não existe mais forçar frame rate.
  - Os launchers `Abrir Destinia 30 FPS.bat` e `Abrir Destinia 60 FPS.bat` foram **APAGADOS**.
  - O ritmo do quadro e sempre `budgetMs = wait` (100 no quadro de carga `wait>=1000`), e o host dorme o restante `budgetMs - elapsed` apenas em janela. Headless continua flat-out (dump rapido de frames).
  - `fastForward` (Shift / L1) permanece como aceleracao manual sob demanda, sem alterar o ritmo normal.
- **Medição (menu, 400x240):** emul ~14ms/quadro, `wait` ~60-75ms -> ~16 FPS, ~22% de 1 núcleo. Em `host_zoom` 800x480: emul ~30ms, ~45% de 1 núcleo. Picos de emulacao (ate ~524ms num quadro de carga) sao custo inerente do Unicorn; nao ha sleep que os reduza.
- **Ferramentas de medicao:**
  - `--profile`: imprime a cada 120 quadros `emul (max)/presente/sleep/wall`, `wait` medio, FPS efetivo e % de quadros acima do orçamento.
  - `Diagnosticar calor.bat` (raiz): abre o jogo em janela com `--profile`, amostra o uso real de CPU do processo em segundo plano (`diagnostico/sample_cpu.ps1`) e, ao fechar o jogo, gera `relatorios\calor_<data>\relatorio_calor.txt` (`diagnostico/build_report.ps1`) com custo de emulacao, ritmo, uso de CPU e um diagnostico automatico da causa do aquecimento. Validado de ponta a ponta (CSV com ponto decimal, parsing correto).

## [2026-10-19] Tradução PT-BR — expansao para "tudo em ingles"
- **Mecanismo (host_zoom, `--lang pt`):** o host intercepta os assets de texto (`isPortugueseAsset`), e `translatePortugueseString` traduz cada string via: mapa exato `portugueseText()` -> nomes de item procedurais (descritor+substantivo com genero/plural) -> substituicao de substrings (maior-primeiro). `.tdt` usa `translateTable` (tabela com prefixo de tamanho, pode crescer); `.gdt` usa `translateGdt` (campos separados por NUL, traduz in-place so se couber).
- **Trabalho desta etapa:** sem arquivos externos de traducao (o conteudo e o mapa embutido). Extraí todas as strings dos assets atendidos (`diagnostico/extract_strings.py`), simulei a passada de substrings e medi o que PERMANECE em ingles: **254 strings**. Traduzi ~205 (UI, system messages, itens, missoes, skills) para PT-BR **sem acentos** (padrao do mapa existente, ex.: "Forca") e adicionei ao mapa `portugueseText()`.
- **Nao traduzido de proposito:** abreviacoes universais de RPG (ATK/DEF/INT/STR/VIT/CRI/EVA/HIT/AGI/BGM/STATUS), nomes proprios fantasy (Excalibur, Katana, Bergheim, Fermion, Revolver, Ninja, Sniper...), marcadores de debug (`<--------GameCanvas...>`), serial/versao e strings coreanas corrompidas.
- **Strings com nova-linha real (0x0A):** 5 (`network error`, `GAMEVIL Inc.`, `Success. `, `Transferring data...`, `item has been obtained.`) usam byte 0x0A; adicionadas com escape C++ `\n` (chave byte-exata).
- **Verificacao:** build `host_zoom` limpo; roda `--lang pt` headless (exit 0) aplicando traducao; sem regressao sem `--lang pt`. Cobertura restante ~49 (a maioria os skips intencionais acima + 3 falsos positivos da ferramenta de checagem).
- **Ferramenta mantida:** `diagnostico/extract_strings.py` verifica cobertura (strings que ainda permanecem em ingles) para rodadas futuras.
- **Limitacao `.gdt`:** traducoes PT mais longas em `item_en.gdt`/`quest_en.gdt` podem ser descartadas por falta de espaco no campo (o host loga "mantido(s) em ingles por falta de espaco"); os `.tdt` (UI/nomes) crescem normalmente.

## [2026-10-18] Superfície de Rede Mapeada (ainda nao implementada)
- A `.so` NAO importa socket (sem `socket/connect/send/recv` indefinidos). Todo o transporte e feito por um buffer em memoria (`SOCKETBUFFER`, `Comm`): a camada Java enchia o buffer e a `.so` lia via `Comm_netSocketRead`/`MC_netSocketRead`.
- Entradas JNI de rede exportadas: `Java_game_destiniaeng_CommManager_commCheck`, `..._commGetConType`, `Java_game_destiniaeng_GameThread_procCommRead`, `..._procCommMsg`.
- Logica `Comm_*` exportada: `Comm_connect`, `Comm_read/2/4/8Bytes`, `Comm_readBytes/Str`, `Comm_write*`, `Comm_flush`, `Comm_resultCB`, `Comm_netSocketRead`, etc.
- Menu de rede e opcional (single-player parece local): existem `_drawNetRankMenu`, `_drawNetBattleMenu`, `_drawNetTradeMenu`, `_drawNetBuyMenu`, `_drawNetPostMenu`.
- **Consequência:** como o host nunca preenche `SOCKETBUFFER`, qualquer fluxo que espere resposta de servidor ficaria em `rede=1 (aguardando)` para sempre. A sonda (`sonda.txt`) mostra `rede=0` ate o menu principal (`menu.estado=2`), entao o caminho ate la nao exige rede.
- **Proximo passo (opção do usuário):** decidir se responde sucesso/erro via `procCommMsg`/`procCommRead` para os fluxoes de ranking/compra/presente, ou se esconde esses menus.
