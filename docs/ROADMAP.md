# Atualizações futuras do Destinia para Windows

> **O jogo está em versão Beta e pode apresentar diversos bugs. Faça backup dos saves antes de atualizar.**

Seleção concluída e publicação do planejamento autorizada pelo autor em **21/10/2026**: **37 melhorias aprovadas**. Este documento registra o planejamento; os itens 01, 02 e 05 já foram implementados na 0.5, com publicação aprovada pelo autor. Os itens 07, 10, 11 e 12 foram implementados na candidata 0.6, ainda sem publicação; o pacote Windows está em verificação. Os demais continuam futuros. A release atual é a **0.5 Beta**; a confirmação visual do zoom, da UI e dos cliques no Windows ainda está pendente.

## Ordem e critérios

A avaliação inicial considera o host C++17/SDL2/Unicorn, as rotinas de entrada/áudio e a ponte para a biblioteca ARM original. Recursos de diagnóstico e preferências podem começar no host; inventário, missões, UI nativa e progressão exigem compreender dados e rotinas do jogo. Esta é uma estimativa de dificuldade, não uma prova de viabilidade.

As versões abaixo são **metas provisórias**, sem calendário garantido. Uma etapa pode ser dividida ou reordenada conforme os resultados. O log TXT está implementado na **0.5**, junto de backups, correções e configuração global de saves solicitada posteriormente. A aprovação das ideias autoriza o planejamento; cada nova release precisa de revisão das verificações, limitações e aprovação do autor antes de ser publicada.

| Meta | Foco | Dificuldade preliminar |
| --- | --- | --- |
| 0.5 | Diagnóstico e proteção inicial | Baixa a média |
| 0.6 | Configurações, comandos e distribuição | Média |
| 0.7 | Interface e acessibilidade | Alta; depende de investigação ARM/renderização |
| 0.8 | Saves, compatibilidade e loja offline | Alta; depende de investigação de saves e callbacks ARM |
| 0.9 | Inventário, coleta e assistência opcional | Alta; depende de dados, UI e persistência nativos |
| 0.10 | Progressão expandida e novos equipamentos | Muito alta; exige prova de viabilidade e balanceamento |
| 0.11 | Missões, armazenamento e desafios configuráveis | Muito alta; depende de missões, dados e saves |
| 0.12 | Pós-jogo e masmorras | Muito alta; etapa exploratória |

## Regras de implementação

