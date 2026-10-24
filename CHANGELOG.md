> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

# Histórico de alterações

Histórico em português, da versão mais recente para a mais antiga. Somente alterações efetivas são registradas como novidades.

## Em desenvolvimento — 0.9 Beta

- Português brasileiro passa a ser o padrão nos dois hosts e no navegador; inglês explícito é preservado. CLI aceita `--lang pt|en` sem gravar uma preferência implicitamente.
- README concentra instalação, estado atual e planejamento; histórico detalhado nas Releases.
- Removidos pacotes auxiliares de compilação/testes antigos; scripts ativos permanecem nos fontes e pacotes atuais. Web elimina cópias WIP já integradas.
- Comando remapeável `nomes_drops` (V quando livre) e botão web exibem nomes de drops ativos/visíveis nas cores originais, sem alterar distância de coleta, inventário ou chance de drop.
- Demais funcionalidades de inventário/assistência da0.9 ainda em preparação; não constituem uma release0.9 publicada.

## 0.8 Beta — 24/10/2026

- Escala independente da interface em **75%, 100% ou 125%**, sem alterar o zoom do cenário; hitboxes transformadas junto da UI e contador XP preservado.
- Controles virtuais opcionais por host, com suas áreas de toque desativadas quando ocultos. Teclado, controle e menus continuam disponíveis.
- Atalhos das habilidades exibem as teclas configuradas ou os botões/stick do controle utilizado no Windows.
- Ampliação **parcial** dos textos em português nos dois hosts e na web; fontes dos painéis do port suportam acentos. Textos sem espaço seguro mantêm o original.
- Saves ARM gravados por lote em temporários, com journal e recuperação após interrupção; mantém backups e pasta compartilhada entre atualizações.
- Loja offline cancela pedidos com aviso, libera o estado de rede e preserva o cash existente, sem conceder crédito.
- Perfis **clássico/expandido** com pastas, locks e backups separados. Importação do clássico exige destino novo e ação explícita; não altera a origem.
- Códex da main reúne a grade de retratos frontais, contador corrigido, informações e drops por categoria/raridade das correções posteriores à release original 0.7.
- Protótipo web acompanha escala, controles, idioma e modos; partidas permanecem temporárias na memória.

Verificações, instalação e limites: [RELEASE_0.8.md](docs/RELEASE_0.8.md).

## Organização das releases — 22/10/2026

- Índice do README em ordem de lançamento, da mais recente para a mais antiga, incluindo as correções da 0.7 e horários de Brasília.
- Padronização das versões Beta como pré-lançamentos no GitHub, com preservação das datas, tags, notas e arquivos existentes. Sem alterações no jogo.

## Códex com detalhes nos dois hosts Windows — atualização da main 0.7

