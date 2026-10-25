# Multi Mode e laboratório de rede — 25/10/2026

## Fluxo informado pelo autor

Na versão web, o autor selecionou **MULTI MODE**, escolheu um personagem e confirmou. A tela exibiu carregamento e depois a mensagem **“Ocorreu um erro de rede. Codigo de Erro = 15”**, retornando à seleção. Capturas fornecidas na conversa confirmam essa sequência visual.

Isso confirma a falha nesse caminho; **não identifica a causa interna do código15**. Não foi capturado o pedido de rede/callback correspondente, nem confirmada a disponibilidade do servidor original. Não atribuir o erro a firewall, túnel ou servidor desativado sem evidência.

## O que está disponível

O [laboratório local](../tools/network-mock/GUIA_TESTE.md) fornece servidor Node22+ sem dependências npm, página de teste e protocolo HTTP polling próprio. Permite duas sessões por sala, ataques/HP fictícios e lances/saldo fictícios, sem acessar saves ou inventário real. Endpoints `onJoin`, `onPlayerAttack` e `onAuctionBid` são contratos do mock, não contratos comprovados da biblioteca Android.

A página do laboratório é **http://127.0.0.1:3000** no PC que roda `npm.cmd start`; o convidado usa a URL HTTPS gerada pelo localtunnel. O site publicado do Destinia não passa a usar o mock ao iniciar esses comandos. O guia também explica a pasta correta, instalação Node e o erro npm ENOENT quando executado fora da pasta com package.json.

## Estado da integração

- **Confirmado:** web0.9.7 executa a biblioteca ARM por emulação; `web/dist/engine.mjs` possui JNI e cancelamento offline de compra em `finishOfflinePurchase`. Na análise direcionada não foi encontrado transporte PvP/Leilão integrado ao protocolo deste mock.
- **Confirmado por testes anteriores:** `npm test` do mock e Chromium com duas páginas passaram (salas, ataque, lances, encerramento e isolamento/validações). Não são testes do Multi Mode original.
- **Pendente:** teste do túnel com o primo, execução física Windows/iPhone e captura do fluxo nativo do Multi Mode. Nenhuma conexão real entre dois jogadores da engine foi demonstrada.
- **Hipótese não comprovada:** a causa específica da mensagem15. A presença de menus/textos não comprova implementação online funcional.

## Próximo trabalho recomendado

1. Rastrear seleção de personagem→solicitação JNI/Comm→callback que produz erro15, usando contratos ARM e testes existentes.
2. Documentar formato/estado da operação original antes de definir o adaptador. Não devolver sucesso artificial ou simular gold/itens no save real.
3. Integrar apenas um fluxo comprovado ao mock e testar falha, cancelamento, repetição, desconexão e preservação do save em cópias.
4. Só então validar dois clientes reais via túnel. PvP completo, economia/leilão persistente e compatibilidade com servidor original não estão entregues.

Este registro e o laboratório não alteram a versão do jogo, a Release0.9.7 ou o site publicado. Não há correção de erro15 nesta entrega.
