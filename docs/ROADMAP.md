# Roadmap do Destinia para Windows

> **O jogo está em versão Beta e pode apresentar diversos bugs. Faça backup dos saves antes de atualizar.**

**Estado atual em30/10/2026:** fontes e candidata0.17.2 (confirmação e testes seguros de gravação Web);0.17.1 publicada e instalada. CI e estado final dos pacotes0.17.2 devem ser conferidos na [release](https://github.com/sugyua/destinia/releases/tag/v0.17.2-beta). Interface original0.17 preservada. WebVPNPC0.17.2 verificada; Site privadoCodex mantém0.13.0 conhecido por indisponibilidade da ferramenta de publicação. Campanha/Safari físico não executados nesta entrega.

Próximas frentes da Issue #5: contratos/recuperação de saves, hooks opcionais/Zoom restantes, profiling comparável, permissões/dependências, divergências intencionais dos hosts, acessibilidade Web e fuzz. A [triagem atual](TRIAGEM_GITHUB_2026-10-30.md) separa correções já entregues das propostas; não reimplementar atalhos, CI Web ou versões anteriores.

## Registro histórico da transição0.12.1–0.16.0

Naquele registro, a release publicada mais recente era 0.12.1 Beta. A branch PR #6 reune 0.13.0–0.16.0 em quatro commits ordenados. A Web privada ja tem arena e refino 0.13.0; F12, Ctrl+F3 e desmontagem em lote pertencem aos hosts PC. Permanecem pendentes contratos e recuperacao de saves, tratamento de erros de inicializacao/hooks Unicorn, profiling comparavel, revisao de licencas/dependencias, divergencia entre hosts, acessibilidade/orientacao Web e testes property/fuzz. A validacao fisica continua necessaria alem dos testes automatizados.

Em 27/10/2026, a release atual era 0.12.1 Beta. Neste branch de continuidade, o workflow de testes Web e a correção de descrições de drops que excediam o limite visual de 55 caracteres estão em revisão local; ainda não enviados nem publicados. Permanecem pendentes os contratos e falhas de recuperação de saves, tratamento de erros de inicialização/hooks Unicorn, profiling comparável, revisão de licenças/dependências, divergência entre hosts, acessibilidade/orientação Web e testes property/fuzz. A frente 0.13.0 segue separada. Testes automatizados não encerram validação física.

## Histórico de planejamento

As metas e contagens abaixo são registros históricos de 21/10; não são uma contagem atual de entregas.

Planejamento autorizado pelo autor em **21/10/2026**: **44 melhorias aprovadas**, das quais **14 concluídas** na 0.5–0.8 e **30 ainda não concluídas**, incluindo o Códex entregue parcialmente, da 0.8 em diante. A publicação da **0.6 Beta** foi aprovada pelo autor. Testes automáticos passaram; campanha e inspeção manual no Windows continuam pendentes.

## Melhorias concluídas com sucesso

- [x] **01. Log TXT do jogo** — entregue na 0.5, com rotação e retenção.
- [x] **02. Backups rotativos de saves** — entregue na 0.5, com cópia verificada e retenção.
- [x] **05. Erros tratados e testes de estabilidade** — entregue na 0.5, com regressões na 0.6.
- [x] **06. EXEs Windows nas releases** — entregue na 0.6, com os dois hosts e checksum, sem dependências externas no ZIP.
- [x] **07. Configurações persistentes** — entregue na 0.6, com restauração de padrões preservando save_dir.
- [x] **10. Remapeamento de teclado e controle** — entregue na 0.6, com perfis e validação de conflitos.
- [x] **11. Volumes separados e persistentes** — entregue na 0.6, com silêncio sem foco opcional.
- [x] **12. Pausa e liberação de comandos** — entregue na 0.6, manual/opcional por foco ou controle.

Implementação e verificações automatizadas concluídas com sucesso. A pasta global de save compartilhado também foi entregue na 0.5 e preservada na 0.6. Consulte o [changelog](../CHANGELOG.md) e as [verificações da 0.6](RELEASE_0.6.md) para o escopo e as limitações.

## Entrega parcial da 0.7 — Códex de monstros

O bestiário inicial do **item 42** está implementado nos dois hosts: livro **CODEX** no menu original do jogo, junto da barra de Craft e Pets. A correção do acesso está na branch `main`; F8 funciona como atalho nesse menu quando a tecla não estiver remapeada. Os ZIPs originais da tag 0.7 preservam o acesso anterior. Mostra as espécies descobertas, os estados **VISTO/DERROTADO** e **quantos monstros de cada espécie foram mortos**, com paginação. A partida pausa durante a consulta. Nomes de monstros permanecem em inglês nesta primeira etapa.

Os registros ficam em **codex-v1.txt**, na pasta compartilhada de saves, separados pelo slot de personagem e gravados por substituição de arquivo temporário. Não altera o formato dos saves ARM nem recupera mortes antigas. A contagem começa na 0.7; o slot reutilizado após exclusão pelo jogo tem seus registros reiniciados. Trocar apenas de pasta de instalação preserva o Códex quando a pasta de save é a mesma.

Testes automatizados cobrem registro, morte única, reaparecimento, slots, persistência e callbacks originais ARM com estruturas controladas, além de desenho dos painéis. Campanha completa e inspeção visual no Windows continuam pendentes. Veja [como usar o Códex](CODEX.md) e [verificações da 0.7](RELEASE_0.7.md).

**O item 42 completo continua desmarcado:** armas, armaduras e integração com drops exclusivos permanecem na 0.11. Os outros quatro ajustes de interface antes previstos para a 0.7 foram mantidos como pendências da 0.8.

## Atualizações futuras — 0.9 em diante

## Ordem e critérios

A avaliação inicial considera o host C++17/SDL2/Unicorn, as rotinas de entrada/áudio e a ponte para a biblioteca ARM original. Recursos de diagnóstico e preferências podem começar no host; inventário, missões, UI nativa e progressão exigem compreender dados e rotinas do jogo. Esta é uma estimativa de dificuldade, não uma prova de viabilidade.

As versões abaixo são **metas provisórias**, sem calendário garantido. Uma etapa pode ser dividida ou reordenada conforme os resultados. O log TXT está implementado na **0.5**, junto de backups, correções e configuração global de saves solicitada posteriormente. A aprovação das ideias autoriza o planejamento; cada nova release precisa de revisão das verificações, limitações e aprovação do autor antes de ser publicada.

| Meta | Foco | Dificuldade preliminar |
| --- | --- | --- |
| 0.8 | Interface pendente, acessibilidade, saves e loja offline | Alta; depende de renderização, saves e callbacks ARM |
| 0.9 | Inventário, coleta e assistência opcional | Alta; depende de dados, UI e persistência nativos |
| 0.10 | Progressão expandida, equipamentos, títulos e drops exclusivos | Muito alta; exige mapear progressão, IDs/condições, tabelas de drops e balanceamento |
| 0.11 | Missões, armazenamento, desafios, pets e Códex | Muito alta; depende de dados, animações/comportamento, eventos de descoberta e saves |
| 0.12 | Pós-jogo e masmorras | Muito alta; etapa exploratória |
| 1.0 | Cooperativo pela internet e LAN; preparação em etapas após a 0.12 | Muito alta; depende de investigação de rede, sincronização e saves |
| A definir | Oito novas classes (11 no total) e armas exclusivas; Necromante com foice, Granadeiro com granadas | Muito alta; depende de IDs/limites de classes e armas, habilidades, gráficos e saves |

## Regras de implementação

- Preservar os recursos existentes e verificar primeiro o que o jogo original já oferece. As melhorias seguintes devem seguir esse princípio; confira o escopo da versão atual em [RELEASE_0.6.md](RELEASE_0.6.md).
- Priorizar correções isoladas e verificáveis. O item 05 reúne várias correções: cada caminho precisa de seu próprio diagnóstico e teste.
- Começar a investigação da escrita de saves e dos formatos nativos nas primeiras etapas; a meta 0.8 é a conclusão da base necessária às mudanças permanentes posteriores.
- Separar os modos clássico e expandido e identificar seus saves antes de mudar progressão/conteúdo. Não converter saves silenciosamente nem testar sobre o único save do jogador.
- Não misturar backups de arquivos antes de iniciar/atualizar (02) com gravação segura durante a partida (03): são proteções complementares.
- Manter coleta automática, assistência por poções, pausa e demais escolhas opcionais configuráveis. Balancear mudanças de economia e dificuldade no modo apropriado.
- Manter os programas, ferramentas e bibliotecas externos fora do ZIP da release, conforme pedido do autor; documentar requisitos em [Dependências](../README.md#dependências). EXEs prontos não eliminam as dependências de execução.
- Distinguir testes automatizados do host de confirmação visual e gameplay no Windows. Não prometer 60 FPS apenas removendo limites: o ritmo da lógica original precisa ser respeitado.

## Etapas futuras aprovadas

A 0.8 entregou os itens03,04,08,09,19 e36; a revisão integral da tradução24 permanece pendente. [Escopo e limites](RELEASE_0.8.md).

### Meta 0.9.1 — complementos de inventário e assistência

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

### Meta 0.10 — Progressão expandida, equipamentos e títulos

Aumentar o nível máximo e acrescentar ranks, itens inéditos, conjuntos, receitas, recompensas offline, novos títulos e drops exclusivos por monstro.

**Dependências:** Depende do mapeamento de XP, limites, tabelas de equipamentos e saves, além do modo expandido da 0.8. Novos títulos também exigem mapear IDs, condições de aquisição, atributos/bônus e seleção do título ativo; ampliar apenas a tabela de nomes não basta. Drops exclusivos exigem mapear a relação monstro/equipamento, tabelas ou rotinas de sorteio, chances, requisitos e demais fontes de obtenção.

- [ ] **25. Aumentar o nível máximo:** Aumentar o nível máximo após verificar o limite original e revisar XP, pontos, atributos, equipamentos, inimigos, HUD e saves; novo limite ainda a definir.
- [ ] **26. Novos ranks de armas:** Criar graus acima do maior atual, com nomes e progressão a definir, incluindo atributos, requisitos, obtenção e identificação visual.
- [ ] **27. Novos ranks de armaduras e acessórios:** Adicionar ranks de armaduras e acessórios, equilibrando defesa, novas armas e inimigos.
- [ ] **28. Mais receitas de fabricação:** Adicionar receitas e usos para materiais, incluindo obtenção dos novos equipamentos; o jogo já possui fabricação.
- [ ] **30. Ganhar cash jogando:** Definir recompensas offline por objetivos, desafios ou missões, com limites e economia equilibrada; independente do aviso de compra indisponível.
- [ ] **37. Novos itens únicos:** Criar armas, armaduras e acessórios inéditos, com identidade, atributos, efeitos e formas de obtenção próprios, além dos novos ranks.
- [ ] **38. Novos conjuntos de equipamentos:** Adicionar conjuntos com bônus por combinar peças; verificar primeiro o sistema original e definir bônus e número de peças.
- [ ] **40. Novos títulos:** Ampliar o sistema original com títulos conquistados por objetivos do jogo, com condições, nomes e eventuais bônus a definir; validar aquisição, seleção, exibição e persistência no modo expandido.
- [ ] **41. Drops de armas e armaduras exclusivos de monstros específicos:** Associar equipamentos exclusivos a espécies ou chefes determinados, com chances e requisitos a definir; validar que esses itens só caiam dos monstros previstos, sem remover os drops existentes do modo clássico.

**Verificação necessária antes da entrega:** Provar primeiro um avanço além do limite original e um equipamento novo em save descartável; depois conferir XP, atributos, habilidades, requisitos, receitas, bônus de conjunto, economia e persistência. Para títulos, conquistar um título realmente novo, selecioná-lo, verificar seus efeitos e salvar/reabrir sem perder os títulos anteriores. Para drops exclusivos, testar monstros permitidos e não permitidos, conferir as regras e chances, preservar os drops clássicos e validar a persistência dos equipamentos obtidos.

### Meta 0.11 — Missões, armazenamento, desafios, pets e Códex

Adicionar baú compartilhado, informação de missões, dificuldades opcionais, contratos offline, novos pets e ampliar o Códex com equipamentos.

**Dependências:** Baú depende da gravação segura da 0.8; missões e recompensas exigem mapear objetivos e validar a economia expandida. Novos pets também exigem mapear IDs, tabelas de habilidades/evolução, animações, comportamento e campos do save. A meta 0.10 fornece a base de expansão e balanceamento. A fase de equipamentos do Códex exige identificar armas/armaduras por IDs estáveis e observar encontro e obtenção, preservando as descobertas e os contadores de derrotas do bestiário cuja fase inicial foi antecipada para a 0.7. A ampliação continua dependendo de inventário e persistência compatíveis.

- [ ] **18. Baú compartilhado entre personagens:** Transferir itens entre personagens pelo armazenamento local, com proteção contra perda/duplicação.
- [ ] **23. Rastreador de missão:** Mostrar objetivo ativo e, quando houver coordenadas conhecidas, marcador de destino; verificar os recursos originais existentes.
- [ ] **32. Dificuldades opcionais:** Perfis de dificuldade com ajustes de inimigos e recompensas; preservar uma experiência equivalente à original.
- [ ] **35. Contratos repetíveis offline:** Objetivos opcionais de caça/coleta para dar propósito ao pós-jogo, com recompensas e repetição controladas.
- [ ] **39. Novos pets:** Acrescentar mascotes ao sistema original, com aparência, animações, atributos, habilidades, evolução e obtenção offline a definir; validar IDs, comportamento e salvamento no modo expandido, preservando os pets existentes.
- [ ] **42. Códex de equipamentos e monstros:** Entrega em fases: bestiário com monstros vistos/derrotados e contador por espécie entregue na 0.7 (fase parcial); armas e armaduras encontradas/obtidas e integração com drops exclusivos ficam na 0.11. Preservar descobertas por personagem entre sessões e ocultar conteúdos ainda não descobertos; marcar o item completo somente após entregar as duas fases.

**Verificação necessária antes da entrega:** Transferir itens entre personagens e reabrir sem perdas/duplicações; validar objetivos, marcadores, recompensas e regras de dificuldade, mantendo o perfil original. Para pets, obter um mascote realmente novo e validar movimento, animação, habilidades, evolução quando aplicável, troca/descarte e salvar/reabrir sem alterar os mascotes existentes. Para o Códex, conferir separadamente encontro/obtenção de equipamento e encontro/derrota de monstro, salvar/reabrir, manter personagens separados e não atribuir descobertas não observadas.

### Meta 0.12 — Pós-jogo e masmorras

Ampliar a duração do jogo com Novo Jogo+, desafio de chefes e masmorras variáveis.

**Dependências:** Depende de campanha, mapas, colisões, geração de inimigos, recompensas e saves já compreendidos; masmorras exigem protótipo próprio.

- [ ] **31. Novo Jogo+:** Reiniciar a campanha mantendo progressão aprovada e aumentando os desafios/recompensas, com regras claras do que é preservado.
- [ ] **33. Desafio de chefes:** Arena ou sequência de chefes já existentes, com regras e recompensas próprias e sem exigir campanha inédita.
- [ ] **34. Masmorras variáveis:** Estudar masmorras geradas ou montadas com salas existentes, se os formatos nativos permitirem; validar colisões, saídas, inimigos e persistência.

**Verificação necessária antes da entrega:** Conferir o que o Novo Jogo+ preserva/reinicia; validar entradas, saídas, colisões, chefes, recompensas e retomada do progresso, inclusive após fechar o jogo.

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

**Análise e antecipação do primeiro teste:** Berserker já tem base original como evolução do Guerreiro. Recomenda-se um ensaio com personagem e pasta de save separados durante a preparação da **0.8**, antes de concluir toda a versão; isso não conta como uma quarta classe-base implementada. A primeira classe adicional recomendada é **Paladino**, com investigação junto da 0.8 e meta condicional de uma prévia na **0.10**, após provar identidade/tabelas/persistência e a base de inventário da 0.9. Engenheiro também merece investigação por existir `Summon Machine Gun` nos dados originais. As demais entregas continuam sem versão definida. Evidências, limites e critérios em [investigação de classes](INVESTIGACAO_CLASSES.md).

## Planejamento — drops exclusivos e Códex

Adicionados a pedido do autor em **21/10/2026**, ainda sem implementação:

- **41. Drops exclusivos — meta 0.10:** armas e armaduras que caiam apenas de monstros ou chefes específicos. Definir associações, chances e requisitos depois de mapear o sistema de drops original e os IDs dos equipamentos. Pertence ao modo expandido; preservar o modo clássico e não confundir equipamento novo com apenas alterar a aparência de um existente.
- **42. Códex — fase 1 implementada na 0.7; fase 2 na 0.11:** bestiário de monstros vistos/derrotados com contagem por espécie implementado; ainda falta ampliar com armas e armaduras encontradas/obtidas. Manter descobertas por personagem após salvar e reabrir e ocultar conteúdos ainda não descobertos. A integração com os drops exclusivos depende de validar as associações da 0.10. A primeira fase tem testes automatizados, mas gameplay completo e inspeção manual continuam pendentes; a segunda ainda é planejamento.

**Dependências e critérios da expansão completa:** a base de saves/perfis da 0.8, o mapeamento de inventário da 0.9 e a identificação dos eventos e IDs nativos. O bestiário inicial da 0.7 usa registro auxiliar por slot junto ao save compartilhado, sem editar o formato do save nativo; a expansão deve preservar esse registro e validar migrações de perfil. Para drops, provar a exclusividade em monstros permitidos e não permitidos e revisar outras fontes de obtenção. Para o Códex, definir em qual evento um equipamento ou monstro passa a contar como encontrado, observar esse evento e testar a gravação separada dos estados de descoberta, obtenção e derrota. Em saves antigos, aproveitar somente informações comprováveis e começar a observar o restante; não reconstruir como fato um histórico de monstros derrotados ou equipamentos vendidos que não esteja disponível. Não substituir, zerar ou converter silenciosamente o save original.

Estas metas são estimativas de planejamento, sem data garantida. Não foi comprovado nesta atualização de documentação que os formatos e rotinas nativos permitem os dois recursos; a investigação e os protótipos descartáveis devem anteceder qualquer entrega.

## Investigação inicial — novos pets e títulos

Adicionados ao planejamento a pedido do autor em **21/10/2026**: **39. Novos pets** (meta 0.11) e **40. Novos títulos** (meta 0.10). São conteúdo futuro, ainda não implementado nem validado em gameplay.

A inspeção somente de leitura confirmou sistemas originais de títulos e pets na biblioteca ARM e nos assets: `nick_en.tdt`, `pet_skill_en.tdt`, arquivos `pet*.agd`/`pet*.wmb` e rotinas de aquisição, seleção, movimento e evolução. Isso permite investigar uma ampliação dos sistemas existentes; não comprova que basta acrescentar arquivos ou nomes. Detalhes e evidências em [INVESTIGACAO_PETS_TITULOS.md](INVESTIGACAO_PETS_TITULOS.md).

**Quando será possível:** começar o mapeamento antes, mas entregar títulos novos na meta **0.10**, após a base de saves/perfis da 0.8 e um protótipo de aquisição/seleção/persistência; entregar pets novos na meta **0.11**, após validar também recursos visuais, comportamento, habilidades e evolução. Essas versões são estimativas condicionais, sem data garantida; podem mudar se os limites nativos exigirem adaptações maiores. Manter a meta 0.7 focada em interface.

## Investigação da expansão de conteúdo

O aumento do nível máximo (25), os novos ranks (26/27), receitas (28), recompensas de cash jogando (30), itens inéditos (37), conjuntos (38), novos títulos (40) e drops exclusivos por monstro (41) fazem parte da meta **0.10**. Novos pets (39) e a fase de equipamentos do Códex (42) ficam na meta **0.11**; o bestiário inicial do item 42 foi implementado na **0.7**, com validação manual de campanha pendente. Novo Jogo+, desafios e masmorras seguem nas etapas de pós-jogo. Os números são uma organização inicial; provas de viabilidade podem ser antecipadas enquanto se concluem melhorias independentes do host.

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

## Entrega 0.8

Itens03,04,08,09,19 e36 implementados; tradução24 ampliada parcialmente, revisão integral ainda pendente. Consulte [RELEASE_0.8.md](RELEASE_0.8.md). Próxima entrega funcional: metas0.9 do README; propostas posteriores preservadas.

## Entrega 0.9 e continuação0.9.1

Por decisão do autor,0.9 reúne PT-BR padrão, nomes de drops, FC e comparador inicial verificados; demais complementos foram transferidos para0.9.1, incluindo filtros/favoritos, automações e Códex organizado. Reset consumirá pergaminhos originais, vendidos no vendedor de poções por10.000gold cada, ainda não implementado. Veja README e RELEASE_0.9.md.