- Preservar os recursos existentes e verificar primeiro o que o jogo original já oferece. As melhorias seguintes devem seguir esse princípio; confira o escopo da candidata atual em [RELEASE_0.6.md](RELEASE_0.6.md).
- Priorizar correções isoladas e verificáveis. O item 05 reúne várias correções: cada caminho precisa de seu próprio diagnóstico e teste.
- Começar a investigação da escrita de saves e dos formatos nativos nas primeiras etapas; a meta 0.8 é a conclusão da base necessária às mudanças permanentes posteriores.
- Separar os modos clássico e expandido e identificar seus saves antes de mudar progressão/conteúdo. Não converter saves silenciosamente nem testar sobre o único save do jogador.
- Não misturar backups de arquivos antes de iniciar/atualizar (02) com gravação segura durante a partida (03): são proteções complementares.
- Manter coleta automática, assistência por poções, pausa e demais escolhas opcionais configuráveis. Balancear mudanças de economia e dificuldade no modo apropriado.
- Manter os programas, ferramentas e bibliotecas externos fora do ZIP da release, conforme pedido do autor; documentar requisitos em [Dependências](../README.md#dependências). EXEs prontos não eliminam as dependências de execução.
- Distinguir testes automatizados do host de confirmação visual e gameplay no Windows. Não prometer 60 FPS apenas removendo limites: o ritmo da lógica original precisa ser respeitado.

## Etapas aprovadas

### 0.5 — Diagnóstico e proteção inicial

Publicação aprovada pelo autor em 21/10/2026 após verificações locais; confirmação manual no Windows pendente. Configuração global de pasta de save incluída por solicitação adicional do autor. As caixas marcadas abaixo indicam implementação na 0.5.

**Dependências:** Trabalho concentrado no host; não exige expandir dados do jogo.

- [x] **01. Log TXT do jogo:** Registrar eventos importantes, mapas, saves, erros e desempenho em TXT na pasta `relatorios`, com limite de tamanho e retenção.
- [x] **02. Backups rotativos de saves:** Guardar cópias datadas antes de iniciar o jogo/atualizar, com restauração orientada e quantidade limitada.
- [x] **05. Erros tratados e testes de estabilidade:** Corrigir operações fread/fwrite de tamanho zero, validar limites ELF e recursos SDL, propagar falhas reais de emulação e testar os caminhos afetados.

**Verificação necessária antes da entrega:** Conferir rotação e limite dos logs, restaurar um backup de teste e exercitar os caminhos de erro, incluindo arquivos ELF inválidos e falhas de recursos SDL.

### Meta 0.6 — Configurações, comandos e distribuição

Preferências e comandos implementados na candidata 0.6; pacote Windows em verificação. A publicação da 0.6 depende de aprovação. Veja [resultados](RELEASE_0.6.md).

**Dependências:** Usar o diagnóstico da 0.5; preferências servem de base aos recursos opcionais seguintes.

- [ ] **06. EXEs Windows prontos nas releases:** Disponibilizar os EXEs dos dois hosts, compilados e verificados no Windows; dependências externas continuam instaladas separadamente, sem serem incluídas no pacote.
- [x] **07. Configurações persistentes:** Salvar preferências de vídeo, janela e comandos em arquivo próprio, com opção de restaurar padrões.
- [x] **10. Remapear teclado e controle:** Escolher teclas e botões por ação e guardar perfis; ampliar o suporte atual de comandos fixos.
- [x] **11. Volumes separados e persistentes:** Controles independentes para música e efeitos, inclusive silenciar ao perder foco se o usuário quiser.
- [x] **12. Pausa ao perder foco ou controle:** Permitir pausa opcional ao perder foco ou desconectar o controle e garantir liberação das direções pressionadas.

**Verificação necessária antes da entrega:** Reabrir e conferir preferências; testar conflitos de teclas, controle desconectado, Alt+Tab, áudio e inicialização dos dois EXEs em Windows com dependências documentadas.

### Meta 0.7 — Interface e acessibilidade

Separar o tamanho da UI do zoom e melhorar a apresentação para teclado, controle e português.

**Dependências:** Depende das configurações da 0.6 e da identificação de como a UI nativa é desenhada e recebe cliques.

- [ ] **08. Escala de UI independente do zoom:** Escolher tamanho da interface e do texto sem mudar a quantidade de cenário visível; alinhar as regiões clicáveis.
- [ ] **09. Ocultar botões virtuais:** Opção de esconder direcional e botões de toque ao jogar por teclado/controle, mantendo os comandos físicos.
- [ ] **19. Atalhos visíveis nas habilidades:** Mostrar a tecla ou botão configurado junto às habilidades e adaptar os símbolos ao controle usado.
- [ ] **24. Português mais completo e legível:** Completar diálogos e descrições ainda em inglês e investigar suporte a acentos/fontes, preservando espaço e quebras de linha.

**Verificação necessária antes da entrega:** Conferir texto, acentos, símbolos dos comandos e regiões clicáveis; testar janela, DPI e tela cheia 3440×1440 com imagem proporcional 2400×1440.

### Meta 0.8 — Saves, compatibilidade e loja offline

Preparar a base para alterar inventário e progressão sem misturar a experiência clássica com a expandida.

**Dependências:** Mapear formatos de dados/saves e o fluxo de escrita; preservar cópias originais antes das mudanças permanentes.

- [ ] **03. Gravação segura de saves:** Investigar gravação temporária, validação e substituição atômica ao concluir, preservando o save anterior se houver falha.
- [ ] **04. Loja offline sem espera nem crédito indevido:** Retornar ao jogo com aviso quando compras reais estiverem indisponíveis, sem espera pendente nem novo crédito indevido; preservar o saldo existente.
- [ ] **36. Modo clássico e expandido:** Separar as alterações de progressão/conteúdo da experiência original, com perfis e saves identificados para evitar conversões silenciosas.

**Verificação necessária antes da entrega:** Interromper uma gravação em cópia descartável e recuperar o save anterior; conferir separação de perfis e retorno da loja sem espera pendente nem alteração indevida de saldo.

### Meta 0.9 — Inventário, coleta e assistência opcional

Reduzir tarefas repetitivas e tornar os equipamentos mais fáceis de entender e proteger.

**Dependências:** Depende da base de saves/perfis da 0.8, das preferências e do mapeamento das rotinas de inventário e combate.

- [ ] **13. Comparador claro de equipamentos:** Exibir diferenças de atributos entre o item selecionado e o equipado, com sinais e cores legíveis; verificar primeiro o comparador original para aprimorá-lo.
- [ ] **14. Organização e filtros do inventário:** Ordenar e filtrar por tipo, rank, nível e preço, sem perder a seleção atual; confirmar o que já existe no jogo.
- [ ] **15. Favoritar e proteger itens:** Marcar equipamentos para evitar venda/desmanche acidentais; pedir confirmação quando apropriado.
- [ ] **16. Filtro de itens no chão:** Escolher quais categorias ou ranks destacar e quais esconder visualmente; não apagar os itens filtrados.
- [ ] **17. Coleta automática opcional:** Coletar ouro e materiais próximos segundo regras configuráveis, preservando a opção de coleta manual.
- [ ] **21. Poções automáticas opcionais:** Usar poções opcionalmente por limites de vida/mana configurados, respeitando estoque e recarga; a assistência deve ser ativada pelo jogador.
- [ ] **22. Nomes e ranks dos drops por tecla:** Exibir nomes e raridades dos itens no chão ao segurar ou alternar uma tecla, com contraste adequado.
- [ ] **29. Redistribuição de pontos acessível offline:** Verificar o reset existente e oferecer uma forma clara de redistribuir atributos/habilidades offline, com custo interno a definir e sem compra real.

**Verificação necessária antes da entrega:** Comparar inventário antes/depois de ordenar, filtrar, vender e reabrir; conferir favoritos, coleta e poções sem perda/duplicação, consumo fora das regras ou mudanças na experiência clássica.

### Meta 0.10 — Progressão expandida e novos equipamentos

Aumentar o nível máximo e acrescentar ranks, itens inéditos, conjuntos, receitas e recompensas offline.

**Dependências:** Depende do mapeamento de XP, limites, tabelas de equipamentos e saves, além do modo expandido da 0.8.

- [ ] **25. Aumentar o nível máximo:** Aumentar o nível máximo após verificar o limite original e revisar XP, pontos, atributos, equipamentos, inimigos, HUD e saves; novo limite ainda a definir.
- [ ] **26. Novos ranks de armas:** Criar graus acima do maior atual, com nomes e progressão a definir, incluindo atributos, requisitos, obtenção e identificação visual.
- [ ] **27. Novos ranks de armaduras e acessórios:** Adicionar ranks de armaduras e acessórios, equilibrando defesa, novas armas e inimigos.
- [ ] **28. Mais receitas de fabricação:** Adicionar receitas e usos para materiais, incluindo obtenção dos novos equipamentos; o jogo já possui fabricação.
- [ ] **30. Ganhar cash jogando:** Definir recompensas offline por objetivos, desafios ou missões, com limites e economia equilibrada; independente do aviso de compra indisponível.
- [ ] **37. Novos itens únicos:** Criar armas, armaduras e acessórios inéditos, com identidade, atributos, efeitos e formas de obtenção próprios, além dos novos ranks.
- [ ] **38. Novos conjuntos de equipamentos:** Adicionar conjuntos com bônus por combinar peças; verificar primeiro o sistema original e definir bônus e número de peças.

**Verificação necessária antes da entrega:** Provar primeiro um avanço além do limite original e um equipamento novo em save descartável; depois conferir XP, atributos, habilidades, requisitos, receitas, bônus de conjunto, economia e persistência.

### Meta 0.11 — Missões, armazenamento e desafios configuráveis

Adicionar baú compartilhado, informação de missões, dificuldades opcionais e contratos offline.

**Dependências:** Baú depende da gravação segura da 0.8; missões e recompensas exigem mapear objetivos e validar a economia expandida.

- [ ] **18. Baú compartilhado entre personagens:** Transferir itens entre personagens pelo armazenamento local, com proteção contra perda/duplicação.
- [ ] **23. Rastreador de missão:** Mostrar objetivo ativo e, quando houver coordenadas conhecidas, marcador de destino; verificar os recursos originais existentes.
- [ ] **32. Dificuldades opcionais:** Perfis de dificuldade com ajustes de inimigos e recompensas; preservar uma experiência equivalente à original.
- [ ] **35. Contratos repetíveis offline:** Objetivos opcionais de caça/coleta para dar propósito ao pós-jogo, com recompensas e repetição controladas.

**Verificação necessária antes da entrega:** Transferir itens entre personagens e reabrir sem perdas/duplicações; validar objetivos, marcadores, recompensas e regras de dificuldade, mantendo o perfil original.

### Meta 0.12 — Pós-jogo e masmorras

Ampliar a duração do jogo com Novo Jogo+, desafio de chefes e masmorras variáveis.

**Dependências:** Depende de campanha, mapas, colisões, geração de inimigos, recompensas e saves já compreendidos; masmorras exigem protótipo próprio.

- [ ] **31. Novo Jogo+:** Reiniciar a campanha mantendo progressão aprovada e aumentando os desafios/recompensas, com regras claras do que é preservado.
- [ ] **33. Desafio de chefes:** Arena ou sequência de chefes já existentes, com regras e recompensas próprias e sem exigir campanha inédita.
- [ ] **34. Masmorras variáveis:** Estudar masmorras geradas ou montadas com salas existentes, se os formatos nativos permitirem; validar colisões, saídas, inimigos e persistência.

**Verificação necessária antes da entrega:** Conferir o que o Novo Jogo+ preserva/reinicia; validar entradas, saídas, colisões, chefes, recompensas e retomada do progresso, inclusive após fechar o jogo.

## Investigação da expansão de conteúdo

O aumento do nível máximo (25), os novos ranks (26/27), receitas (28), recompensas de cash jogando (30), itens inéditos (37) e conjuntos (38) fazem parte da meta **0.10**. Novo Jogo+, desafios e masmorras seguem nas etapas de pós-jogo. Os números são uma organização inicial; provas de viabilidade podem ser antecipadas enquanto se concluem melhorias independentes do host.

Antes de alterar esses sistemas, é necessário:

1. Confirmar o limite original e mapear as tabelas de XP, atributos, pontos, requisitos, inimigos, equipamentos e os respectivos campos do save. O helper atual do HUD só trata níveis até 99; alterar esse teste isoladamente não aumenta o limite da lógica nativa.
2. Verificar os formatos dos dados, compressão, limites de índices/IDs e espaço para novos registros. As observações anteriores de ZCP/zlib precisam ser confirmadas em cada formato antes de criar ferramentas de edição.
3. Provar, em perfil expandido descartável, um avanço além do limite original e a criação/obtenção de um item novo, com salvamento e carregamento corretos.
4. Definir o novo limite, nomes dos ranks, requisitos, curvas de progressão, receitas, obtenção, custos, bônus de conjuntos e recompensas. Reaproveitar os sistemas originais quando existirem e preservar o modo clássico.
5. Testar progressão e economia ao longo do jogo, efeitos/descrições na UI e persistência. Cash obtido por objetivos offline é um recurso separado do retorno seguro da loja: tentar uma compra real indisponível não deve gerar crédito indevido.

A geração de masmorras (34) depende de provar que os formatos e rotinas nativas permitem salas variáveis com colisões, saídas e persistência corretas. Nenhuma campanha nova, formato de dados ou compatibilidade de patch foi considerada pronta nesta etapa.

## Referências da pesquisa

Fontes primárias consultadas em **21/10/2026**. São referências de experiência, manutenção e conteúdo; a maioria usa engines abertas, enquanto este port executa a lógica ARM original. As ideias precisam de uma implementação e verificação próprias no Destinia.

- **[Destinia Vita — README e changelog](https://github.com/withLogic/destinia-vita)**: Wrapper da biblioteca Android ARMv6. README menciona APK 2.1.0, configuração de resolução e ritmo de quadros; changelog 0.1.1 documenta correção do limitador e do prompt de compra, substituindo-o por crédito local. Isso não comprova compatibilidade com a biblioteca deste projeto nem valida usar crédito automático no nosso port.
- **[DevilutionX — Features](https://devilutionx.com/features)**: Nas seções lançadas: baú, coleta automática opcional de ouro, atalhos, remapeamento de controle, melhorias de informação e animações atualizadas na renderização. A ordenação de inventário aparece na seção 1.6 em desenvolvimento; não tratá-la como recurso já lançado a partir dessa página.
- **[GemRB — Innovations](https://gemrb.org/Innovations.html)**: Documenta interface de opções, suporte a resoluções, fontes, atalhos, organização de listas e ajustes configuráveis. É uma reimplementação de engine, diferente do nosso carregador ARM; serve como referência de experiência e organização.
- **[OpenMW — lançamento 0.50](https://openmw.org/2025/openmw-0-50-0-released/)**: Documenta UI e navegação para gamepad, transferência rápida de itens e melhorias na configuração. Não indica que a mesma solução possa ser copiada diretamente para Destinia.
- **[OpenMW — Saves Settings](https://openmw.readthedocs.io/en/stable/reference/modding/settings/saves.html)**: Documenta autosave e múltiplos slots rotativos para salvamentos rápidos/automáticos.
- **[Flare 1.15 — anúncio oficial](https://flarerpg.org/2026/05/03/flare-1-15/)**: Publicação documenta gráficos para 1080p/1440p, ordenação opcional de inventário/baú, auto-loot configurável, mapas e itens procedurais, masmorra adicional e revisão de habilidades. Novas mecânicas de Destinia abaixo são propostas próprias inspiradas nisso, não funcionalidades prontas para importar.

O port Vita documenta outra versão do APK; endereços e patches não podem ser presumidos compatíveis. Sua solução de crédito local para a loja não é a proposta aprovada aqui: o item 04 deve preservar o saldo e retornar ao jogo sem novo crédito indevido.
