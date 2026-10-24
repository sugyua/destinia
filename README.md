# Destinia para Windows — 0.9 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**[Baixar Destinia 0.9 Beta](https://github.com/sugyua/destinia/releases/tag/v0.9-beta)** — ZIP Windows com os dois EXEs e inicializadores, ZIP de fontes e checksums. Dependências e dados originais do jogo são instalados separadamente; consulte a seção **Dependências**.

[Instalação e dependências](INSTALL.md) · [Releases e histórico de mudanças](https://github.com/sugyua/destinia/releases) · [Teste web privado](https://destinia-web-experimental.leafyheron257.chatgpt.site).


## Novidades da 0.9

- **FC nos status** e comparador de equipamentos próprios no inventário: atual → previsto, ganho/perda e diferenças de ATK/DEF/PV/PM máximos/crítico. Inclui buffs ativos; não diminui ao perder vida atual. [Fórmula e limites](docs/FC.md).
- **PT-BR padrão**; inglês em **Configurar Destinia → 9 — Idioma**, no seletor web ou `--lang en`. Tradução parcial, escolha explícita preservada.
- **Nomes de drops:** segureV (remapeável como `nomes_drops`); web também tem botão. Não altera alcance de coleta ou os itens.
- README enxuto e redundâncias removidas, preservando saves, dados necessários, testes e releases anteriores.

[Notas completas e instalação](docs/RELEASE_0.9.md). As funcionalidades ainda não concluídas estão na **0.9.1**, abaixo; não acompanham a 0.9.

## Releases em ordem de lançamento

Da mais recente para a mais antiga. As releases anteriores e seus downloads são preservados.

| Release | Lançamento (Brasília) |
| --- | --- |
| [0.9 Beta — FC e comparador](https://github.com/sugyua/destinia/releases/tag/v0.9-beta) | 24/10/2026 |
| [0.8 Beta — interface e saves](https://github.com/sugyua/destinia/releases/tag/v0.8-beta) | 24/10/2026 |
| [0.7 Beta — imagens e câmera, fix2](https://github.com/sugyua/destinia/releases/tag/v0.7-codex-imagens-fix2) | 22/10/2026 05:54 |
| [0.7 Beta — correção de abertura, fix1](https://github.com/sugyua/destinia/releases/tag/v0.7-codex-imagens-fix1) | 21/10/2026 18:42 |
| [0.7 Beta — Códex com imagens](https://github.com/sugyua/destinia/releases/tag/v0.7-codex-imagens) | 21/10/2026 18:20 |
| [0.7 Beta — Códex inicial](https://github.com/sugyua/destinia/releases/tag/v0.7-beta) | 21/10/2026 13:34 |
| [0.6 Beta](https://github.com/sugyua/destinia/releases/tag/v0.6-beta) | 21/10/2026 08:55 |
| [0.5 Beta](https://github.com/sugyua/destinia/releases/tag/v0.5-beta) | 21/10/2026 07:56 |
| [0.4 Beta](https://github.com/sugyua/destinia/releases/tag/v0.4-beta) | 21/10/2026 02:53 |
| [0.3 Beta](https://github.com/sugyua/destinia/releases/tag/v0.3-beta) | 20/10/2026 21:18 |
| [0.2 Beta](https://github.com/sugyua/destinia/releases/tag/v0.2-beta) | 20/10/2026 19:35 |

[Ver todas as releases no GitHub](https://github.com/sugyua/destinia/releases)

## Arquitetura

O host em C++17 executa a biblioteca ARM original do Destinia Android com Unicorn. SDL2 apresenta o framebuffer RGB565 e recebe entradas; SDL2_mixer habilita áudio opcional. A lógica original continua na biblioteca ARM.

| Diretório | Finalidade |
| --- | --- |
| `host` | Host 400×240 |
| `common` | Logs, backups, configuração compartilhada e validação ELF usados pelos dois hosts |
| `host_zoom` | Zoom out 800×480, mapas pequenos centralizados com fundo cinza, tela cheia proporcional com F11, tradução PT e trace de viewport |
| `game` | APK original do Destinia Android extraído; recursos necessários em execução, não incluídos no ZIP curado de fontes |
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
- Português brasileiro por padrão nos dois hosts, inglês opcional e trace F9 no `host_zoom`.
- Códex dentro do menu do jogo, com retratos, descobertas, contagem de mortes e detalhes de monstros.
- Configuração de UI/controles, áudio, comandos, save compartilhado e modos clássico/expandido.

A presença no código não equivale a validação completa de gameplay. O host_zoom usa framebuffer 800×480 para zoom out, mapas pequenos centralizados, fundo cinza fora dos limites do mapa e UI nativa proporcionalmente menor. Oferece tela cheia com `--fullscreen` e alternância por F11. Em 3440×1440, o jogo ocupa 2400×1440 com barras laterais de 520 pixels. O autor confirmou teste no PC e iPhone; isso não comprova campanha completa. Compras reais e recursos online continuam indisponíveis.

## Dependências

As dependências externas devem ser instaladas separadamente no PC do usuário. **Não serão incluídos no arquivo da release instaladores, ferramentas de compilação ou bibliotecas externas que o usuário precise instalar.**

| Dependência | Quando é necessária |
| --- | --- |
| Windows 64 bits e MSYS2 UCRT64 | Ambiente de execução atualmente esperado pelos launchers; instalação padrão em `C:\msys64` |
| SDL2 e Unicorn para UCRT64 | Execução do host e emulação ARM, com suas dependências transitivas disponíveis no mesmo ambiente |
| zlib para UCRT64 | Leitura dos sprites originais comprimidos das miniaturas do Códex nas compilações atuais da `main` |
| SDL2_mixer e suporte OGG/MP3 | Áudio, quando habilitado na compilação |
| Compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf | Somente para quem compilar os fontes |
| Python ≥ 3.10 | Somente para testar ou gerar pacotes |
| Windows PowerShell 5.1 | Inicializadores (validacao de VERSION), diagnóstico e configuradores; normalmente já acompanha o Windows |

Instale o MSYS2 pela [página oficial](https://www.msys2.org/) e use o gerenciador de pacotes desse ambiente. Consulte [INSTALL.md](INSTALL.md) para verificar os pacotes e compilar. O pacote Windows exige também o SDL2_mixer, pois os EXEs são compilados com áudio. Instale seus pacotes e dependências transitivas pelo MSYS2; não copie DLLs avulsas.

A pasta `game` é o APK Android extraído, conforme informado pelo autor. Não é um programa para instalar no Windows: o host lê a biblioteca ARM, os assets e o áudio diretamente dessa pasta. Ter esses dados não substitui as dependências listadas acima.

## Compilar com duplo clique

Abra **Compilar Destinia.bat** para compilar os dois hosts. Depois use **Abrir Destinia.bat** para o normal ou **Abrir Destinia Zoom 800x480.bat** para o zoom. Mantenha o BAT e `scripts/compile_windows.sh` na pasta do projeto. O compilador usa o MSYS2 UCRT64 já instalado em `C:\msys64`; não instala nem baixa dependências. Feche o jogo antes de compilar. As ferramentas necessárias estão na seção Dependências e em INSTALL.md.

O compilador e seus scripts atuais acompanham o [pacote de fontes da release](https://github.com/sugyua/destinia/releases); use sempre os arquivos da mesma versão.

## Desenvolvimento e distribuição

Para testar uma instalação existente, execute **[Testar Destinia.bat](Testar%20Destinia.bat)** na raiz do projeto. Ele verifica a inicialização dos dois hosts, usa cópias dos saves e reúne logs/capturas em `relatorios`. No menu, escolha **1 para o novo teste de câmera do host_zoom** ou **2 para o roteiro geral**. O modo guiado geral acompanha testes de gameplay, salvar/reabrir, câmera, mouse, áudio e controle. Veja o [roteiro completo](diagnostico/COMO_TESTAR.md). Não instala dependências nem altera o código do jogo; testes visuais dependem da sua observação.

O relatório é salvo ao longo da sessão e informa quando está parcial. Após cada execução, o diagnóstico também destaca compras observadas, alterações de cash e a rede aguardando resposta; esses avisos não significam que o problema da loja foi corrigido.

Para esta versão, use o diagnóstico incluído no pacote completo de fontes 0.9, com EXEs da mesma versão. Não sobreponha os auxiliares antigos da main: eles podem não incluir as proteções de preferências da 0.6.

Consulte [INSTALL.md](INSTALL.md) para compilar e executar com dados próprios compatíveis. Os launchers existentes continuam voltados ao ambiente de desenvolvimento MSYS2 em `C:\msys64\ucrt64\bin`; não são launchers portáteis.

Para verificar o empacotador, com Python 3.10 ou superior:

```sh
python -m unittest discover -s tests -v
python scripts/package_release.py --out dist/release-0.9
```

No Linux, o comando pode ser `python3`. O empacotador cria ZIP e SHA-256, não compila executáveis. Não sobrescreve artefatos existentes. A reprodução byte a byte foi verificada com a mesma versão de Python/zlib; outras versões podem mudar a compressão.

O [manifesto](release/manifest.json) identifica a base e os arquivos da 0.9. O workflow Windows compila/testa os EXEs e gera um pacote separado; o empacotador de fontes não compila executáveis. Consulte o [roadmap](docs/ROADMAP.md) para as etapas seguintes.

## Atualizações futuras

Das melhorias aprovadas, **29 ainda não estão concluídas**, incluindo o Códex entregue parcialmente, nas próximas versões. As versões abaixo são metas provisórias, sem datas garantidas, e dependem de investigação e testes.

**A expansão continua incluindo aumento do nível máximo, novos ranks de armas, armaduras e acessórios, itens únicos, conjuntos com bônus, novos títulos, novos pets, drops exclusivos por monstro e um Códex de equipamentos e monstros.** Limites, nomes e valores serão definidos após verificar dados originais e balanceamento.

Consulte o [roadmap detalhado](docs/ROADMAP.md) para dependências, critérios de verificação e referências. A [investigação inicial de pets e títulos](docs/INVESTIGACAO_PETS_TITULOS.md) registra a base original encontrada e as condições para as metas 0.10 (títulos) e 0.11 (pets); nenhum desses conteúdos novos está implementado. A aprovação do planejamento não comprova a viabilidade de todos os itens nem autoriza automaticamente novas releases.

**Novas classes em planejamento:** Mago, Berserker, Paladino, Monge, Necromante, Druida, Engenheiro e Granadeiro (item 44). Com as três atuais, a meta passa a ser **pelo menos 11 classes**. Armas exclusivas por classe (item 45): **foice para Necromante** e **granadas para Granadeiro**; demais equipamentos e versão de entrega a definir.

**Novo objetivo para a 1.0:** cooperativo pela internet e LAN, registrado como item 43 e ainda não implementado.

**Próximos pedidos do Códex:** Drops exclusivos continuam como meta da **0.10**, e a ampliação do Códex com armas e armaduras fica na **0.11**. Não existe reconstrução automática do histórico antigo.

### Meta 0.9.1 — complementos de inventário, coleta e assistência opcional

Reduzir tarefas repetitivas e tornar os equipamentos mais fáceis de entender e proteger.

- [ ] **13. Complementar o comparador:** a 0.9 entrega FC/ATK/DEF/PV/PM máximos/crítico; ampliar o detalhamento dos outros atributos e contextos de itens.
- [ ] **14. Organização e filtros do inventário:** Ordenar e filtrar por tipo, rank, nível e preço, sem perder a seleção atual; confirmar o que já existe no jogo.
- [ ] **15. Favoritar e proteger itens:** Marcar equipamentos para evitar venda/desmanche acidentais; pedir confirmação quando apropriado.
- [ ] **16. Filtro de itens no chão:** Escolher quais categorias ou ranks destacar e quais esconder visualmente; não apagar os itens filtrados.
- [ ] **17. Coleta automática opcional:** Coletar ouro e materiais próximos segundo regras configuráveis, preservando a opção de coleta manual.
- [ ] **21. Poções automáticas opcionais:** Usar poções opcionalmente por limites de vida/mana configurados, respeitando estoque e recarga; a assistência deve ser ativada pelo jogador.

- [ ] **29. Redistribuição de pontos acessível offline:** Verificar o reset existente e oferecer uma forma clara de redistribuir atributos/habilidades offline, consumindo o pergaminho original, vendido por 10.000 gold no vendedor de poções, sem compra real.

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

### Novas classes e armas exclusivas — versão a definir

- [ ] **44. Oito novas classes, totalizando pelo menos 11:** Preservar as três classes atuais e acrescentar **Mago, Berserker, Paladino, Monge, Necromante, Druida, Engenheiro e Granadeiro**. As cinco propostas de Paladino a Engenheiro foram aprovadas pelo autor, que também acrescentou Granadeiro. São opções adicionais futuras do port, ainda não entregues; Berserker já tem uma evolução original, investigada abaixo. As versões de entrega permanecem condicionadas às provas técnicas.
- [ ] **45. Armas exclusivas por classe:** Planejar famílias de armas e regras de equipamento próprias para algumas classes. Associações definidas pelo autor: **Necromante → foice** e **Granadeiro → granadas**. As demais armas, exclusividades e ferramentas adicionais do Granadeiro serão definidas na etapa de projeto, junto das restrições de uso e do balanceamento.

| Nova classe | Direção de design aprovada | Arma definida pelo autor |
| --- | --- | --- |
| Mago | Identidade e habilidades a detalhar | A definir |
| Berserker | A evolução original do Guerreiro é candidata ao primeiro ensaio isolado; opção inicial e habilidades próprias ainda a detalhar | A definir |
| Paladino | Resistência, proteção, cura moderada e auras para coop | A definir |
| Monge | Combos, esquivas, contra-ataques e mobilidade | A definir |
| Necromante | Lacaios, maldições, drenagem de vida e enfraquecimento | Foice |
| Druida | Natureza, veneno, raízes, regeneração e interação com animais; transformações como possibilidade posterior | A definir |
| Engenheiro | Bombas, armadilhas e torres, com preparação e posicionamento | A definir |
| Granadeiro | Combate com granadas; habilidades e recursos adicionais a detalhar | Granadas |

Cada classe deve funcionar no modo solo e oferecer sinergias no cooperativo. Diferenciar os dispositivos do Engenheiro das armas e habilidades do Granadeiro faz parte do projeto das classes; ainda não foram definidos tipos de granada, valores de dano ou habilidades específicas.

**Dependências antes de escolher a versão:** investigar IDs e limites nativos de classes e armas, seleção/criação de personagens, requisitos de equipamento, animações, habilidades e campos dos saves. Definir progressão, atributos, obtenção, fabricação, drops e apresentação das novas armas no inventário e no Códex. Validar criação, equipar/recusar armas conforme a classe, combate, evolução e salvar/reabrir, preservando classes e saves existentes. Avaliar também a compatibilidade com o cooperativo planejado para a 1.0. O planejamento ainda não comprova a viabilidade de ampliar esses sistemas na biblioteca ARM original.

**Análise e antecipação do primeiro teste:** Berserker já tem base original como evolução do Guerreiro. Recomenda-se um ensaio com personagem e pasta de save separados durante a preparação da **0.8**, antes de concluir toda a versão; isso não conta como uma quarta classe-base implementada. A primeira classe adicional recomendada é **Paladino**, com investigação junto da 0.8 e meta condicional de uma prévia na **0.10**, após provar identidade/tabelas/persistência e a base de inventário da 0.9. Engenheiro também merece investigação por existir `Summon Machine Gun` nos dados originais. As demais entregas continuam sem versão definida. Evidências, limites e critérios em [investigação de classes](docs/INVESTIGACAO_CLASSES.md).

Os recursos opcionais terão configurações próprias. Mudanças permanentes de progressão/conteúdo dependem de perfis e saves identificados para os modos clássico e expandido, sem conversão silenciosa. Antes de ampliar o jogo, será verificado o que já existe para melhorar os sistemas atuais. A publicação de cada nova release continuará dependendo da revisão das verificações e da aprovação do autor.

## Complementos ao planejamento — propostas de versões futuras

> Registro do relatório de continuidade de 23/10/2026. **Tudo abaixo é planejamento, não implementação.** Preserva o escopo aprovado da 0.8–0.12. As versões sugeridas dependem de avaliação técnica e não representam datas ou entregas garantidas.

### Códex organizado — complemento para a 0.9.1

- [ ] Separar **Monstros comuns** e **Chefes / Bosses**, mantendo cada chefe em uma entrada própria e preservando imagens, descobertas, contador de mortes e informações existentes.
- [ ] Confirmar nos dados originais a classificação de monstros, minibosses e chefes, com IDs e referências; não classificar somente por nome ou aparência.
- [ ] Mostrar informações verificadas: região/mapa, combate, variantes e drops. Associar armas exclusivas às entradas correspondentes quando forem implementadas.
- [ ] Preservar navegação por teclado, mouse e controles virtuais nos dois hosts. A categorização do Códex não altera o comportamento dos monstros.

### Uma arma exclusiva para cada boss — complemento sugerido para a 0.10

- [ ] Planejar **pelo menos uma arma exclusiva para cada chefe confirmado**, com identidade própria, sem substituir equipamentos existentes.
- [ ] Especificar boss/ID, nome e tipo da arma, classes permitidas, aparência, atributos, efeitos, raridade, obtenção e chance de drop a definir no balanceamento.
- [ ] Investigar limites de IDs, sprites, atributos, classes e persistência antes de criar itens; não presumir bosses, efeitos ou animações inexistentes.
- [ ] Exibir a arma e suas condições entre as recompensas possíveis do boss no Códex; avaliar integração posterior com Boss Rush e conquistas.
- [ ] Priorizar o modo expandido, preservando o clássico e saves antigos. Testar drop, inventário, equipamento e salvar/carregar nos dois hosts.

### Prioridade específica da web — progresso persistente no iPhone

- [ ] Investigar salvamento e carregamento persistentes, usando IndexedDB ou solução apropriada, incluindo progresso do personagem e Códex.
- [ ] Oferecer exportação/importação de saves e verificar recuperação após fechar/reabrir o Safari, com testes em iPhone físico.
- [ ] Tratar como trabalho próprio da web, sem aguardar expansões de conteúdo e sem versão fixa atribuída. **Hoje o progresso publicado continua temporário; lembrar preferências não equivale a salvar a partida.**

### Sequência adicional proposta — 0.13 a 0.30

A tabela organiza as sugestões do relatório; **a viabilidade e as versões ainda não estão confirmadas**. Não cancela metas anteriores nem aumenta o escopo da 0.8 em andamento.

| Versão sugerida | Atualização futura |
| --- | --- |
| 0.13 | Conquistas e troféus, incluindo possíveis objetivos relacionados a bosses |
| 0.14 | Painel de diagnóstico F12 e testes avançados do host/Unicorn |
| 0.15 | Ficha de atributos secundários verificáveis |
| 0.16 | Desmontagem em massa e Auto-Salvage com proteção de itens |
| 0.17 | Loadouts de equipamentos e habilidades |
| 0.18 | Segredos e easter eggs |
| 0.19 | Mercador viajante |
| 0.20 | Eventos de emboscada |
| 0.21 | Altares de sacrifício e oferendas |
| 0.22 | Encantamentos elementais e efeitos compatíveis |
| 0.23 | Modificadores de masmorra |
| 0.24 | Defesa de ponto contra ondas de inimigos |
| 0.25 | Modo Hardcore opcional e isolado |
| 0.26 | Sistema de Legado / talentos de conta |
| 0.27 | Paladino e investigação do Berserker |
| 0.28 | Mago |
| 0.29 | Monge |
| 0.30 | Consolidação da preparação para o cooperativo |
| 1.0 | Cooperativo pela internet e LAN, já previsto no planejamento principal |

A pesquisa do cooperativo deve começar **antes da 0.30**. A linha 0.27 é uma proposta posterior de entrega de classes; não elimina a possibilidade de uma prévia condicional do Paladino na 0.10 já registrada acima. Berserker exige distinguir sua evolução original de uma nova classe-base independente.

**Após a 1.0, provisoriamente:** Necromante, Druida, Engenheiro, Granadeiro e possível Berserker como classe-base independente. Permanecem as definições do autor: Necromante com foice e Granadeiro com granadas. Não há nova classe entregue ou mudança de saves por este registro.

## Novas ideias propostas pelo autor — avaliação técnica pendente

> **Banco de propostas (23/10/2026).** As ideias abaixo foram registradas pelo autor, mas **ainda não passaram pela análise técnica e pelo planejamento de versões do Codex**. Não são funcionalidades implementadas, não integram a contagem das melhorias já aprovadas e **não possuem versão de entrega confirmada**; a sequência adicional acima é apenas uma proposta. Exemplos de atributos, porcentagens e recompensas são ilustrativos, não decisões de balanceamento. Antes de marcar qualquer item como viável, verificar recursos originais, possíveis conflitos com o roadmap, compatibilidade com o modo clássico e saves, custo técnico e testes necessários.

### Engenharia, diagnóstico e desempenho

- [ ] **Testes automatizados de falhas do host (Unicorn):** Expandir os testes Python (unittest) para simular erros de alocação, ponteiros corrompidos e estouro de pilha na camada ARM, com tratamento seguro das falhas.
- [ ] **Cache LRU para miniaturas do Códex (zlib/sprites):** Investigar cache em memória com reciclagem dos sprites descomprimidos para reduzir recarregamentos e picos de RAM durante a navegação.
- [ ] **Desacoplamento de entrada (IInputProvider):** Estudar uma interface entre comandos SDL2 e a emulação Unicorn, pensando em remapeamento, testes e eventual cooperativo, sem mudar o comportamento atual.
- [ ] **Painel de diagnóstico no jogo (F12 Developer Overlay):** Propor exibição opcional de FPS real, memória, custo da emulação e estados relevantes para depuração, condicionada à disponibilidade e precisão das medições.

**Ideia já coberta pelo planejamento existente:** a proposta de *abstração dos saves e escrita atômica* do arquivo de ideias se relaciona ao **item 03 — Gravação segura de saves**, já implementado; veja os limites nas Releases. Não foi cadastrada como uma nova melhoria nem como garantia de proteção comprovada contra falta de energia.

### Segredos e descobertas

- [ ] **Sistema de segredos e easter eggs:** Recompensar ações pouco óbvias, como acender tochas em uma ordem, descobrir paredes falsas, vencer chefes com restrições ou usar habilidades em locais aparentemente sem saída. Possíveis recompensas: títulos secretos, aparências, equipamentos de homenagem e salas de bônus. Confirmar se a estrutura dos mapas e eventos permite essas interações.

### Inventário e qualidade de vida

- [ ] **Desmontagem em massa e Auto-Salvage:** Selecionar vários equipamentos para desmontar em materiais de craft; estudar também regras opcionais de desmontagem automática ao coletar, com proteções contra perda de itens valiosos.
- [ ] **Ficha avançada de atributos secundários:** Mostrar, quando realmente existirem ou puderem ser calculados corretamente, roubo de vida, evasão, dano crítico, espinhos/retaliação e velocidade de movimento.
- [ ] **Loadouts rápidos locais:** Configurar perfis de equipamento e habilidades, com troca em cidade/área segura (por exemplo, build de dano e build de sobrevivência); verificar restrições de inventário, pontos e persistência.

### Progressão, combate e eventos

- [ ] **Conquistas locais (Achievements):** Criar painel de troféus integrado ao Códex ou menu principal, com metas e possíveis recompensas.
- [ ] **Modificadores de masmorra (Mutators / desafios diários):** Estudar portais ou masmorras especiais com regras variáveis para o pós-jogo, como aumento simultâneo de XP e dano inimigo ou alterações em recargas e poções. Os valores citados são exemplos.
- [ ] **Altares de sacrifício e oferendas:** Trocar ouro ou parte temporária da vida máxima por bônus por tempo limitado ou recompensas especiais, avaliando riscos e equilíbrio.
- [ ] **Encantamentos elementais visuais (Elemental Imbues):** Investigar armas com fogo, gelo, veneno ou eletricidade, consumíveis/habilidades, alteração visual dos golpes e possíveis efeitos de status. Exige verificar animações e regras de combate originais.
- [ ] **Mini-eventos de emboscada:** Encontrar hordas surpresa ou situações em que portas se fecham, com desafio curto de sobrevivência e recompensa; duração e condições ainda não definidas.
- [ ] **Mercador viajante aleatório (Smuggler):** NPC ocasional com estoque temporário de itens raros, materiais incomuns ou consumíveis próprios, sujeito às limitações de NPC, mapas e economia do jogo.
- [ ] **Legado / talentos de conta (Account Prestige):** Progresso compartilhado capaz de conceder pequenas vantagens a novos personagens, como bônus iniciais de XP e ouro; investigar separação entre personagens, saves e modos clássico/expandido.
- [ ] **Defesa de ponto (Horda PvE):** Missões de proteger um NPC aliado ou artefato contra ondas de monstros, verificando se IA, spawn e colisões suportam o formato.
- [ ] **Modo Hardcore opcional (Ironman):** Criar personagem com regra de morte permanente, apagando ou bloqueando o progresso daquela tentativa, mediante desenho de salvamento isolado e proteções explícitas contra perda acidental.

### Sugestões de armas para as classes já planejadas

O planejamento existente das **oito novas classes (item 44)** e de **armas exclusivas (item 45)** permanece inalterado. A tabela abaixo registra **sugestões ainda não avaliadas**, não definições aprovadas nem confirmação de compatibilidade com animações, dados ARM ou equipamentos.

| Classe | Sugestão nova de arma | Situação |
| --- | --- | --- |
| Mago | Cajado mágico | Sugestão não avaliada |
| Berserker | Machado de duas mãos | Sugestão não avaliada |
| Paladino | Maça e escudo | Sugestão não avaliada |
| Monge | Manoplas de combate | Sugestão não avaliada |
| Druida | Cetro natural / totem | Sugestão não avaliada |
| Engenheiro | Chave mecânica de combate | Sugestão não avaliada |

As associações **Necromante → foice** e **Granadeiro → granadas** já estavam definidas pelo autor no planejamento anterior e não são novas propostas.

**Próxima etapa:** solicitar ao Codex uma triagem destas ideias, separando viabilidade técnica, dependências, riscos, sobreposições e esforço estimado. **Só depois** decidir quais entram no roadmap e em que versão; o registro aqui não autoriza implementação nem publicação de release.

## Publicação

Pacotes oficiais e histórico detalhado ficam em [Releases](https://github.com/sugyua/destinia/releases). A `main` contém os lotes de desenvolvimento indicados acima; a versão0.9 tem o escopo das notas de release e os complementos foram transferidos à 0.9.1 por decisão do autor. O autor confirmou testes da base no PC/iPhone; isso não comprova campanha completa ou validação manual de cada mudança nova.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