- AÇÃO/ataque na grade abre a janela interna de informações, com retrato frontal, contador de mortes e HP máximo/nível do último encontro da sessão.
- Páginas fixas: 1 Drops, 2 Locais de spawn, 3 Variantes. Listas maiores usam SUBIR/DESCER; VOLTAR retorna à mesma seleção. A consulta continua pausando a partida e ocultando os dados de desconhecidos.
- Um item por linha, subpáginas próprias por categoria e raridade, nomes base de equipamentos e receitas e cores originais: branco, verde, azul, roxo e laranja. Raridades respeitam nível e ramo normal/elite/chefe; ovos e consumíveis preservam branco.
- Leitura limitada e validada das tabelas originais, 172 mapas e 59 scripts. Arquivos incompatíveis mantêm a grade e contadores disponíveis. Nenhuma alteração de chances, itens, combate ou formato dos saves/codex-v1.txt.
- Dados de encontro (HP/nível/variante/locais observados) ficam apenas na sessão por personagem; descobertas e mortes continuam persistentes. Não reconstrói informações antigas.
- Incluídos testes de metadados, cores, paginação, máscara de desconhecidos e rotinas ARM nos dois hosts; workflow Windows passa a executá-los e gerar pacotes de teste com os novos EXEs.
- Verificação local: ambos os hosts compilados com áudio e 37 testes Python aprovados; revisão de metadados/UI com sanitizadores e inspeção das imagens em 400×240 e 800×480. Compilação e testes Windows/PowerShell ficam registrados separadamente no Actions.
- Compilação dos dois EXEs com áudio, testes ARM e roteiros PowerShell aprovados no [workflow Windows](https://github.com/sugyua/destinia/actions/runs/39858876580), commit f50bcfbed4c38ce000d26cb07fea54809b5f23b6. Pacotes de fontes e Windows com SHA-256 disponíveis nos artefatos dessa execução.
- Publicação dos fontes na main; tags e ZIPs anteriores não foram substituídos. Verificação visual e campanha completa no Windows continuam pendentes.

## Lista de drops do Códex web — 22/10/2026

- Um item por linha, com cada categoria de drops em subpáginas próprias; equipamentos e receitas separados por raridade e ovos listados individualmente.
- Nomes base e raridades nas cores originais: Comum branco, Mágico verde, Raro azul, Épico roxo e Lendário laranja. Consumíveis, materiais e ovos preservam a cor branca original.
- Raridades respeitam nível observado e ramo de monstro normal, elite ou chefe; receitas originais aparecem separadas, sem anéis.
- Mantidas as categorias 1 Drops, 2 Locais de spawn e 3 Variantes. Mudança exclusiva da consulta web, sem alterar drops reais, gameplay, saves ou EXEs Windows.

## Organização dos detalhes do Códex web

- Separadas categorias fixas: página 1 Drops; página 2 Locais de spawn; página 3 Variantes.
- Listas longas têm subpáginas na própria categoria, com SUBIR/DESCER ou cima/baixo; ANTERIOR/PROXIMA, esquerda/direita e AÇÃO mudam a categoria.
- Cabeçalho com retrato frontal, HP máximo, nível e contador preservado. VOLTAR retorna à seleção anterior; desconhecidos continuam ocultos. Sem alterações de gameplay, drops ou saves.
- Alteração exclusiva do protótipo web; documentação atualizada sem modificar os EXEs Windows.

## Contagem de derrotas comuns — correção após a grade

- Corrigido o aviso de derrota do Códex nos dois hosts: `procMonster` chama `checkQuestMonster` antes de escrever o estado morto 2. A exigência desse estado descartava mortes comuns, como as de Imperial Soldier; o observador agora considera o HP esgotado nessa rotina original.
- Preservadas a deduplicação do mesmo cadáver, separação por personagem e gravação em codex-v1.txt. Não há reconstrução retroativa de mortes não registradas.
- Regressão nativa com o estado prévio à marcação de morte, além de teste web que carrega o mapa original Dungeon Prison -4 e processa dois Imperial Soldiers pelo fluxo ARM de morte.
- Na web, AÇÃO/ataque na grade abre detalhes dentro do jogo: HP máximo observado e nível, locais conhecidos e categorias de drops. As informações vêm dos mapas/scripts/tabelas originais; janela e dados de encontro são exclusivos do protótipo web nesta etapa.

## Códex em grade — atualização da main após a 0.7

- Grade de 8 colunas × 4 linhas por página, com as 73 espécies originais distribuídas em três páginas.
- Monstros desconhecidos exibem `?`; encontrados exibem a miniatura frontal completa, reduzida proporcionalmente. A seleção mostra abaixo nome ou `??????`, estado ENCONTRADO ou `???????` e contador MORTOS.
- Toque/clique e setas selecionam quadrados; ANTERIOR/PROXIMA mudam a página. Teclas e gestos da consulta não passam ao combate; voltar preserva a aba original de Craft/Pets.
- Corrigida a escolha da pose: ação 1 dos monstros comuns, que corresponde à frente na rotina original de direção, e ação 0 dos chefes. Imperial Soldier frontal ocupa 47×46 pixels antes da redução.
- Descobertas e mortes por personagem, arquivo codex-v1.txt, saves e lógica original preservados. Consultar um quadrado desconhecido não registra encontro.
- Protótipo web recebe a mesma grade e pose frontal, além de otimização própria de desenho e meta de apresentação de 16 FPS, sem alterar os EXEs por essa otimização.
- Esta mudança na main não substitui os ZIPs das releases anteriores. Validação manual no Windows e Safari físico permanece pendente.

## Correção das miniaturas e desempenho da 0.7 — fix2

- Corrigida a montagem das miniaturas do Códex: os deslocamentos das partes agora seguem as âncoras do desenho original. Corpo e arma do Imperial Soldier permanecem unidos; figura ocupa 30×45 pixels antes da redução proporcional, em vez de 75×106.
- Corrigida a leitura RGB da paleta das miniaturas; vermelho e azul estavam invertidos. Mesma correção aplicada ao protótipo web, sem alterar registros, contadores ou saves.
- No host_zoom, substituído o observador de leituras de memória da câmera por 16 pontos de atualização ARM verificados. Isso reduz a instrumentação do emulador e mantém zoom out 800×480, centralização, margens cinzas, UI e ritmo original.
- Endereços da câmera protegidos por impressão do arquivo ARM original 1.0.6 inteiro; outra biblioteca é recusada com mensagem explícita, em vez de executar pontos incompatíveis.
- Testes dos 16 escritores reais de câmera, mapas pequenos/grandes, tela cheia 3440×1440 com imagem 2400×1440, fundo cinza e tamanho do texto XP. Regressão das miniaturas inclui comparação pixel a pixel com a rotina ARM original para seis figuras, além da leitura dos 73 IDs.
- No cenário ARM de introdução medido no Linux, mediana de emulação caiu de 216,22 para 88,93 ms por quadro e a imagem final permaneceu idêntica. Não é uma medição de gameplay no Windows nem garantia de FPS; o intervalo solicitado pelo jogo continua em 60 ms nessa cena.
- Novos EXEs dos dois hosts gerados pela compilação de verificação, com correções anteriores dos inicializadores e diagnóstico mantidas. Dependências externas, dados originais, saves e relatórios pessoais permanecem fora dos pacotes.

## Correção de abertura da 0.7 — fix1

- Corrigidos os dois inicializadores BAT: a versão esperada agora vem do arquivo VERSION. Eles exigiam 0.6 e bloqueavam os EXEs 0.7 distribuídos; a checagem de versão/runtime permanece ativa.
- VERSION ausente ou inválido é informado antes de abrir o jogo; preservados os parâmetros de execução, pasta compartilhada de saves, opções de vídeo e configurações existentes.
- Diagnóstico volta a perguntar quando a resposta S/N está vazia ou inválida, em vez de interpretar como recusa. Aplica-se à abertura da janela e à reabertura da cópia de save. Fim da entrada é tratado como erro explícito.
- Relatório final destaca etapas pendentes/não executadas mesmo com código zero; isso não comprova teste visual ou gameplay.
- Testes de regressão dos BATs no CMD do Windows com EXE de teste inofensivo, versão atual/futura, incompatibilidade e caminhos com espaços; regressão do roteiro com resposta vazia antes de S.
- Pacote completo e correção pequena com somente os dois BATs, VERSION e diagnostico/test_suite.ps1. EXEs verificados da compilação ba18cede reutilizados sem alterações de gameplay; sem dados originais, bibliotecas externas ou saves.

## Atualização da 0.7 — Códex com imagens

- Pacotes Windows x64 e fontes da compilação verificada `ba18cede` disponibilizados na pré-release `v0.7-codex-imagens`, com dois EXEs e checksums SHA-256. Versão interna continua 0.7 Beta; release original preservada.

- Miniatura original de cada monstro ao lado do nome no Códex, nos dois hosts e no protótipo web. Montagem da primeira pose a partir dos sprites WBI/AGD do APK, preservando cores, transparência, variantes e o compartilhamento original de arte entre os chefes 101/103; sem sombras e efeitos externos.
- Quatro monstros por página para manter as figuras legíveis. Nome, estado e contador de mortes continuam disponíveis; formato dos saves e registros anteriores preservados.
- Cache de imagens, limites de descompressão/dimensões e indicador `?` quando um recurso estiver ausente ou incompatível, preservando a consulta. No Windows as imagens são carregadas só para as páginas consultadas, sem executar ou modificar memória ARM.
- zlib declarada nas dependências de instalação e compilação; biblioteca externa permanece fora dos pacotes. Na web a leitura usa DecompressionStream e as imagens derivam dos recursos já existentes.
- Testes com os 73 IDs originais, variantes, chefes completos, cores e espelhamento, arquivos inválidos, paginação e preservação de contadores/memória ARM.
- Workflow da release 0.7 restrito à execução manual para que ajustes na automação não tentem republicá-la; correções da `main` continuam gerando somente pacotes de teste no workflow do Códex.

- Acesso ao Códex transferido para o menu original do jogo, junto de Craft e Pets, nos dois hosts e no protótipo web: livro CODEX ao lado da barra de ícones.
- Lista e contadores desenhados dentro da imagem do jogo, com VOLTAR para a opção anterior e paginação por toque, teclado e botões de controle no Windows.
- Removidos o botão do HUD de gameplay e o botão/diálogo externos do site. F8 fica restrito ao menu, preservando remapeamentos existentes.
- Consulta suspende a execução da partida e bloqueia comandos de combate/save; os registros e o formato de codex-v1.txt são preservados. A abertura seleciona o slot atual do personagem.
- Testes do menu original ARM, ausência de botão no gameplay, retorno, captura de toque e suspensão da memória de jogo; QA de navegador em retrato/paisagem. Workflow separado compila os dois EXEs e disponibiliza pacotes de teste sem alterar a tag/release já publicada.

## 0.7 Beta — 21/10/2026

- Teste de empacotamento com fixture binária de finais de linha idênticos no Windows e Linux; o pacote continua preservando os bytes dos arquivos originais.

- Códex inicial de monstros nos dois hosts, com estados visto/derrotado e contador de mortes por espécie.
- Botão CODEX F8, atalho F8 quando não remapeado, paginação e pausa da partida durante a consulta.
- Registro auxiliar por slot em codex-v1.txt na pasta global de saves, compartilhado entre instalações e hosts; não muda o formato do save ARM nem importa mortes antigas.
- Observação de desenho/derrota pelas rotinas ARM originais, validação de compatibilidade e deduplicação por monstro/reaparecimento. O contador acumula eventos mesmo que a sessão termine sem salvar a partida.
- Gravação periódica/ao sair por temporário e substituição; arquivo auxiliar inválido preservado, com aviso e registros novos apenas na memória da sessão.
- Reinício dos registros de um slot quando seu save é excluído pelo próprio jogo; importação manual de outro personagem no mesmo slot exige o auxiliar correspondente.
- Correção do PATH do compilador para impor UCRT64 apenas no MSYS2, preservando ferramentas do chamador em outros ambientes e permitindo os testes de falhas do compilador.
- Novos testes de modelo, persistência, callbacks ARM e renderização do Códex; workflow para compilar/testar/publicar ZIPs e checksums.
- Protótipo web independente com botão Códex no topo, diálogo, contador e pausa/retomada; dados apenas durante a sessão, sem áudio.
- Ajustes maiores da UI continuam pendentes na 0.8. O Códex de equipamentos permanece para a 0.11; não foram implementados novos itens, pets, títulos, drops ou alterações de combate.
- Validação manual de campanha Windows e Safari/iPhone físico permanece pendente. Consulte docs/RELEASE_0.7.md e docs/CODEX.md.

## 0.6 Beta — 21/10/2026

- Preferências persistentes por host para tamanho de janela/tela cheia e globais para áudio, pausa e perfil em config.ini, preservando save_dir e opções desconhecidas. Salvamento de vídeo ao encerrar normalmente; flags CLI e modo sem gravação para diagnóstico.
- Configurar Destinia.bat com validação pelos dois hosts e restauração dos padrões sem alterar progresso. Escritas de preferências e seleção de pasta de save compartilham trava e substituição atômica.
- Remapeamento de teclado/botões por ação, perfis nomeados e rejeição de conflitos ou teclas reservadas. Contagem por fonte evita que soltar um alias anule outro comando ainda pressionado.
- Música e efeitos com percentuais separados, persistentes e combinados com o volume nativo; silêncio sem foco opcional. Evitada divisão por zero no mixer ao ajustar volume antes de abrir o dispositivo de áudio.
- Pausa manual pela ação configurável (F10 por padrão) e pausa opcional por perda de foco/desconexão. Entradas e toque liberados, remoção de outro controle ignorada e analógicos exigem retorno ao centro. Áudio acompanha pausa/retomada.
- F11, proporção preservada e conversão de mouse por viewport nos dois hosts. Inicializadores deixam o modo salvo prevalecer; normal abre em janela e zoom em tela cheia por padrão.
- Workflow Windows UCRT64 para compilar/testar ambos os EXEs com áudio e gerar ZIP x64 próprio, sem DLLs, instaladores, dados do APK ou saves. Pacote de fontes e checksums separados.
- Versionamento, manifesto, instalação, preferências e roadmap atualizados. Testes Linux com ARM/mixer real e verificações Windows automatizadas documentados em docs/RELEASE_0.6.md; campanha e periféricos físicos no Windows permanecem sem confirmação manual.
- Save compartilhado, logs/backups, câmera/zoom/UI, progressão/itens, ritmo nativo e limitações de compras/rede preservados. Aumento de nível/ranks continua no roadmap futuro.

- Publicação da 0.6 aprovada pelo autor; README e roadmap marcam os oito itens concluídos nas versões 0.5/0.6 e reservam a seção Atualizações futuras aos 29 itens da 0.7 em diante. ZIPs Windows e fontes com checksums disponíveis em Releases.

## 0.5 Beta — 21/10/2026

### Logs e proteção do progresso

- Log TXT automático dos dois hosts em `relatorios`, com identificação da sessão, entradas e eventos observáveis, mensagens Android nativas sem exigir `-v`, arquivos/recursos, escritas de save, estados amostrados, desempenho resumido e erros. Não há decodificação completa dos eventos internos ARM nem despejo de todos os quadros.
- Rotação de três arquivos de até 2 MiB por sessão e retenção de dez sessões por host, preservando sessões ativas. `--reports` permite escolher o destino; avisos de disco/permissão não interrompem o jogo somente por falha do logger.
- Backup verificado dos arquivos existentes antes da inicialização da partida, com dez snapshots por pasta de save. Falha na cópia impede carregar o progresso existente, preservando a origem e os snapshots completos anteriores.
- Trava por pasta para impedir que dois hosts novos escrevam a mesma partida simultaneamente; arquivo de trava é liberado pelo sistema ao encerrar e não acompanha backups. Versões antigas não respeitam essa proteção.

### Save compartilhado entre updates

- Adicionado `Configurar pasta de saves.bat` e configuração global em `%LOCALAPPDATA%\Destinia\config.ini`, usada por normal, zoom e instalações futuras compatíveis, sem copiar/apagar/mesclar partidas.
- Padrão `%LOCALAPPDATA%\Destinia\save` preservado, permitindo compartilhar também com a 0.2–0.4. Configuração personalizada só é lida pelos hosts novos; o configurador explica essa diferença e preserva a configuração anterior.
- Precedência: `--save`, `DESTINIA_SAVE_DIR`, configuração global, padrão. Configuração inválida não inicia silenciosamente outra partida.
- Inicializadores verificam a identificação 0.5 via `--version` e orientam recompilar EXEs antigos; deixam de fixar `--save`, respeitando a seleção global. Diagnóstico obtém a origem pela mesma configuração, mantém clones isolados e reúne os novos TXT dentro da sessão de teste.

### Estabilidade

- fread/fwrite de tamanho ou contagem zero retornam zero sem acessar o buffer ARM; tamanho total e erros de acesso à memória passam a ser verificados nesses caminhos.
- Carregador valida ELF32 ARM little-endian, segmentos, limites de memória, metadados dinâmicos, strings, símbolos e relocações antes de aplicá-las. Leituras sem limite/terminador e alocações excessivas são recusadas com mensagem.
- Falhas de janela, renderer e textura SDL são tratadas com liberação dos recursos criados. Falhas de execução emulada, inclusive em entradas, passam a propagar código de erro ao final.
- Encerramento fecha arquivos e libera a trava após a última escrita pendente. Exceções de stubs são contidas na fronteira do callback C do Unicorn; erros tratados geram mensagem e código de falha.

### Preparação e limites

- Testes de falhas dos dois hosts e de retenção/backup/configuração adicionados; documentação, versionamento e manifesto de fontes atualizados para a candidata 0.5.
- Compilação e verificações locais com biblioteca ARM real; publicação aprovada pelo autor em 21/10/2026, com confirmação manual de gameplay/UI/áudio no Windows ainda pendente. Detalhes em `docs/RELEASE_0.5.md`.
- Gameplay, zoom/câmera/UI, ritmo nativo e fluxo de compras/rede preservados. Sem EXEs Windows prontos, dados originais, saves ou dependências externas no pacote.

## Não lançado — documentação do planejamento (21/10/2026)

- README e roadmap atualizados com as 37 melhorias aprovadas, organizadas por dificuldade e dependências em metas provisórias de 0.5 a 0.12.
- Log TXT mantido como meta da 0.5; expansão de nível máximo, ranks, itens únicos e conjuntos incluída no planejamento, com investigação de dados, balanceamento e compatibilidade de saves antes da implementação.
- Referências da pesquisa, critérios de verificação e separação entre recursos atuais e futuros documentados. Descrição da arquitetura do host_zoom alinhada ao zoom out já presente na 0.4.
- Atualização somente de documentação: sem alterações de gameplay, nova release ou mudança na versão publicada 0.4 Beta.

## 0.4 Beta — 21/10/2026

### Compilação e inicializadores

- Adicionado `Compilar Destinia.bat` e executor Bash para MSYS2 UCRT64: o duplo clique configura e compila os dois hosts com CMake/Ninja. Os argumentos host_zoom, host e ambos continuam disponíveis.
- Falhas de configuração/compilação interrompem o processo com o código de erro preservado. Caminhos com espaços suportados; sem instalação automática de ferramentas ou dependências e sem limpeza automática do build.
- Inicializadores separados `Abrir Destinia.bat` e `Abrir Destinia Zoom 800x480.bat` incluídos no pacote; usam a mesma raiz MSYS2 configurável por DESTINIA_MSYS2_ROOT e orientam executar o compilador quando falta EXE.
- Arquivos BAT normalizados para CRLF e script Bash para LF. Saves existentes não são migrados ou apagados.

### Zoom out, fundo e UI

- Restaurado o framebuffer 800×480 no host_zoom, mostrando área maior do mapa e UI nativa proporcionalmente menor. O host normal permanece em 400×240.
- Mapas menores que a área visível centralizados por eixo. A centralização atua antes da leitura da câmera pelo ARM, incluindo caminhos com cálculo inline, e preserva a câmera nativa em mapas maiores.
- Área fora do mapa preenchida em cinza médio, com filtro dos tiles externos mantido. O cenário deixa de ser alinhado somente à direita; mapa de 480×480 tem margens de 160 pixels por lado no framebuffer 800×480.
- Margens externas da apresentação SDL também cinzas. Tela cheia proporcional e F11 preservados: área de imagem 2400×1440 centralizada no monitor 3440×1440.
- Fonte do contador de XP ajustada para manter aproximadamente o tamanho visual da 0.3 na mesma resolução de monitor, enquanto a UI original fica proporcionalmente menor. Não há controle configurável de escala da UI nesta versão.
- Diagnóstico atualizado para BMPs 800×480, identificação do novo zoom e pergunta manual sobre UI/XP. EXE desatualizado é recusado com instrução de recompilação.

### Verificações e limites

- Compilação em Linux e testes automatizados de inicialização, BMPs, empacotamento, compilador e diagnóstico; fixture ARM com mapas pequenos/grandes, escrita inline simulada, preenchimento cinza e preservação de pixels internos.
- Verificação de janela/textura, alternância F11, geometria ultrawide simulada e altura do texto XP. A confirmação visual em gameplay no Windows, incluindo cliques na UI, PT e F9, permanece pendente.
- Sem alteração de compras/rede e sem reteste de áudio nesta preparação. Sem EXE Windows pronto ou dependências no pacote de fontes.
- Log TXT detalhado dos eventos do jogo reservado para a **0.5**; não implementado na 0.4. Os relatórios de diagnóstico já existentes permanecem disponíveis.
- Releases 0.2 e 0.3 preservadas. Notas, manifesto e instruções da 0.4 revisados; publicação aprovada pelo usuário.

## 0.3 Beta — 21/10/2026

Esta versão registra o estado atual com enquadramento original ampliado. A restauração do zoom out, o fundo cinza, a centralização dos mapas pequenos e o ajuste da UI ficam para a próxima atualização.

### Tela cheia e enquadramento

- `host_zoom` renderiza em 400×240 e amplia a apresentação, com pixels nítidos e proporção preservada; a janela inicial é 800×480.
- Opção `--fullscreen` para tela cheia sem bordas na resolução atual do monitor. F11 alterna entre tela cheia e janela. Launcher do zoom e teste focado iniciam em tela cheia.
- No monitor 3440×1440, a imagem ocupa 2400×1440, centralizada, com barras externas de 520 pixels por lado. Não é renderização nativa ultrawide.
- Mapeamento do mouse acompanha a área central e o redimensionamento; cliques nas barras externas não acionam o jogo.
- PT, HUD, comandos e F9 preservados. As capturas do zoom passam a ter 400×240.
- O zoom out anterior de 800×480 foi retirado nesta versão. A ausência da faixa preta/duplicação no enquadramento original não significa correção da câmera em visão ampliada.

### Diagnóstico e proteção de saves

- Adicionados `Testar Destinia.bat`, executor Windows PowerShell 5.1 e roteiro em português; nenhuma instalação, download ou compilação automática.
- Menu com teste focado de câmera do host_zoom e roteiro geral dos dois hosts; opções por linha de comando preservadas.
- Saves copiados para pastas isoladas por sessão/host, recusa de sobrescrita/links e comparação de hashes da origem antes/depois.
- Verificação de ELF ARM, assets, executáveis, logs, contagem de quadros e integridade básica dos BMPs. Erros conhecidos no log não são ignorados quando o processo retorna zero.
- Captura de stdout/stderr e timeout somente dos processos iniciados pelo diagnóstico.
- Relatórios TXT/JSON desde o início, atualizados durante o roteiro, com etapa, indicação de sessão parcial, falhas, avisos e pendências.
- Roteiro manual de gameplay, salvar/reabrir, câmera, entradas, áudio, PT/F9; modo focado evita repetir os demais itens e registra a descrição da falha.
- Registro passivo de câmera, orientação de capturas F9 antes/depois e contagem de imagens. EXE antigo de 800×480 é identificado e exige recompilação.
- Avisos de alterações locais de cash e rede aguardando na última sonda; recuperação posterior da rede evita aviso de espera ainda pendente. Compras não são validadas automaticamente.
- Pacote separado do diagnóstico, com BAT, executor e instruções, sem dados originais, saves, executáveis ou dependências.

### Organização, documentação e distribuição

- Removidos logs locais, backups, cópias antigas de fontes, configuração do editor, arquivos vazios e material pessoal de continuidade/exportação de conversas.
- Reforçado o ignore para saves, relatórios, backups e credenciais. Nenhum save pessoal estava rastreado na árvore limpa; commits antigos não foram reescritos.
- Mantidos fontes ativos, recursos originais no repositório e dados necessários à compilação; arquivos de distribuição continuam excluindo o APK extraído e saves.
- BAT de diagnóstico normalizado para CRLF no Windows; launcher do zoom e instruções atualizados.
- Adicionado contato para colaboradores no README, mantendo a seção Dependências e instalação separada de ferramentas/bibliotecas.
- Versão 0.3 Beta no VERSION, manifesto e títulos dos dois hosts; notas, verificações, ZIP curado de fontes e SHA-256 preparados para revisão.

### Verificações e limites

- Compilação, inicialização ARM, capturas e testes do empacotador/diagnóstico verificados em Linux. Teste da rotina ARM da câmera com mapa de 30×30 tiles; janela/textura, F11 e geometria ultrawide simulada também exercitados.
- No Windows, o usuário informou troca de mapa sem defeito na sessão de 20/10 às 20:58, com inicialização 400×240, código zero e hashes da origem inalterados. Não houve captura F9 nessa sessão; o resultado manual não aprova todos os recursos.
- A UI original foi ampliada junto com o cenário e continua grande; o usuário considera o contador de XP a referência desejada para o ajuste futuro. O contador já existia antes da 0.3.
- Compras/rede continuam pendentes: alteração local de cash e rede aguardando foram observadas. Não foi confirmada compra online nem persistência do saldo.
- Permanecem limitações de libc/ELF/SDL já documentadas na 0.2. Sem novo EXE Windows distribuível, instalador ou validação em máquina limpa; áudio não foi retestado em Linux nesta preparação.

## 0.2 Beta — 20/10/2026

### Alterações em relação ao commit inicial

- Identificação “Destinia 0.2 Beta” no título da janela de `host` e `host_zoom`.
- Arquivo `VERSION` com o número 0.2.
- README e instruções de instalação/compilação em português, com distinção entre fontes e pacote executável.
- Este changelog com aviso de Beta no início.
- Notas propostas para a primeira release, plano de publicação e relatório de verificações/pendências.
- Registro dos componentes originais e dependências de terceiros, sem atribuir uma licença não autorizada.
- Identificação de `game` como o APK original do Destinia Android extraído.
- Seção **Dependências** no README e política de instalação separada: instaladores, ferramentas e bibliotecas externas necessárias no PC não serão incluídos no arquivo da release.
- Regras de exportação em `.gitattributes` para manter o APK extraído, backups e logs fora dos arquivos automáticos de código-fonte da tag, preservando esses arquivos no repositório.
- Empacotador de fontes com lista explícita de arquivos, validação de caminhos e versão, ZIP reproduzível no mesmo ambiente, SHA-256 e proteção contra sobrescrita.
- Testes do empacotamento para evitar inclusão de saves, credenciais, dados originais, caminhos externos e artefatos anteriores.
- Atualização das orientações de repositório/versionamento em `AGENTS.md` e inclusão de `/dist/` no ignore.

### Preservado

A única alteração nos fontes C++ é o título da janela. Gameplay, resolução, câmera, biblioteca ARM, renderização, controles, tradução, áudio, saves, compras e instrumentação F9 não foram modificados. Os CMakeLists e launchers existentes permanecem preservados.

### Limitações conhecidas

- Câmera/transição de mapa em 800×480 ainda em investigação.
- Fluxos de compra/rede podem ficar aguardando resposta.
- Falhas emuladas podem encerrar com código zero.
- Tamanho zero nos stubs de fread/fwrite é tratado incorretamente.
- Possível desalinhamento de mouse/DPI em `host`.
- Falta verificação dos retornos de criação dos recursos SDL.
- O carregador ELF não valida todos os limites de offsets/tamanhos antes de acessar o arquivo.
- Launchers dependem do MSYS2 em caminho fixo e compartilham save; scripts antigos de teste podem usar uma origem de save desatualizada.
- Não há novo executável Windows, validação de DLLs ou teste de gameplay desta candidata.

## Base anterior

Commit inicial: `d9a9352bbbe9902b7b9bc3ef0dac2353b3f26611`, “Backup inicial do projeto Destinia”. Não havia releases no GitHub na consulta desta preparação. Não foi inventada uma versão 0.1.

A preparação 0.2 feita anteriormente nesta conversa não chegou ao GitHub; esta candidata a substitui e amplia.
