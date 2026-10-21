# Destinia para Windows — 0.7 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**[Baixar Destinia 0.7 Beta](https://github.com/sugyua/destinia/releases/tag/v0.7-beta)** — publicação autorizada pelo autor; os pacotes de fontes e Windows x64 são publicados pelo workflow somente após os testes passarem.

A 0.6 adiciona configurações persistentes, comandos remapeáveis, volumes separados, pausa opcional e distribuição Windows x64 com os dois EXEs. Veja [instalação](INSTALL.md), [preferências](docs/PREFERENCIAS.md), [changelog](CHANGELOG.md) e [verificações](docs/RELEASE_0.6.md). Dependências externas e dados originais continuam separados.

[0.6](https://github.com/sugyua/destinia/releases/tag/v0.6-beta) · [0.5](https://github.com/sugyua/destinia/releases/tag/v0.5-beta) · [0.4](https://github.com/sugyua/destinia/releases/tag/v0.4-beta) · [0.3](https://github.com/sugyua/destinia/releases/tag/v0.3-beta) · [0.2](https://github.com/sugyua/destinia/releases/tag/v0.2-beta) · [Todas as releases](https://github.com/sugyua/destinia/releases)

## Arquitetura

O host em C++17 executa a biblioteca ARM original do Destinia Android com Unicorn. SDL2 apresenta o framebuffer RGB565 e recebe entradas; SDL2_mixer habilita áudio opcional. A lógica original continua na biblioteca ARM.

| Diretório | Finalidade |
| --- | --- |
| `host` | Host 400×240 |
| `common` | Logs, backups, configuração compartilhada e validação ELF usados pelos dois hosts |
| `host_zoom` | Zoom out 800×480, mapas pequenos centralizados com fundo cinza, tela cheia proporcional com F11, tradução PT e trace de viewport |
| `game` | APK original do Destinia Android extraído; recursos necessários em execução, não incluídos no ZIP curado de fontes |
| `Claude` | Especificação do host e ferramentas de análise para desenvolvimento |
| `docs` | Decisões e histórico técnico; documentos antigos podem estar desatualizados |
| `release` | Manifesto e notas das versões |
| `scripts` | Compilação, configuração de saves e empacotamento dos fontes revisados |
| `tests` | Testes de distribuição, diagnóstico, suporte, falhas do host e zoom |

O repositório atual não inclui saves pessoais, logs de partidas, relatórios locais ou cópias antigas dos fontes. O `.gitignore` impede o envio acidental desses arquivos. A documentação técnica histórica pode citar arquivos removidos na limpeza; os fontes ativos estão em `host` e `host_zoom`.

## Recursos presentes no código

- Entradas por teclado, mouse e gamepad.
- Saves redirecionados; no Windows, o padrão é `%LOCALAPPDATA%\Destinia\save`.
- Música e efeitos quando compilado com SDL2_mixer.
- HUD de experiência, modo sem janela e capturas BMP.
- Tradução opcional `--lang pt` e trace F9 com `--trace-viewport`, apenas em `host_zoom`.

A presença no código não equivale a validação completa de gameplay. O host_zoom usa framebuffer 800×480 para zoom out, mapas pequenos centralizados, fundo cinza fora dos limites do mapa e UI nativa proporcionalmente menor. Oferece tela cheia com `--fullscreen` e alternância por F11. Em 3440×1440, o jogo ocupa 2400×1440 com barras laterais de 520 pixels. A correção ainda precisa de confirmação visual nas passagens do Windows; compras e rede continuam com limitações documentadas.

## Novidades da 0.7

- Códex de monstros acessível pelo livro **CODEX** no menu original do jogo, junto aos ícones de Craft e Pets, com estados de descoberta e contador de mortes por espécie.
- Registro auxiliar separado por personagem na mesma pasta de save dos dois hosts; preserva o progresso entre instalações/updates sem alterar o save nativo.
- Observação das rotinas ARM originais de desenho e confirmação de derrota; proteção contra contagem duplicada e IDs incompatíveis.
- Interface de consulta com paginação e pausa da partida; importação de histórico antigo não está disponível.
- O [protótipo web privado](https://destinia-web-experimental.leafyheron257.chatgpt.site) usa o mesmo acesso **dentro do menu do jogo**, sem botão Códex na interface do site. Na web, o progresso e o Códex continuam apenas na memória da sessão, sem áudio.

Leia [CODEX.md](docs/CODEX.md), [CHANGELOG.md](CHANGELOG.md) e [RELEASE_0.7.md](docs/RELEASE_0.7.md). Não inclui novos equipamentos, novos monstros nem alterações de combate.

## Novidades da 0.6

- **Configurar Destinia.bat** guarda vídeo por host, áudio, pausa e perfis em `%LOCALAPPDATA%\Destinia\config.ini`, preservando a pasta compartilhada de save entre updates.
- Tamanho da janela e modo tela cheia/janela são lembrados ao fechar; F11 funciona nos dois hosts, com imagem proporcional e mouse alinhado à área central.
- Teclas e botões por ação, perfis nomeados e validação de conflitos. Entradas simultâneas deixam de se anular; perder foco/desconectar libera comandos presos.
- Música/efeitos de 0 a 100%, combinados com o volume original. Silêncio ao perder foco e pausa automática são opcionais; F10 pausa/retoma por padrão.
- Compilação e testes Windows automatizados, pacote x64 com dois EXEs próprios e checksum; DLLs, ferramentas, instaladores, APK e saves permanecem fora do ZIP.

A 0.5 já publicada mantém log TXT, backups verificados, trava de save e seleção global da pasta de progresso. **O BAT de testes usa cópias: o progresso jogado nele não substitui o save principal.** Veja [saves, restauração e logs](docs/SAVES_E_LOGS.md).

A 0.6 preserva zoom out, fundo cinza, câmera, UI nativa, ritmo e conteúdo. Testes automáticos não equivalem a confirmação manual da campanha, visual, áudio ou controle físico no Windows. Resultados em [RELEASE_0.6.md](docs/RELEASE_0.6.md).

## Dependências

As dependências externas devem ser instaladas separadamente no PC do usuário. **Não serão incluídos no arquivo da release instaladores, ferramentas de compilação ou bibliotecas externas que o usuário precise instalar.**

| Dependência | Quando é necessária |
| --- | --- |
| Windows 64 bits e MSYS2 UCRT64 | Ambiente de execução atualmente esperado pelos launchers; instalação padrão em `C:\msys64` |
| SDL2 e Unicorn para UCRT64 | Execução do host e emulação ARM, com suas dependências transitivas disponíveis no mesmo ambiente |
| SDL2_mixer e suporte OGG/MP3 | Áudio, quando habilitado na compilação |
| Compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf | Somente para quem compilar os fontes |
| Python ≥ 3.10 | Somente para testar ou gerar pacotes |
| Windows PowerShell 5.1 | Diagnóstico e configuradores; normalmente já acompanha o Windows |

Instale o MSYS2 pela [página oficial](https://www.msys2.org/) e use o gerenciador de pacotes desse ambiente. Consulte [INSTALL.md](INSTALL.md) para verificar os pacotes e compilar. O pacote Windows exige também o SDL2_mixer, pois os EXEs são compilados com áudio. Instale seus pacotes e dependências transitivas pelo MSYS2; não copie DLLs avulsas.

A pasta `game` é o APK Android extraído, conforme informado pelo autor. Não é um programa para instalar no Windows: o host lê a biblioteca ARM, os assets e o áudio diretamente dessa pasta. Ter esses dados não substitui as dependências listadas acima.

## Compilar com duplo clique

Abra **Compilar Destinia.bat** para compilar os dois hosts. Depois use **Abrir Destinia.bat** para o normal ou **Abrir Destinia Zoom 800x480.bat** para o zoom. Mantenha o BAT e `scripts/compile_windows.sh` na pasta do projeto. O compilador usa o MSYS2 UCRT64 já instalado em `C:\msys64`; não instala nem baixa dependências. Feche o jogo antes de compilar. As ferramentas necessárias estão na seção Dependências e em INSTALL.md.

[Baixar somente os arquivos do compilador](https://raw.githubusercontent.com/sugyua/destinia/main/diagnostico/pacotes/compilar-destinia.zip) para extrair na raiz do projeto existente. Esse auxiliar foi adicionado após a publicação da 0.3; os ZIPs já publicados da 0.2 e 0.3 permanecem inalterados.

## Desenvolvimento e distribuição

Para testar uma instalação existente, execute **[Testar Destinia.bat](Testar%20Destinia.bat)** na raiz do projeto. Ele verifica a inicialização dos dois hosts, usa cópias dos saves e reúne logs/capturas em `relatorios`. No menu, escolha **1 para o novo teste de câmera do host_zoom** ou **2 para o roteiro geral**. O modo guiado geral acompanha testes de gameplay, salvar/reabrir, câmera, mouse, áudio e controle. Veja o [roteiro completo](diagnostico/COMO_TESTAR.md). Não instala dependências nem altera o código do jogo; testes visuais dependem da sua observação.

O relatório é salvo ao longo da sessão e informa quando está parcial. Após cada execução, o diagnóstico também destaca compras observadas, alterações de cash e a rede aguardando resposta; esses avisos não significam que o problema da loja foi corrigido.

Para esta versão, use o diagnóstico incluído no pacote completo de fontes 0.7, com EXEs da mesma versão. Não sobreponha os auxiliares antigos da main: eles podem não incluir as proteções de preferências da 0.6.

Consulte [INSTALL.md](INSTALL.md) para compilar e executar com dados próprios compatíveis. Os launchers existentes continuam voltados ao ambiente de desenvolvimento MSYS2 em `C:\msys64\ucrt64\bin`; não são launchers portáteis.

Para verificar o empacotador, com Python 3.10 ou superior:

```sh
python -m unittest discover -s tests -v
python scripts/package_release.py --out dist/release-0.7
```

No Linux, o comando pode ser `python3`. O empacotador cria ZIP e SHA-256, não compila executáveis. Não sobrescreve artefatos existentes. A reprodução byte a byte foi verificada com a mesma versão de Python/zlib; outras versões podem mudar a compressão.

O [manifesto](release/manifest.json) usa a 0.6 e seu planejamento como base da 0.7. O workflow Windows compila/testa os EXEs e gera um pacote separado; o empacotador de fontes não compila executáveis. Consulte o [roadmap](docs/ROADMAP.md) para as etapas seguintes.

## Melhorias concluídas com sucesso

Os oito itens abaixo foram implementados e tiveram as verificações automatizadas aprovadas. A 0.6 foi aprovada para publicação pelo autor. Isso não substitui a campanha e a inspeção manual no Windows, que permanecem pendentes; veja [verificações](docs/RELEASE_0.6.md).

### Entregues na 0.5

- [x] **01. Log TXT do jogo:** eventos, erros e desempenho em relatórios com rotação e limite de tamanho.
- [x] **02. Backups rotativos de saves:** cópias verificadas antes de carregar o progresso, com retenção limitada.
- [x] **05. Erros tratados e testes de estabilidade:** operações de arquivo, limites ELF, recursos SDL e propagação de falhas emuladas.

A pasta global de save compartilhada entre hosts e atualizações também foi entregue na 0.5 e preservada na 0.6.

### Entregues na 0.6

- [x] **06. EXEs Windows nas releases:** os dois hosts compilados e verificados no Windows, em ZIP próprio com checksum e dependências instaladas separadamente.
- [x] **07. Configurações persistentes:** vídeo por host, preferências globais e restauração dos padrões preservando a pasta de saves.
- [x] **10. Remapeamento de teclado e controle:** teclas e botões por ação, perfis nomeados e validação de conflitos.
- [x] **11. Volumes separados e persistentes:** música e efeitos independentes e silêncio opcional sem foco.
- [x] **12. Pausa e liberação de comandos:** pausa manual/opcional por foco ou controle, com liberação de entradas e retomada do áudio.

## Entrega parcial da 0.7 — Códex de monstros

**Correção após a publicação da 0.7:** o acesso descrito abaixo está na branch `main` e no site. Os ZIPs da tag `v0.7-beta` preservam a versão originalmente publicada. Para testar no Windows, use o código atual com **Compilar Destinia.bat** ou os pacotes de teste do workflow **Verificar Codex no menu do jogo** em [Actions](https://github.com/sugyua/destinia/actions/workflows/verify-codex-menu.yml); não são uma nova release.

O bestiário inicial do **item 42** está implementado nos dois hosts: livro **CODEX**, à direita da barra de ícones do **menu do próprio jogo**, junto de Craft e Pets. Abra o menu durante a partida e toque/clique no livro; **VOLTAR** retorna ao mesmo menu. Setas e confirmar também permitem selecionar o livro; **F8** é um atalho somente com esse menu aberto, quando a tecla não estiver remapeada. Mostra as espécies descobertas, os estados **VISTO/DERROTADO** e **quantos monstros de cada espécie foram mortos**, com paginação. A partida pausa durante a consulta. Nomes de monstros permanecem em inglês nesta primeira etapa.

Os registros ficam em **codex-v1.txt**, na pasta compartilhada de saves, separados pelo slot de personagem e gravados por substituição de arquivo temporário. Não altera o formato dos saves ARM nem recupera mortes antigas. A contagem começa na 0.7; o slot reutilizado após exclusão pelo jogo tem seus registros reiniciados. Trocar apenas de pasta de instalação preserva o Códex quando a pasta de save é a mesma.

Testes automatizados cobrem registro, morte única, reaparecimento, slots, persistência e callbacks originais ARM com estruturas controladas, além de desenho dos painéis. Campanha completa e inspeção visual no Windows continuam pendentes. Veja [como usar o Códex](docs/CODEX.md) e [verificações da 0.7](docs/RELEASE_0.7.md).

**O item 42 completo continua desmarcado:** armas, armaduras e integração com drops exclusivos permanecem na 0.11. Os outros quatro ajustes de interface antes previstos para a 0.7 foram mantidos como pendências da 0.8.

## Atualizações futuras

Das **43 melhorias aprovadas**, **8 foram concluídas** nas versões 0.5 e 0.6; **35 ainda não estão concluídas**, incluindo o Códex entregue parcialmente, somente da **0.8 em diante**. As versões abaixo são metas provisórias, sem datas garantidas, e dependem de investigação e testes.

**A expansão continua incluindo aumento do nível máximo, novos ranks de armas, armaduras e acessórios, itens únicos, conjuntos com bônus, novos títulos, novos pets, drops exclusivos por monstro e um Códex de equipamentos e monstros.** Limites, nomes e valores serão definidos após verificar dados originais e balanceamento.

Consulte o [roadmap detalhado](docs/ROADMAP.md) para dependências, critérios de verificação e referências. A [investigação inicial de pets e títulos](docs/INVESTIGACAO_PETS_TITULOS.md) registra a base original encontrada e as condições para as metas 0.10 (títulos) e 0.11 (pets); nenhum desses conteúdos novos está implementado. A aprovação do planejamento não comprova a viabilidade de todos os itens nem autoriza automaticamente novas releases.

**Novas classes em planejamento:** Mago e Berserker (item 44), com versão de entrega e terceira classe ainda a definir.

**Novo objetivo para a 1.0:** cooperativo pela internet e LAN, registrado como item 43 e ainda não implementado.

**Novos pedidos:** o Códex inicial de monstros e seu contador foram implementados na **0.7**, sujeitos às limitações de verificação acima. Drops exclusivos continuam como meta da **0.10**, e a ampliação do Códex com armas e armaduras fica na **0.11**. Não existe reconstrução automática do histórico antigo.

### Meta 0.8 — Interface e acessibilidade

Separar o tamanho da UI do zoom e melhorar a apresentação para teclado, controle e português. Estes ajustes não entraram na 0.7 e continuam pendentes para a 0.8.

- [ ] **08. Escala de UI independente do zoom:** Escolher tamanho da interface e do texto sem mudar a quantidade de cenário visível; alinhar as regiões clicáveis.
- [ ] **09. Ocultar botões virtuais:** Opção de esconder direcional e botões de toque ao jogar por teclado/controle, mantendo os comandos físicos.
- [ ] **19. Atalhos visíveis nas habilidades:** Mostrar a tecla ou botão configurado junto às habilidades e adaptar os símbolos ao controle usado.
- [ ] **24. Português mais completo e legível:** Completar diálogos e descrições ainda em inglês e investigar suporte a acentos/fontes, preservando espaço e quebras de linha.

### Meta 0.8 — Saves, compatibilidade e loja offline

Preparar a base para alterar inventário e progressão sem misturar a experiência clássica com a expandida.

- [ ] **03. Gravação segura de saves:** Investigar gravação temporária, validação e substituição atômica ao concluir, preservando o save anterior se houver falha.
- [ ] **04. Loja offline sem espera nem crédito indevido:** Retornar ao jogo com aviso quando compras reais estiverem indisponíveis, sem espera pendente nem novo crédito indevido; preservar o saldo existente.
- [ ] **36. Modo clássico e expandido:** Separar as alterações de progressão/conteúdo da experiência original, com perfis e saves identificados para evitar conversões silenciosas.

### Meta 0.9 — Inventário, coleta e assistência opcional

Reduzir tarefas repetitivas e tornar os equipamentos mais fáceis de entender e proteger.

- [ ] **13. Comparador claro de equipamentos:** Exibir diferenças de atributos entre o item selecionado e o equipado, com sinais e cores legíveis; verificar primeiro o comparador original para aprimorá-lo.
- [ ] **14. Organização e filtros do inventário:** Ordenar e filtrar por tipo, rank, nível e preço, sem perder a seleção atual; confirmar o que já existe no jogo.
- [ ] **15. Favoritar e proteger itens:** Marcar equipamentos para evitar venda/desmanche acidentais; pedir confirmação quando apropriado.
- [ ] **16. Filtro de itens no chão:** Escolher quais categorias ou ranks destacar e quais esconder visualmente; não apagar os itens filtrados.
- [ ] **17. Coleta automática opcional:** Coletar ouro e materiais próximos segundo regras configuráveis, preservando a opção de coleta manual.
- [ ] **21. Poções automáticas opcionais:** Usar poções opcionalmente por limites de vida/mana configurados, respeitando estoque e recarga; a assistência deve ser ativada pelo jogador.
- [ ] **22. Nomes e ranks dos drops por tecla:** Exibir nomes e raridades dos itens no chão ao segurar ou alternar uma tecla, com contraste adequado.
- [ ] **29. Redistribuição de pontos acessível offline:** Verificar o reset existente e oferecer uma forma clara de redistribuir atributos/habilidades offline, com custo interno a definir e sem compra real.

### Meta 0.10 — Progressão expandida, equipamentos e títulos

Aumentar o nível máximo e acrescentar ranks, itens inéditos, conjuntos, receitas, recompensas offline, novos títulos e drops exclusivos por monstro.

- [ ] **25. Aumentar o nível máximo:** Aumentar o nível máximo após verificar o limite original e revisar XP, pontos, atributos, equipamentos, inimigos, HUD e saves; novo limite ainda a definir.
- [ ] **26. Novos ranks de armas:** Criar graus acima do maior atual, com nomes e progressão a definir, incluindo atributos, requisitos, obtenção e identificação visual.
- [ ] **27. Novos ranks de armaduras e acessórios:** Adicionar ranks de armaduras e acessórios, equilibrando defesa, novas armas e inimigos.
- [ ] **28. Mais receitas de fabricação:** Adicionar receitas e usos para materiais, incluindo obtenção dos novos equipamentos; o jogo já possui fabricação.
- [ ] **30. Ganhar cash jogando:** Definir recompensas offline por objetivos, desafios ou missões, com limites e economia equilibrada; independente do aviso de compra indisponível.
- [ ] **37. Novos itens únicos:** Criar armas, armaduras e acessórios inéditos, com identidade, atributos, efeitos e formas de obtenção próprios, além dos novos ranks.
- [ ] **38. Novos conjuntos de equipamentos:** Adicionar conjuntos com bônus por combinar peças; verificar primeiro o sistema original e definir bônus e número de peças.
- [ ] **40. Novos títulos:** Ampliar o sistema original com títulos conquistados por objetivos do jogo, com condições, nomes e eventuais bônus a definir; validar aquisição, seleção, exibição e persistência no modo expandido.
- [ ] **41. Drops de armas e armaduras exclusivos de monstros específicos:** Associar equipamentos exclusivos a espécies ou chefes determinados, com chances e requisitos a definir; validar que esses itens só caiam dos monstros previstos, sem remover os drops existentes do modo clássico.

### Meta 0.11 — Missões, armazenamento, desafios, pets e Códex

Adicionar baú compartilhado, informação de missões, dificuldades opcionais, contratos offline, novos pets e ampliar o Códex com equipamentos.

- [ ] **18. Baú compartilhado entre personagens:** Transferir itens entre personagens pelo armazenamento local, com proteção contra perda/duplicação.
- [ ] **23. Rastreador de missão:** Mostrar objetivo ativo e, quando houver coordenadas conhecidas, marcador de destino; verificar os recursos originais existentes.
- [ ] **32. Dificuldades opcionais:** Perfis de dificuldade com ajustes de inimigos e recompensas; preservar uma experiência equivalente à original.
- [ ] **35. Contratos repetíveis offline:** Objetivos opcionais de caça/coleta para dar propósito ao pós-jogo, com recompensas e repetição controladas.
- [ ] **39. Novos pets:** Acrescentar mascotes ao sistema original, com aparência, animações, atributos, habilidades, evolução e obtenção offline a definir; validar IDs, comportamento e salvamento no modo expandido, preservando os pets existentes.
- [ ] **42. Códex de equipamentos e monstros:** Entrega em fases: bestiário com monstros vistos/derrotados e contador por espécie entregue na 0.7 (fase parcial); armas e armaduras encontradas/obtidas e integração com drops exclusivos ficam na 0.11. Preservar descobertas por personagem entre sessões e ocultar conteúdos ainda não descobertos; marcar o item completo somente após entregar as duas fases.

### Meta 0.12 — Pós-jogo e masmorras

Ampliar a duração do jogo com Novo Jogo+, desafio de chefes e masmorras variáveis.

- [ ] **31. Novo Jogo+:** Reiniciar a campanha mantendo progressão aprovada e aumentando os desafios/recompensas, com regras claras do que é preservado.
- [ ] **33. Desafio de chefes:** Arena ou sequência de chefes já existentes, com regras e recompensas próprias e sem exigir campanha inédita.
- [ ] **34. Masmorras variáveis:** Estudar masmorras geradas ou montadas com salas existentes, se os formatos nativos permitirem; validar colisões, saídas, inimigos e persistência.

### Meta 1.0 — Cooperativo pela internet e LAN

- [ ] **43. Cooperativo pela internet e LAN:** Permitir que jogadores participem da mesma sessão, com personagens, inimigos, combate e mudanças de mapa sincronizados. Esta é uma meta nova para a 1.0; ainda não está implementada nem tem viabilidade técnica comprovada no port.

O planejamento da **0.8 até a 0.12 permanece preservado**. A preparação para o cooperativo deverá começar antes da 1.0, em etapas após a 0.12: investigar o MULTI MODE original e suas rotinas, provar uma sessão mínima entre duas instâncias, definir progresso/recompensas e testar os modos internet/LAN, latência, desconexão, reconexão e integridade dos saves. A existência do menu multiplayer original não comprova um modo cooperativo funcional.

O autor definiu as duas modalidades: **internet e rede local (LAN)**. Limite de jogadores, criação/entrada em salas, conteúdo compartilhado e regras de XP, drops e missões serão definidos na etapa de projeto. O escopo inicial é o port Windows; suporte a navegador, iOS ou partidas entre plataformas exige avaliação própria. A versão 1.0 só poderá ser considerada entregue com uma sessão cooperativa real validada, sem comprometer o modo solo nem os saves.

### Novas classes — versão a definir

- [ ] **44. Novas classes: Mago e Berserker:** Classes adicionais em planejamento, solicitadas pelo autor. Mago e Berserker são os dois nomes definidos até agora; a terceira classe mencionada continua a definir. Nenhuma dessas novas classes foi implementada.

Habilidades, armas/armaduras permitidas, atributos, animações e progressão serão definidos na etapa de projeto. Antes de escolher a versão de entrega, investigar os IDs e limites nativos de classes, a seleção/criação de personagens, os equipamentos e os campos dos saves; validar criação, combate, evolução e salvar/reabrir com os novos personagens. Preservar as classes e os saves existentes e avaliar a compatibilidade com o cooperativo planejado para a 1.0. O pedido registra o objetivo; ainda não confirma que a biblioteca ARM permite acrescentar novas classes sem outras alterações.

Os recursos opcionais terão configurações próprias. Mudanças permanentes de progressão/conteúdo dependem de perfis e saves identificados para os modos clássico e expandido, sem conversão silenciosa. Antes de ampliar o jogo, será verificado o que já existe para melhorar os sistemas atuais. A publicação de cada nova release continuará dependendo da revisão das verificações e da aprovação do autor.

## Publicação

Versão **v0.7-beta**, com publicação autorizada pelo autor em 21/10/2026. O workflow só cria a release após compilar e testar os dois hosts no Windows. A validação manual no Windows continua pendente; testes automatizados e de inicialização não comprovam uma campanha completa. A versão permanece **Beta**. As releases anteriores continuam disponíveis.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
