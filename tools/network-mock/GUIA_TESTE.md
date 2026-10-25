# Destinia — teste gratuito de comunicação PvP e Leilão

## O que este pacote testa

Este é um **laboratório independente**, não uma atualização do cliente Destinia. Oferece uma página de simulação e um servidor HTTP polling (atualização a cada segundo), com salas para dois jogadores, ataque com dano fixo e leilão de item fictício. Nada é lido ou gravado nos saves. Não compartilha gold/itens reais.

A web0.9.7 analisada não contém um transporte WebSocket/HTTP de PvP/Leilão ligado a este protocolo. `web/dist/engine.mjs` emula JNI/ARM e trata compra indisponível por cancelamento offline (`finishOfflinePurchase`). Textos e menus originais não comprovam um servidor online funcional. **Abrir PvP no jogo original não conecta a este mock.** Para integração real ainda é necessário mapear os contratos/callbacks originais e implementar um adaptador; isso não foi feito neste pacote.

## 1. Dependências (somente no seu PC)

- Node.js **22 ou mais recente**, versão LTS: https://nodejs.org/ (instalador inclui npm).
- Navegador atualizado, internet e PC ligado durante o teste. Seu primo só precisa de navegador.
- Localtunnel para túnel temporário HTTPS. Sem conta ou domínio pago; disponibilidade e desempenho do serviço gratuito não são garantidos. Não é necessário abrir porta no roteador ou pagar hospedagem.

Não acompanham instaladores/dependências externas no ZIP. Este servidor não usa pacotes npm; localtunnel será baixado pelo `npx` quando solicitado.

## 2. Extraia e inicie (Windows PowerShell)

Copie os sete arquivos desta pasta para `C:\destinia-mock`, mantendo `public` ao lado de `server.js`. No ZIP do branch, eles ficam em `tools/network-mock`; copie o conteúdo dessa pasta, não os arquivos do jogo.
Abra PowerShell nessa pasta:

```powershell
Set-Location 'C:\destinia-mock'
node --version
npm.cmd --version
npm.cmd test
npm.cmd start
```

`npm.cmd` evita o bloqueio de `npm.ps1` sem mudar a política do PowerShell. Em macOS/Linux, use `npm test` e `npm start`.

Deixe este terminal aberto. Acesse **http://127.0.0.1:3000**. Aparecerá “Destinia — laboratório de rede”, com aviso de simulação. **Não é a página do jogo publicado.** Se a porta estiver ocupada, feche o servidor duplicado. Alternativamente, no PowerShell:

```powershell
$env:PORT='3001'
npm.cmd start
```

Nesse caso ajuste também a porta do túnel para3001.

## 3. Abra o túnel gratuito

Com o servidor ligado, abra **outro PowerShell** e execute:

```powershell
npx.cmd --yes localtunnel@2.0.2 --port 3000
```

Em macOS/Linux:

```sh
npx --yes localtunnel@2.0.2 --port 3000
```

Isso baixa/executa localtunnel, sem instalação global. Ele imprime uma URL semelhante a `https://algum-nome.loca.lt`. **Copie a URL realmente exibida**, não este exemplo. Compartilhe somente com seu primo. Deixe os dois terminais abertos.

O serviço pode mostrar uma página de proteção antes do simulador. Se pedir “Tunnel Password”, no PC que abriu o túnel execute:

```powershell
curl.exe https://loca.lt/mytunnelpassword
```

O resultado é o IP público de saída usado pelo túnel. Envie ao primo em privado, para digitar na página de proteção. Não é senha do seu Windows, GitHub ou jogo; não digite credenciais pessoais. VPN/proxy ou políticas de rede podem impedir o serviço. Se o túnel não gerar URL ou a página não abrir, registre a mensagem; não é validação de integração do jogo.

## 4. Você e seu primo

1. Primeiro teste sozinho em duas abas locais (cada aba tem sessão própria).
2. Depois ambos acessem a **URL HTTPS do túnel**. Seu primo não deve usar localhost, IP privado ou o site Destinia hospedado anteriormente.
3. Digitem nomes diferentes e **exatamente a mesma sala**, por exemplo `primos-7f2b9`. Use uma sala difícil de adivinhar; não há contas nem autenticação real.
4. Clique em **Entrar na sala** em cada navegador. Ambos devem aparecer em até aproximadamente1segundo, além da latência do túnel.

