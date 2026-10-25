# Multi Mode e laboratório de rede — 25/10/2026

## Fluxo informado pelo autor

Na versão web, o autor selecionou **MULTI MODE**, escolheu um personagem e confirmou. A tela exibiu carregamento e depois a mensagem **“Ocorreu um erro de rede. Codigo de Erro = 15”**, retornando à seleção. Capturas fornecidas na conversa confirmam essa sequência visual.

## Causa interna confirmada do `Codigo de Erro = 15`

O rastreamento estático completo do binário `game/lib/armeabi/libdestinia_jni.so` está documentado em **[CONTRATO_MULTI_MODE_ARM.md](CONTRATO_MULTI_MODE_ARM.md)**:

1. Ao confirmar um personagem em **MULTI MODE** (`Menu.state == 8`, `MenuCanvas_mKeyPressed` em `0x22c36`), o jogo carrega o save do slot escolhido, inicializa `Comm_setConType(0)` + `Comm_init(0)` (`cmd = 0`, Login Multi Mode), escreve 58 bytes de payload do herói e chama `Comm_flush()` (`0x0edd9`), que define `MMain.netState = 1` (`MMain + 0x3920`), zera `MMain.netTimer = 0` (`MMain + 0x3924`) e publica o pacote de 111 bytes em `Comm.pendingSendLen` (`Comm + 0x2f90`).
2. Como os hosts C++ (`host/main.cpp`, `host_zoom/main.cpp`) e o motor Web (`web/dist/engine.mjs`) ainda não consomem `Java_game_destiniaeng_CommManager_commCheck` (`0x07bed`) nem injetam resposta via `procCommRead` (`0x07b4d`) / `procCommMsg` (`0x07c4d`), `MMain.netState` permanece em `1`.
3. A cada quadro, `MMain_run` (`0x1040d`) incrementa `MMain.netTimer` (`0x10456`). Ao ultrapassar **150 quadros** (`0x10484: cmp r3, #150`), `MMain_run` chama `Comm_close()`, define `Comm.retryCount = 10` e invoca `Comm.processCB(15, 0)` → **`Menu_netProc(15, 0)`** (`0x104b6: movs r0, #15`).
4. `Menu_netProc(15, 0)` (`0x22f0e`) formata `"Ocorreu um erro de rede.\nCodigo de Erro = 15"` em `netData + 260`, limpa `Game` em memória e muda para `MMain_setNetState(2)`, exibindo o popup sobre a seleção de personagem.

## O que está disponível

O [laboratório local](../tools/network-mock/GUIA_TESTE.md) fornece servidor Node22+ sem dependências npm, página de teste e protocolo HTTP polling próprio. Permite duas sessões por sala, ataques/HP fictícios e lances/saldo fictícios, sem acessar saves ou inventário real. Endpoints `onJoin`, `onPlayerAttack` e `onAuctionBid` são contratos do mock, não contratos comprovados da biblioteca Android.

A página do laboratório é **http://127.0.0.1:3000** no PC que roda `npm.cmd start`; o convidado usa a URL HTTPS gerada pelo localtunnel. O site publicado do Destinia não passa a usar o mock ao iniciar esses comandos. O guia também explica a pasta correta, instalação Node e o erro npm ENOENT quando executado fora da pasta com package.json.

## Estado da integração

- **Confirmado:** web0.9.7 executa a biblioteca ARM por emulação; `web/dist/engine.mjs` possui JNI e cancelamento offline de compra em `finishOfflinePurchase`.
- **Confirmado por engenharia reversa ARM:** protocolo binário `Comm_*` (Big-Endian), máquina de estados de `Menu_netProc` (`cmd = 0` Login → `cmd = 8` Mailbox → abertura de `Menu.state = 20` / `GameUI = 0xa0000001`) e proteção de save (`readAvail == 0` no `cmd = 0` evita chamar `GameCanvas_SaveGame`) mapeados em [CONTRATO_MULTI_MODE_ARM.md](CONTRATO_MULTI_MODE_ARM.md).
- **Confirmado por validação automatizada de 2 clientes ARM simultâneos (`tools/verify-pvp-auction-mock.mjs`):**
  - Instanciação de dois motores ARM emulados (`libdestinia_jni.so`) representando Cliente 1 (Guerreiro) e Cliente 2 (Atirador).
  - Ambos completam o handshake de rede (`cmd = 0` e `cmd = 8`), transitando para `Menu.state = 20` e `GameUI+124 = 0xa0000001` com `gameSaveCalls = 0` (zero escritas em saves em disco).
  - Registro sincronizado dos dois clientes na sala do mock (`/api/onJoin`).
  - Duelo PvP executado com sucesso (`/api/onPlayerAttack`: Guerreiro desfere ataque e HP do Atirador é reduzido para 75).
  - Leilão executado com sucesso (`/api/onAuctionBid`: Atirador cobre o lance inicial de 100 gold ofertando 250 gold, validado no estado global da sala).
- **Confirmado por testes do mock:** `node --test tools/network-mock/server.test.js` passa 100% (salas, ataque, lances, isolamento e validações).
- **Pendente:** testar o túnel físico com o primo e validar execução física Windows/iPhone com latência real.

## Próximo trabalho recomendado

1. ~~Rastrear seleção de personagem→solicitação JNI/Comm→callback que produz erro15, usando contratos ARM e testes existentes.~~ *(Concluído em [CONTRATO_MULTI_MODE_ARM.md](CONTRATO_MULTI_MODE_ARM.md))*
2. ~~Documentar formato/estado da operação original antes de definir o adaptador.~~ *(Concluído em [CONTRATO_MULTI_MODE_ARM.md](CONTRATO_MULTI_MODE_ARM.md))*
3. ~~Integrar fluxo comprovado (`cmd = 0` + `cmd = 8` em modo sandbox sem gravar no save real) e simular 2 clientes ARM com PvP e Leilão no mock.~~ *(Concluído em [verify-pvp-auction-mock.mjs](../tools/verify-pvp-auction-mock.mjs))*
4. Validar dois clientes reais via túnel (localtunnel/ngrok) com UI web/desktop. Economia/leilão persistente e compatibilidade com servidor original Everple permanecem como tarefas futuras de longo prazo.

Este registro e o laboratório não alteram a versão do jogo, a Release0.9.7 ou o site publicado.
