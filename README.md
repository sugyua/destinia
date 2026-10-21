# Destinia para Windows — 0.5 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**Versão atual: 0.5 Beta, distribuída em código-fonte. Publicação aprovada pelo autor com validação manual no Windows ainda pendente.**

**[Baixar a versão 0.5](https://github.com/sugyua/destinia/releases/tag/v0.5-beta)** · [Versão 0.4](https://github.com/sugyua/destinia/releases/tag/v0.4-beta) · [Versão 0.3](https://github.com/sugyua/destinia/releases/tag/v0.3-beta) · [Versão 0.2](https://github.com/sugyua/destinia/releases/tag/v0.2-beta) · [Todas as releases](https://github.com/sugyua/destinia/releases) O pacote contém os fontes, não um jogo pronto para executar. Veja [instalação](INSTALL.md), [changelog](CHANGELOG.md) e [verificações da candidata 0.5](docs/RELEASE_0.5.md).

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

## Novidades da 0.5

- Log TXT automático em `relatorios`, com eventos observáveis, erros e desempenho resumido; rotação de três arquivos de até 2 MiB por sessão e dez sessões por host.
- Backups verificados do save antes de carregar, com dez snapshots e proteção contra abertura simultânea da mesma pasta pelos hosts novos.
- **Configurar pasta de saves.bat** para escolher uma pasta comum aos dois hosts e a diferentes instalações, mantendo o progresso entre updates. O padrão continua sendo `%LOCALAPPDATA%\Destinia\save`, também usado nas versões anteriores.
- Correções de operações de arquivo com tamanho zero, validação ELF, tratamento de falhas SDL e códigos de saída de erros emulados.

Use os inicializadores normais para continuar sua partida. **O BAT de testes usa cópias: o progresso jogado nele não substitui o save principal.** O configurador não copia nem sobrescreve saves. Configuração personalizada vale nos hosts 0.5 e futuros; mantenha o padrão para compartilhar também com a 0.2–0.4. Veja [saves, restauração e logs](docs/SAVES_E_LOGS.md).

Os testes locais estão descritos em [RELEASE_0.5.md](docs/RELEASE_0.5.md). A publicação foi aprovada pelo autor em 21/10/2026. A confirmação de gameplay/UI/áudio no Windows continua pendente. O ZIP contém fontes e não inclui dependências externas.

## Dependências

As dependências externas devem ser instaladas separadamente no PC do usuário. **Não serão incluídos no arquivo da release instaladores, ferramentas de compilação ou bibliotecas externas que o usuário precise instalar.**

| Dependência | Quando é necessária |
| --- | --- |
| Windows 64 bits e MSYS2 UCRT64 | Ambiente de execução atualmente esperado pelos launchers; instalação padrão em `C:\msys64` |
| SDL2 e Unicorn para UCRT64 | Execução do host e emulação ARM, com suas dependências transitivas disponíveis no mesmo ambiente |
| SDL2_mixer e suporte OGG/MP3 | Áudio, quando habilitado na compilação |
| Compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf | Somente para quem compilar os fontes |
| Python ≥ 3.9 | Somente para testar ou gerar o pacote de fontes |
| Windows PowerShell 5.1 | Diagnóstico e configurador de saves; normalmente já acompanha o Windows |

Instale o MSYS2 pela [página oficial](https://www.msys2.org/) e use o gerenciador de pacotes desse ambiente. Consulte [INSTALL.md](INSTALL.md) para verificar os pacotes e compilar. As versões exatas de runtime e DLLs da futura release executável ainda precisam ser confirmadas por uma compilação Windows.

A pasta `game` é o APK Android extraído, conforme informado pelo autor. Não é um programa para instalar no Windows: o host lê a biblioteca ARM, os assets e o áudio diretamente dessa pasta. Ter esses dados não substitui as dependências listadas acima.

## Compilar com duplo clique

Abra **Compilar Destinia.bat** para compilar os dois hosts. Depois use **Abrir Destinia.bat** para o normal ou **Abrir Destinia Zoom 800x480.bat** para o zoom. Mantenha o BAT e `scripts/compile_windows.sh` na pasta do projeto. O compilador usa o MSYS2 UCRT64 já instalado em `C:\msys64`; não instala nem baixa dependências. Feche o jogo antes de compilar. As ferramentas necessárias estão na seção Dependências e em INSTALL.md.

[Baixar somente os arquivos do compilador](https://raw.githubusercontent.com/sugyua/destinia/main/diagnostico/pacotes/compilar-destinia.zip) para extrair na raiz do projeto existente. Esse auxiliar foi adicionado após a publicação da 0.3; os ZIPs já publicados da 0.2 e 0.3 permanecem inalterados.

## Desenvolvimento e distribuição

Para testar uma instalação existente, execute **[Testar Destinia.bat](Testar%20Destinia.bat)** na raiz do projeto. Ele verifica a inicialização dos dois hosts, usa cópias dos saves e reúne logs/capturas em `relatorios`. No menu, escolha **1 para o novo teste de câmera do host_zoom** ou **2 para o roteiro geral**. O modo guiado geral acompanha testes de gameplay, salvar/reabrir, câmera, mouse, áudio e controle. Veja o [roteiro completo](diagnostico/COMO_TESTAR.md). Não instala dependências nem altera o código do jogo; testes visuais dependem da sua observação.

O relatório é salvo ao longo da sessão e informa quando está parcial. Após cada execução, o diagnóstico também destaca compras observadas, alterações de cash e a rede aguardando resposta; esses avisos não significam que o problema da loja foi corrigido.

[Baixar o pacote de testes](https://raw.githubusercontent.com/sugyua/destinia/main/diagnostico/pacotes/testes-destinia.zip) e extrair na raiz do projeto existente. O pacote contém somente o BAT, seu executor PowerShell, o leitor de configuração compartilhada e as instruções. Para testar a 0.5, extraia e compile primeiro os fontes completos da versão.

Consulte [INSTALL.md](INSTALL.md) para compilar e executar com dados próprios compatíveis. Os launchers existentes continuam voltados ao ambiente de desenvolvimento MSYS2 em `C:\msys64\ucrt64\bin`; não são launchers portáteis.

Para verificar o empacotador, com Python 3.9 ou superior:

```sh
python -m unittest discover -s tests -v
python scripts/package_release.py --out dist/candidata-0.5
```

No Linux, o comando pode ser `python3`. O empacotador cria ZIP e SHA-256, não compila executáveis. Não sobrescreve artefatos existentes. A reprodução byte a byte foi verificada com a mesma versão de Python/zlib; outras versões podem mudar a compressão.

O [manifesto](release/manifest.json) usa a 0.4 como base de comparação. A 0.5 acrescenta logs, backups, configuração de saves e correções de estabilidade, preservando o zoom e a UI da 0.4. Consulte o [roadmap](docs/ROADMAP.md) para as etapas seguintes.

## Atualizações futuras

**37 melhorias aprovadas para planejamento em 21/10/2026.** Os itens 01, 02 e 05 estão implementados na 0.5, publicada com validação manual no Windows pendente; os demais continuam planejados. A ordem prioriza mudanças no host e proteção dos saves antes das alterações na biblioteca ARM e no conteúdo. As versões são metas provisórias, sem datas garantidas, e podem ser divididas ou reordenadas conforme a investigação.

**A expansão inclui aumento do nível máximo, novos ranks de armas, armaduras e acessórios, itens únicos inéditos e conjuntos com bônus.** O novo limite de nível, os nomes dos ranks e os valores dos equipamentos ainda serão definidos após verificar os dados originais e o balanceamento.

O log TXT está incluído na **0.5**. Consulte o [roadmap detalhado](docs/ROADMAP.md) para dificuldades, dependências, critérios de verificação e referências da pesquisa. Caixas marcadas indicam implementação na 0.5. Aprovar o planejamento não comprova a viabilidade dos itens restantes nem autoriza automaticamente a publicação.
### Meta 0.5 — Diagnóstico e proteção inicial

Prioridade imediata: registrar falhas, proteger o progresso e corrigir erros verificáveis do host.

- [x] **01. Log TXT do jogo:** Registrar eventos importantes, mapas, saves, erros e desempenho em TXT na pasta `relatorios`, com limite de tamanho e retenção.
- [x] **02. Backups rotativos de saves:** Guardar cópias datadas antes de iniciar o jogo/atualizar, com restauração orientada e quantidade limitada.
- [x] **05. Erros tratados e testes de estabilidade:** Corrigir operações fread/fwrite de tamanho zero, validar limites ELF e recursos SDL, propagar falhas reais de emulação e testar os caminhos afetados.

### Meta 0.6 — Configurações, comandos e distribuição

Preferências persistentes, comandos configuráveis e executáveis Windows verificados.

- [ ] **06. EXEs Windows prontos nas releases:** Disponibilizar os EXEs dos dois hosts, compilados e verificados no Windows; dependências externas continuam instaladas separadamente, sem serem incluídas no pacote.
- [ ] **07. Configurações persistentes:** Salvar preferências de vídeo, janela e comandos em arquivo próprio, com opção de restaurar padrões.
- [ ] **10. Remapear teclado e controle:** Escolher teclas e botões por ação e guardar perfis; ampliar o suporte atual de comandos fixos.
- [ ] **11. Volumes separados e persistentes:** Controles independentes para música e efeitos, inclusive silenciar ao perder foco se o usuário quiser.
- [ ] **12. Pausa ao perder foco ou controle:** Permitir pausa opcional ao perder foco ou desconectar o controle e garantir liberação das direções pressionadas.

### Meta 0.7 — Interface e acessibilidade

Separar o tamanho da UI do zoom e melhorar a apresentação para teclado, controle e português.

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

### Meta 0.10 — Progressão expandida e novos equipamentos

Aumentar o nível máximo e acrescentar ranks, itens inéditos, conjuntos, receitas e recompensas offline.

- [ ] **25. Aumentar o nível máximo:** Aumentar o nível máximo após verificar o limite original e revisar XP, pontos, atributos, equipamentos, inimigos, HUD e saves; novo limite ainda a definir.
- [ ] **26. Novos ranks de armas:** Criar graus acima do maior atual, com nomes e progressão a definir, incluindo atributos, requisitos, obtenção e identificação visual.
- [ ] **27. Novos ranks de armaduras e acessórios:** Adicionar ranks de armaduras e acessórios, equilibrando defesa, novas armas e inimigos.
- [ ] **28. Mais receitas de fabricação:** Adicionar receitas e usos para materiais, incluindo obtenção dos novos equipamentos; o jogo já possui fabricação.
- [ ] **30. Ganhar cash jogando:** Definir recompensas offline por objetivos, desafios ou missões, com limites e economia equilibrada; independente do aviso de compra indisponível.
- [ ] **37. Novos itens únicos:** Criar armas, armaduras e acessórios inéditos, com identidade, atributos, efeitos e formas de obtenção próprios, além dos novos ranks.
- [ ] **38. Novos conjuntos de equipamentos:** Adicionar conjuntos com bônus por combinar peças; verificar primeiro o sistema original e definir bônus e número de peças.

### Meta 0.11 — Missões, armazenamento e desafios configuráveis

Adicionar baú compartilhado, informação de missões, dificuldades opcionais e contratos offline.

- [ ] **18. Baú compartilhado entre personagens:** Transferir itens entre personagens pelo armazenamento local, com proteção contra perda/duplicação.
- [ ] **23. Rastreador de missão:** Mostrar objetivo ativo e, quando houver coordenadas conhecidas, marcador de destino; verificar os recursos originais existentes.
- [ ] **32. Dificuldades opcionais:** Perfis de dificuldade com ajustes de inimigos e recompensas; preservar uma experiência equivalente à original.
- [ ] **35. Contratos repetíveis offline:** Objetivos opcionais de caça/coleta para dar propósito ao pós-jogo, com recompensas e repetição controladas.

### Meta 0.12 — Pós-jogo e masmorras

Ampliar a duração do jogo com Novo Jogo+, desafio de chefes e masmorras variáveis.

- [ ] **31. Novo Jogo+:** Reiniciar a campanha mantendo progressão aprovada e aumentando os desafios/recompensas, com regras claras do que é preservado.
- [ ] **33. Desafio de chefes:** Arena ou sequência de chefes já existentes, com regras e recompensas próprias e sem exigir campanha inédita.
- [ ] **34. Masmorras variáveis:** Estudar masmorras geradas ou montadas com salas existentes, se os formatos nativos permitirem; validar colisões, saídas, inimigos e persistência.

Os recursos opcionais terão configurações próprias. Mudanças permanentes de progressão/conteúdo dependem de perfis e saves identificados para os modos clássico e expandido, sem conversão silenciosa. Antes de ampliar o jogo, será verificado o que já existe para melhorar os sistemas atuais. A publicação de cada nova release continuará dependendo da revisão das verificações e da aprovação do autor.

## Publicação

Versão **v0.5-beta**, publicada com aprovação do autor em 21/10/2026 após os testes locais. A validação manual no Windows continua pendente; testes automatizados e de inicialização não comprovam uma campanha completa. A versão permanece **Beta**. As releases anteriores continuam disponíveis.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