### PvP simulado

- Com dois jogadores na lista, clique **Atacar oponente**. O HP dele deve cair de100 para90 nos dois navegadores.
- Alterne ataques; dano fixo10 e intervalo mínimo500ms, determinados pelo servidor. Ao chegar a0, ataques são bloqueados.
- **Restaurar meu HP** restaura só seu personagem fictício. Serve exclusivamente para repetir o teste.
- Não há movimento de personagens, colisão, habilidades ou combate da engine original neste laboratório.

### Leilão simulado

- Autor oferece100; primo deve ver lance100. Primo oferece150; ambos devem ver novo líder.
- Lance menor/igual, fracionado, negativo ou acima dos1000gold fictícios deve ser rejeitado.
- **Finalizar leilão de teste** pode ser acionado por qualquer participante: fecha uma única vez e desconta o valor do vencedor fictício. Não entrega item no jogo original.
- Para outro leilão, ambos saiam e criem nova sala. Sem ranking ou economia persistente.

### Isolamento e desconexão

- Em outra aba, use sala diferente: não deve ver os jogadores/lances da primeira.
- Terceiro jogador na mesma sala deve receber “Sala cheia”.
- Use **Sair** para desconexão imediata. Fechar/recarregar a aba perde o token; a sessão antiga expira após120segundos sem requisições, e a remoção é observada na próxima consulta. Se sala ficar cheia, aguarde ou use nova sala.
- Desconectar líder cancela seu lance se leilão ainda aberto. Histórico limitado aos últimos40eventos.
- Desligue o túnel e observe falha de atualização; religá-lo normalmente muda a URL. Parar Node apaga todas as salas.

## 5. Encerrar e limites

**Ctrl+C nos dois terminais** encerra servidor e túnel. O servidor escuta somente127.0.0.1; apenas os três arquivos públicos permitidos são servidos. Não coloque segredos na página. Quem tiver acesso à sala pode executar suas ações de teste; não use isto como servidor público de produção. Limites:32salas/64sessões/2participantes por sala, JSON4KB e ações limitadas por sessão. Não é proteção completa contra abuso/DDoS.

O túnel expõe apenas este simulador na porta escolhida, não todos os arquivos da pasta. Não forneça acesso administrativo, não desative firewall/antivírus e não exponha seus saves.

## Protocolo para um futuro adaptador

Todos os endpoints ficam na mesma origem da página. `POST` usa JSON; `GET /api/state` atualiza o estado. Sem CORS aberto e sem endpoint de configuração do jogo.

| Operação | Requisição |
| --- | --- |
| Entrar | `POST /api/onJoin` com `{"name":"Autor","room":"primos"}` |
| Consultar | `GET /api/state` |
| Atacar | `POST /api/onPlayerAttack` com `{"targetId":"ID do oponente"}` |
| Lance | `POST /api/onAuctionBid` com `{"amount":150}` |
| Encerrar leilão | `POST /api/onAuctionClose` com `{}` |
| Restaurar HP próprio | `POST /api/resetHp` com `{}` |
| Sair | `POST /api/leave` com `{}` |
| Saúde | `GET /health` sem token |

A resposta de entrada contém `token`, `me`, jogadores, leilão e eventos. Todas as operações após entrada usam `Authorization: Bearer TOKEN`. IDs/token são gerados pelo servidor; não enviar dano, HP ou saldo escolhidos pelo cliente. Token fica só na memória da aba. HTTP polling é deliberado: não existe WebSocket neste pacote.

## Evidências e relatório manual

`npm test` exercita dois clientes HTTP reais, ataque/saldo autoritativos, isolamento, limite de sala, lances inválidos, encerramento único, sessão, JSON/origem inválidos e saída. Isso **não comprova túnel externo ou iPhone físico**.

Anote: versão Node/navegadores, rede, horário, passos, resultado esperado/observado e erro exibido. Não compartilhe tokens. Confirme separadamente: duas abas locais; primo pela internet; navegador iPhone; queda/reconexão. A integração ARM real continua pendente.
