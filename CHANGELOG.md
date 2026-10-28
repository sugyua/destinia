> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

# Histórico de alterações

Histórico em português, da versão mais recente para a mais antiga. Somente alterações efetivas são registradas como novidades.

## 0.16.0 Beta - 28/10/2026

- Desmontagem em lote nos hosts normal e Zoom, com previa baseada nos filtros ativos e confirmacao separada.
- Limite de 50 itens; itens protegidos/equipados ou alterados apos a previa sao excluidos. Cada alvo e revalidado e usa o menu ARM original.
- F8 inicia/confirma o lote; Esc cancela a previa. Uma inconsistencia interrompe a sequencia e mostra quantos itens foram concluidos.
- Verificacao: testes de politica/painel e ARM nos dois hosts aprovados; builds Release CMake/Ninja aprovados. Correcao posterior: cancelar previa tambem ao fechar o painel/resetar, com estado revalidado no teste nativo.

## 0.15.0 Beta - 28/10/2026

- Ctrl+F3 abre uma ficha somente leitura com PV/PM maximos, ATK, DEF, HIT, EVA, CRI e BLD, lidos do calculo nativo ativo.
- Os hosts normal e Zoom nao recalculam atributos nem alteram equipamento ou save.
- Verificacao: testes de formato/renderizacao e builds Release CMake/Ninja dos dois hosts aprovados. O glifo % foi incluído para exibir HIT/EVA/CRI/BLD corretamente.

## 0.14.0 Beta - 28/10/2026

- Hosts PC normal e Zoom: painel F12 mostra FPS, tempo medio de emulacao e memoria de trabalho do processo. A coleta ocorre somente enquanto o painel esta visivel.
- Verificacao: teste focado e builds Release CMake/Ninja dos dois hosts aprovados. A leitura de memoria agora substitui valores antigos tambem quando a consulta retorna zero.

## 0.13.0 Beta - 28/10/2026

- Arena infinita em PC e Web com ate 12 inimigos simultaneos, chefe a cada 5 ondas, elite a cada 3 e recordes por personagem.
- Titulo Mestre da Sobrevivencia na onda 20 com bonus de atributos, resistencia e esquiva.
- Ultimo Suspiro com limite de dano, 2 segundos de imunidade e 120 segundos de recarga; elixir opcional de 20% por 5 minutos.
- Refino de armas e armaduras ate +15, pedras por faixa e progresso separado do save original.
- Validacao: suite Web, regressao focada, smoke ARM de 51 ondas e builds Release dos dois hosts aprovados. O teste de arena e executado explicitamente nos hosts normal e Zoom.

## 0.12.1 Beta — 27/10/2026

- Corrigidos atalhos F4/F5/F8 no teclado Web, com press/release, proteção contra repetição e F5 durante pausa. Tab e comandos anteriores preservados.
- Notas do pacote Windows selecionadas por VERSION; ausência interrompe o empacotamento. DLLs externas excluídas conforme diretriz do autor.
- README, manifesto, banner e roadmap histórico alinhados. Web 0.12.1 em fontes; deployment externo não concluído por ferramenta indisponível.
- [Verificação e limites](docs/VERIFICACAO_0.12.1.md). Demais itens da Issue #5 não implementados neste lote.

## 0.12.0 Beta — 26/10/2026

- **Modo Abissal Hardcore Completo**:
  - **100% Aura de Elite:** Todos os monstros do mapa nascem compulsoriamente como monstros Elites em `common/difficulty_arm.h`, garantindo o desafio supremo para heróis no Nível 999.
  - **Escalonamento Brutal:** Multiplicadores nativos de 6.0x de Vida e 6.0x de Dano base em relação ao modo normal.
  - **Zero Bônus de Drop:** Tabela de recompensas e raridades preservada idêntica à da campanha normal sem facilitação, servindo puramente como teste definitivo de maestria.
- **Título Lendário "Senhor do Abismo"**:
  - Título honorífico exclusivo concedido ao derrotar Diablo (`species 108`) no Modo Abissal.
  - Concede **+150% em todos os atributos principais** (ATK, DEF, PV, PM).
  - Persistência isolada no sidecar `titles-v1.txt` (PC) e `localStorage` (Web).
- **Aba de Títulos no Mago dos Bosses (5ª Aba: [TITULOS])**:
  - O Mago dos Bosses agora conta com 5 abas ([CHEFES], [TORRE], [MASCOTES], [SANTUARIO], [TITULOS]).
  - Lista de 8 títulos com descrição de requisitos, badges [EQUIPADO], [LIBERADO] ou [BLOQUEADO], e acionamento por botão/toque para equipar.

- **Correções e Estabilidade (0.12.0)**:
  - **Rastreamento de Bounties / Contratos Diários:** Corrigido o ID de espécie dos Soldados Imperiais de `2` (Fungus Mágico) para `61` (Soldado Imperial canônico de `mon_data_en.gdt`), permitindo que abates de Soldados Imperiais (`species 61, 62, 63`) sejam devidamente computados no contrato "Caça: Soldados Imperiais" e no Rastreador de Missões (Quest Tracker).
  - **Migração Automática:** Saves existentes com contratos apontando para o ID antigo são migrados transparentemente ao carregar o jogo.

## 0.11.0 Beta — 26/10/2026

- **Suporte Integral a Controles USB / Gamepads (PC e Web)**:
  - Integração nativa e unificada para que 100% dos novos recursos, menus e modais sejam ativados e navegados usando controles USB (Xbox, PlayStation e gamepads genéricos):
    - **`R3` (Clique do Analógico Direito)**: Alterna ativação imediata da **Auto-Poção** inteligente.
    - **`L3` (Clique do Analógico Esquerdo)**: Liga/desliga a visualização do **Rastreador de Missões (Quest Tracker)** no HUD.
    - **Guia Visual de Atalhos no Rodapé (OSD)**: Segurar o botão `SELECT` (ou `BACK`) exibe em tempo real uma faixa com borda dourada Gamevil indicando os atalhos disponíveis (`[SELECT]+ [X] Catálogo [Y] Baú [B] Bounties [A] Missão [R3] Auto-Poção`).
    - **Combinações de Atalho (Chords) com `SELECT`**:
      - `SELECT + X`: Abre o **Catálogo de Equipamentos**.
      - `SELECT + Y`: Abre o **Baú Compartilhado entre Personagens**.
      - `SELECT + B`: Abre o **Mural de Contratos / Bounties Diárias**.
      - `SELECT + A`: Alterna o **Rastreador de Missões**.
      - `SELECT + D-Pad Cima`: Abre o menu do **Mago dos Bosses** (Boss Rush / Torre Abissal / Mascotes / Santuário).
      - `SELECT + D-Pad Baixo`: Alterna a **Auto-Poção**.
      - Toque rápido em `SELECT`: Executa Salvamento Rápido nativo (`touch(0, 0, 3)`).
  - **Navegação Completa por D-Pad e Botões nos Menus e Modais**:
    - *Catálogo*: Esquerda/Direita para mudar páginas, Cima/Baixo/Y para trocar abas de classe, X/A para alternar entre Armas e Armaduras, B/Back para fechar.
    - *Baú Compartilhado*: D-Pad para navegar pela grade 8x3 de itens, Botão A para transferir/retirar item, X/Y/LB/RB para alternar páginas 1 e 2, B/Back para fechar.
    - *Mural de Contratos*: D-Pad Cima/Baixo para selecionar contratos, Botão A para reivindicar recompensas, B/Back para fechar.
    - *Mago dos Bosses*: D-Pad Cima/Baixo para selecionar opções, LB/RB para alternar abas temáticas, Botão A para confirmar desafio, B/Back para fechar.
- **Catálogo Completo de Equipamentos e Complecionista (F4 / Ctrl+W / SELECT+X)**:
  - Expansão do catálogo para 48 equipamentos canônicos: 24 Armas (Espadas, Pistolas/Canhões e Adagas) e 24 Armaduras (Peitorais leves, médios e pesados), distribuídos em 8 patamares por classe (Guerreiro, Atirador e Ladino).
  - 5 patamares de raridade rastreados para cada equipamento (Comum, Mágico, Raro, Épico e Lendário), totalizando 240 entradas colecionáveis.
  - Alternância imediata de categoria entre Armas e Armaduras via tecla `T` ou toque no botão de categoria.
  - Varredura em tempo real do inventário em busca de novas descobertas, com cálculo proporcional de progresso e sinergia com o título `Complecionista` (+100% de atributos principais ao completar).
  - Persistência transacional e segura no sidecar `weapons-catalog-v1.txt` (PC) e `localStorage` (Web).
- **Baú Compartilhado entre Personagens / Shared Stash (F5 / B)**:
  - Capacidade ampliada para 48 slots divididos em 2 páginas de 24 itens, acessível a qualquer momento ou em cidades.
  - Permite depositar e retirar armamentos, elmos, armaduras, luvas, botas, acessórios e consumíveis entre todos os saves do jogador.
  - Preservação estrita da integridade: gravações atômicas em `shared-stash-v1.txt` com checksums e backups, sem tocar na estrutura binária dos arquivos `save*.dat` originais.
  - Interface intuitiva com destaque por cor de raridade, navegação por teclado e toque nas duas páginas.
- **Sistema de Bounties / Contratos Offline Repetíveis (F8 / C)**:
  - 3 contratos diários gerados por slot de personagem: Caça a Monstros Comuns (ex: Soldados Imperiais), Caça a Chefes (ex: Diablo) e Desafios de Caça Épica.
  - Contabilização automática e sem overhead de abates através do hook ARM nativo de derrota de monstros.
  - Resgate instantâneo de recompensas com premiações balanceadas em Ouro e Cash offline.
  - Interface com barra de progresso em tempo real, status de resgate e persistência em `bounties-v1.txt` (PC) e `localStorage` (Web).
- **Rastreador de Missões no HUD / Quest Tracker (Q / Toque)**:
  - Widget retrátil no canto superior direito do HUD exibindo missão ativa ou contrato diário prioritário, contagem de abates e destino.
  - Moldura e paleta dourada autêntica Gamevil (`0xee00` / `0x1084`), ícone de pergaminho pixel-art estilizado e alternância por toque ou tecla `Q`.
- **Arena Gauntlet / Boss Rush de Sobrevivência (Mago dos Bosses)**:
  - 11ª opção no Mago dos Bosses (`species 999`), desafiando o jogador a derrotar em sequência todos os 10 chefes de Destinia (Dungeon Guardian até Lava Guardian).
  - Taxa de entrada de 10.000 G e recompensas multiplicadas ao sobreviver ao desafio.
- **Auto-Potion QoL / Consumo Inteligente de Poções (H)**:
  - Monitoramento contínuo de PV (< 35%) e PM (< 25%), consumindo automaticamente poções da bolsa de consumíveis com respeito estrito ao cooldown de 60 frames (1 segundo).
  - Tecla de atalho `H` para alternar entre Ativado e Desativado com aviso visual translúcido no HUD.
- **Reset de Atributos e Habilidades Offline (Santuário do Poder)**:
  - Opções integradas ao Mago dos Bosses permitindo resetar Atributos (FOR, DES, CON, INT) e Habilidades por 5.000 G de ouro do jogo.
  - Devolução integral de pontos acumulados para redistribuição imediata e recálculo seguro no motor ARM nativo (`GameUI_calcHeroStat`), eliminando o bloqueio de builds originais dependentes da loja online descontinuada.
- **Mascotes & Companheiros Offline / Pets System**:
  - 4 mascotes canônicos introduzidos: Lobo Glacial (+10% Velocidade, +15% Crítico), Fênix Ígnea (+20% Dano de Ataque, +10% Regen PV), Golem de Pedra (+25% Defesa, +15% Vida Máx) e Mini-Diablo (+15% Dano Geral, +5% Roubo de Vida).
  - Progressão individual de níveis até Lv50, persistência segura no sidecar `pets-v1.txt` e mascote flutuante com animação de flutuação e sombra suave desenhado ao lado do herói durante o jogo.
- **Ciclo Novo Jogo+ / New Game+ (NG+1 a NG+5)**:
  - Reinício de campanha preservando nível, experiência, atributos, equipamentos, baú compartilhado, títulos e mascotes.
  - Escalonamento progressivo de dificuldade (+35% de Vida e Dano por ciclo) e ganhos (+50% de XP e Ouro por ciclo), gravado no sidecar canônico `progression-v1.txt`.
- **Torre Abissal / Masmorra Infinita (50 Andares)**:
  - 50 andares de desafio progressivo no Mago dos Bosses, com andares de chefes a cada 5 níveis e recompensas escalonadas de ouro e experiência registradas no sidecar `abyss-v1.txt`.
- **Otimizações Críticas e Performance Web (Mobile / Safari iOS)**:
  - Resolução definitiva de gargalos em navegadores móveis: substituição de conversão lenta por tabela de busca (LUT) RGB565 -> RGBA32 de 256 KB, acelerando o desenho de quadro para ~0.35ms (ganho de 7x de performance).
  - Pacing do Web Worker acionado diretamente pela janela principal (`nextFrame`), evitando o estrangulamento de segundo plano do Safari (que limitava a 3 FPS). Framerate cravado em 16 FPS.
  - Avanço automático suave pelas telas de apresentação (Everple / Gamevil / Logos) e suporte a toque para pular direto para o menu inicial.
  - 129 testes unitários automatizados passando com 100% de sucesso.

## 0.10.3 Beta — 26/10/2026

- **Sistema Canônico de Dificuldades (Normal, Difícil, Pesadelo, Inferno e Abissal)**:
  - Multiplicadores de HP, Dano, XP e Ouro implementados via hooks ARM nos executáveis `destinia_host.exe` e `destinia_host_zoom.exe`.
  - Normal (1.0x), Difícil (1.5x/1.3x), Pesadelo (2.5x/1.8x), Inferno (4.0x/2.5x) e Abissal (6.0x/4.0x).
  - Modo Abissal preserva taxa padrão de drops de itens (zero bônus em itens), garantindo o teste de maestria.
  - Escala proporcional de vida sem empilhar em respawns e sem reviver mortos; dano final respeita defesas e críticos nativos.
  - Congelamento de dificuldade na morte (`deadTier`), impedindo alteração de recompensas ao trocar de menu durante animação.
- **Interface e Navegação**:
  - Menu Sistema inclui botão "Mudar Dificuldade", permitindo escolher entre os níveis desbloqueados no slot.
  - Indicador da dificuldade atual renderizado ao lado da classe na tela de Seleção de Personagem (apenas com save).
  - Desbloqueio progressivo: derrotar o Boss Final Diablo (`species 108`) libera o nível seguinte no slot.
- **Persistência Transacional Segura**:
  - Saves binários originais `save*.dat` 100% preservados. Desbloqueios e configurações gravados atomicamente no sidecar `difficulty-unlocks-v1.txt`.
  - Homologado em testes nativos nos dois hosts e auditado pelo Sentinel (Score 10/10).

## 0.10.2 Beta — 26/10/2026

- **Mago dos Bosses Presente em Todas as 5 Cidades (Safe Zones com Lojas)**:
  - Injeção padronizada em todos os 5 mapas que cumprem a regra estrita de Safe Zone (bloqueio nativo de ataque/dash em `reserveHeroAct` e `GameCanvas_mKeyPressed`) com ambos os vendedores (Armas e Poções):
    - Acampamento da Resistência (Resistance Camp - Mapa 61): X=280, Y=232.
    - Bergheim (Mapa 62): X=504, Y=280.
    - Acampamento dos Refugiados (Refuge Camp - Mapa 63): X=280, Y=296.
    - Fermion (Mapa 64): X=344, Y=424.
    - Lagrant (Mapa 65): X=328, Y=344.
- **Hitbox de Colisão Física com o Cenário**:
  - Implementada colisão sólida de 16x16 na grade nativa do mapa (`curMapData + 0x68`), impedindo que o jogador atravesse o Mago.
  - Alcance de interação (`mageFacing`) calibrado para até 28 pixels para garantir acionamento natural imediato ao encostar no Mago.
- **Letreiro de Nome com Fundo Nativo e Proximidade**:
  - O nome "MAGO DOS BOSSES" agora só é renderizado quando o jogador está a menos de 48 pixels de distância.
  - Caixa de fundo escuro semitransparente com bordas arredondadas e padding, perfeitamente integrada à identidade visual da UI nativa do jogo.
- **Gatilho de Interação e Menu de Teleporte Ativo**:
  - Pressionar o botão de Ação (ou tecla de ataque) abre a interface interativa de seleção de Boss Rush (`BossRushUI`), permitindo teleporte pago em ouro (500 a 50.000 G) para qualquer um dos 10 chefes de espécie 100 a 109.

## 0.10.1 Beta — 26/10/2026

- **Correção Crítica de Inicialização Web em Redes Locais / HTTP**:
  - Resolução da falha de tela preta undefined is not an object (evaluating 'crypto.subtle.digest') observada ao acessar o jogo via IP local (ex: http://192.168.x.x:8000/) em dispositivos móveis e navegadores sem HTTPS.
  - Implementado algoritmo SHA-256 nativo em JavaScript puro como fallback transparente dentro do Web Worker (sha256Hex). A validação de integridade dos binários não depende mais da API WebCrypto restrita a contextos seguros.
  - Atualização do controle de cache (?v=0.10.1), cabeçalhos Cache-Control: no-cache e desregistro forçado de Service Workers legados.

## 0.10.0 Beta — 26/10/2026

- **Correção Crítica de Viewport e Performance Web 800x480**:
  - Resolução da regressão visual onde mapas menores que 800x480 apareciam duplicados no canto direito e o framerate caía para ~7 FPS.
  - Implementado em web/dist/map-viewport.mjs espelhando host_zoom: centralização de eixos pequenos em Game+18552/18554, limpeza de margens com cinza médio (0x8410) em 0x19308 e descarte imediato (0x4000 com fastReject) de tiles fora dos limites em 0xBB90/0xB4BC.
  - Queda de milhares de chamadas redundantes para renderização leve a ~55ms por quadro (meta de 16 FPS estável). Suíte completa Web com 114 testes aprovados (0 falhas).
  - Cache buster aplicado no Service Worker/Web Worker e cliente web (?v=0.10.0), com headers no-cache e desregistro forçado de caches legados para dispositivos móveis.
- **Barra Superior Centralizada (10 Ícones)**:
  - Recálculo dos offsets em X de todos os ícones nativos para centralização exata da barra com a adição do 10º ícone (Códex). Hitboxes de toque e clique recalculadas e perfeitamente alinhadas em 400x240 e 800x480.
- **Mago dos Bosses (Boss Rush)**:
  - NPC em Lagrant estilizado em memória com base no mon_6_0 (paleta azul/branco/sombra).
  - Tabela canônica de custo em ouro por Boss (species 100 a 109, custando de 500 a 50.000 ouro de acordo com o nível/dificuldade).
  - Disparo de evento deferido openBossRushMenu() fora da emulação ARM para a interface de seleção.
- **Sistema de Títulos & Título Complecionista**:
  - Persistência sidecar titles-v1.txt com locking e compare-and-swap.
  - Título Complecionista desbloqueado ao registrar 100% do catálogo do Códex (73 monstros), concedendo bônus épico de +100% em ATK, DEF, PV e PM.
- **Frente 1 (Vanguard - Core de Progressão e Nível 999)**:
  - Núcleo puro NV1–999: XP em u64/BigInt com extensão da curva original até o cap de 2.035.180.079.455 XP.
  - 14 Patentes de Ranks (Aprendiz até Supremo), status parametrizados e guardas contra overflow e truncamento ARM.
  - Cinco dificuldades com multiplicadores aprovados e NG+ linear configurável.
  - Suporte a conjuntos de equipamento com bônus progressivos cumulativos para patamares de 2, 3, 4, 5 e 6 peças equipadas.
  - Sidecar progression-v1.txt com CAS e lock; saves nativos intocados.
- **Frente 2 (Sentinel - Sistemas de Loot e Economia)**:
  - Drops exclusivos para os 10 chefes de espécie 100 a 109.
  - 5 Conjuntos de Equipamento completos com origens Concentradas e Distribuídas.
  - Economia de Cash reformulada: remoção total do ganho passivo de Cash offline por tempo. Cash concedido exclusivamente por 5 Missões Épicas de marcos de chefes e set completo (10 a 25 Cash), calibrado com os preços da loja (.gdt).
  - Sidecar economy-v1.txt / DESTINIA_ECONOMY_2 sem tocar nos saves originais.

### Backlog Planejado para 0.10.1 (Endgame e Seleção de Dificuldade)

1. **Troca de Dificuldade no Menu Sistema**: Botão 'Mudar Dificuldade' no menu Sistema listando as 5 dificuldades; liberação sequencial após derrotar o Boss Final da dificuldade atual; opções bloqueadas em tom escuro/cinza.
2. **Indicador de Dificuldade na Seleção de Personagem**: Dificuldade do save em fonte branca nativa ao lado do nome da classe (apenas para personagens salvos).
3. **Reaproveitamento Estrito**: 100% de recursos, fontes e molduras originais da engine.


- Barra superior: dez ícones centralizados, desenho/cursor/hitboxes compartilhados nos dois tamanhos e escalas; superfície/toque web800 corrigidos sem criar estados ARM extras.
- Mago Boss Rush: paleta azul/branco/sombra isolada sobre mon6_0 e shadow originais, NPC host-side em posição livre de Lagrant, ação nativa manual com evento openBossRushMenu diferido para a UI externa. Pools, colisões, assets e saves preservados; ver docs/BOSS_MAGE.md para contrato e identificação pendente de Icarus.

> **Beta em desenvolvimento: integração ARM/HUD e validação física pendentes.**

- **Frente 1 (Vanguard - Core Progressão e Matemática):**
  - Novo núcleo puro C++/web para NV1–999: XP u64/BigInt com a curva original estendida, 14 ranks canônicos, status u32 parametrizados e guardas contra overflow/truncamento ARM (u16/i16/u32).
  - Cinco dificuldades com multiplicadores aprovados e NG+ linear configurável; Normal/NG+0 permanece 1x. Não há reset automático de nível ou campanha.
  - Estruturas puras para bônus incrementais de sets em 2, 3, 4, 5 e 6 peças, acumulados até o patamar equipado; composição com dificuldade/NG+ e aplicação a status/XP/ouro com overflow verificado. Formato do sidecar progression-v1 preservado.
  - Sidecar versionado progression-v1.txt por modo/slot, codec C++/JS idêntico, parser estrito, lock/CAS e substituição atômica nativa; adaptador web exige Web Locks e confirmação de gravação. Arquivos save*.dat e pools/memória ARM intactos.
- **Frente 2 (Sentinel - Sistemas de Loot e Economia):**
  - **Drops Exclusivos de Boss:** Catálogo de 10 itens lendários exclusivos para cada um dos chefes do jogo (species 100 a 109). Drops integrados ao Códex de monstros e persistidos em sidecar.
  - **Sets de Equipamento Expandidos (2 a 6 Peças):** 5 conjuntos temáticos (Set do Dragão, Set Sombrio, Set Imperial, Set Arcano, Set do Guardião) com bônus progressivos cumulativos para 2, 3, 4, 5 e 6 peças equipadas.
  - **Origens de Drops:** Sets Concentrados (Set do Dragão e Set do Guardião caindo exclusivamente de seus respectivos chefes) e Sets Distribuídos (Set Sombrio, Imperial e Arcano espalhados entre chefes e monstros elites).
  - **Economia de Cash Calibrada e Missões Épicas:** Calibrada contra itens premium originais (.gdt). Ganho de Cash offline com teto estrito de 24 horas (máx 48 Cash) e Missões Épicas com metas de 10, 25, 50, 100 chefes derrotados e set completo de 6 peças concedendo recompensas justas de Cash.
  - **Integridade de Saves:** Sidecar economy-v1.txt / DESTINIA_ECONOMY_2 mantendo save*.dat 100% intocados. Testes C++ e Node aprovados.

## 0.9.9 Beta — 26/10/2026

- Ajustes aprovados pelo proprietário: F7/P alternam proteção diretamente no equipamento do cursor, sem abrir FILTROS; F6 passa a abrir FILTROS. Item/Comparar continuam acessíveis por clique e controle/RS.
- Proteção na web passa pela janela: worker solicita gravação e aguarda ACK do localStorage antes de confirmar F7/P. Falhas/concorrência não confirmam a alteração; venda bloqueada durante pendência, sem escrever nos saves ARM.
- Bolinha verde no canto superior direito dos ícones nativos e indicador no painel filtrado quando a comparação de FC confirma ganho naquele slot. Simulação restaura Game e usa cache invalidado por mudança de estado/equipamento; não equipa nem modifica saves.
- Ouro automático passa a vir ativo por padrão, com raio nativo19. Configuração explícita coleta.ouro=0/localStorage desativado é respeitada; materiais e filtro visual continuam opt-in.

- Filtro visual de drops opcional: cores/marcadores por categoria e rank, máscara de categorias e rank mínimo de equipamento. Oculta somente sprites, nomes e marcadores; processamento de existência/expiração e coleta manual permanecem nativos.
- Autoloot opcional e separado para ouro e materiais de fabricação confirmados nas tabelas originais. Ouro ativo por padrão e materiais opt-in, raio configurável de 1–19 pixels por eixo, um alvo por ciclo de quatro frames e espera de 32 frames após tentativa sem coleta. Não coleta equipamento, poções, receitas ou itens de missão; mochila cheia mantém o material no chão.
- Coleta usa o caminho ARM original, limitado ao alvo elegível e encerrado antes de interações com NPC/ataques. Pausa, menus/transições e perda de foco suspendem assistência. Configuração em config.ini no PC e controles/localStorage na web, sem novo formato ou gravação direta em save*.dat.
- Classificação usa a tabela original antes da tradução PT-BR. Testes ARM em host/host_zoom e web 400×240/800×480, inclusive PT-BR, cobrem capacidade, não duplicação, filtro sem alteração extra de Game e nenhuma chamada a NPC/ataque/save. Validação física de campanha/controle permanece pendente.
- Suíte legada Node declara explicitamente a fixture 400×240 após a migração prévia do produto para 800×480; o padrão da engine web continua 800×480 e novos casos validam ambas as bases. README, preferências e allowlist de fontes atualizados; release/deployment aguardam integração/revisão.

## 0.9.8 Beta — 25/10/2026

- Equipamentos podem receber/remover proteção pelo painel FILTROS, botão PROT/DESPROT, F7 livre e ação extra do controle. Indicador `[P]` e nome dourado; seleção normal para equipar preservada.
- No estado original `0x91040004`, itens protegidos permanecem visíveis, mas não podem ser escolhidos no painel nem confirmados no menu ARM de venda/desmanche, incluindo comandos gerados pelo toque. Navegação, pools, gold e formato dos saves preservados.
- Persistência autorizada em sidecar `inventory-protect-v1.txt` na pasta de saves, por personagem/pool/fingerprint; hosts compartilham a configuração da pasta e clássico/expandido seguem separados. Nenhuma escrita nova em `save*.dat` e save compatível 100% com o Google Drive.
- Gravação atômica do sidecar; falhas preservam o estado anterior. Arquivo inválido é preservado e bloqueia confirmações destrutivas no contexto protegido. Web usa metadados locais por modo.
- Códex com separação entre Monstros Comuns e Chefes/Bosses: adicionadas abas de navegação [TODOS], [COMUNS] (`species < 100`) e [CHEFES] (`species >= 100`), com filtro de exibição na grade 8x4 e identificador visual destacado para chefes nos dois hosts PC e na versão Web.
- Bateria de testes automatizados dedicados em C++, Node e sondas ARM dos dois hosts (`tests/inventory_favorite_cases.cpp`, `web/tests/inventory-favorite.test.mjs`, `web/tests/codex*.test.mjs`) validando alternância, integridade de memória ARM, bloqueio de transação no vendedor e filtros do Códex.
- Formato de saves originais, atributos, catálogo de drops, taxas e rotinas ARM nativas 100% preservados. Validação automatizada em ARM e Chromium; testes manuais com controle físico no Windows/iPhone pendentes.



## 0.9.7 Beta — 25/10/2026

- FC na aba Status desenhado com a fonte original do jogo e fundo preservado, substituindo a faixa azul.
- Comparador integrado à caixa nativa de descrição do equipamento, com páginas Item/Comparar, moldura e fonte originais e nome na cor de raridade. Item mantém a descrição original; Comparar mostra FC atual→previsto, ganho/perda e diferenças de ATK/DEF/PV/PM/CRI.
- Corrigido o mapeamento entre classe do personagem (`Game+0x4886`: `0=Guerreiro`, `1=Atirador`, `2=Ladrão`) e tipo de arma (`(header>>5)&7`: `0=Espada Grande`, `1=Espada/Adaga`, `2=Arma de Fogo`) no comparador de FC dos dois hosts C++ e da web, aplicando a tabela nativa `{0, 2, 1}` (`0x40f7c`).
- Alternância por clique/toque no rodapé, F6 (se não remapeado no PC) ou analógico direito. F7/filtros, seleção, comandos de equipamento e modo clássico preservados; página escolhida vale para a sessão.
- Fórmula de FC, buffs, requisitos, atributos, inventário, gold e formato de saves permanecem iguais. Desenho usa cache e restaura estado gráfico/superfície emprestada mesmo após falha.
- Validação automatizada de ARM, Node e Chromium; controle físico Windows/iPhone pendente. Não inclui correção térmica nem ampliação dos atributos comparados.


## 0.9.6 Beta — 25/10/2026

- Setas geradas dentro do jogo passam pelo mesmo tratamento do Códex: corrige Sistema → Status pulando o livro. Compatibilidade com os códigos nativos do direcional, sem alterar comandos do gameplay.
- Captura de entrada nos limites das rotinas ARM, com processamento depois da emulação e desvio explícito para chamadas internas de navegação; não cria um estado de página inválido.
- Direcionais originais selecionam, abrem e navegam no Códex. Soltar após arrastar o toque para dentro da janela libera o comando. Moldura da 0.9.5, teclado, touch, controle, nove abas, atributos, itens e saves preservados.
- Testes automatizados de ARM, Node e Chromium não substituem controle físico Windows/iPhone; sem mudança térmica.


## 0.9.5 Beta — 25/10/2026

- Livro do Códex inteiro dentro de uma moldura contínua: reaproveita fundo e fechamento da barra original, deslocando a borda para incluir a décima aba. Remove o recorte de seis colunas e a divisória antiga.
- Área clicável inclui toda a largura visível do livro. Preserva nove ícones/posições, navegação da 0.9.4 e pixels da mão animada original; não cria atalho F8 nem altera saves.
- Correção visual aplicada aos dois hosts PC e à web. Testes automatizados não substituem Windows/iPhone físicos. Sem mudança de desempenho/térmica.


## 0.9.4 Beta — 25/10/2026

- Corrigida a passagem da página Sistema para o Códex pela seta direita: o corpo da página retorna pelo cancelamento nativo antes de selecionar a aba do livro. Confirmar abre o Códex, sem entrar no submenu de configurações.
- Correção compartilhada pelos dois hosts PC e pela web; mantém acesso por clique/toque, teclado e controle. Não muda submenus profundos, itens, atributos ou saves; sem atalho F8.
- Testes ARM e navegador direcionados à navegação; controle emulado não comprova USB físico no iPhone. Não inclui correção térmica nem mudança de desempenho.


## 0.9.3 Beta — barra e acesso ao Códex — 24/10/2026

- Nove abas com espaçamento original de23px; barra ampliada para o décimo ícone inteiro, sem sobreposição de posições e hitboxes.
- Mão original preservada nas nove abas e desenhada por último no Códex, sem ficar encoberta pela barra.
- Acesso pela aba também ao navegar no inventário/equipamento; contexto original preservado ao voltar. Prévia antes de confirmar e clique/toque interativo nessa prévia.
- Removido o atalho dedicado F8 conforme pedido; abertura pela barra, com teclado/controle/touch preservados.
- Correção nos dois hosts e na web100%, que foi publicada. Saves/atributos/contadores, launchers na raiz e releases anteriores preservados. Testes físicos ainda necessários.

## 0.9.2 Beta — inicializadores Windows na raiz — 24/10/2026

- Novo **Destinia.exe** e **Destinia Zoom.exe** na raiz da pasta Windows extraída, sem necessidade de executar BAT para jogar. Ambos são aplicativos GUI Windows x64 e abrem os hosts nas subpastas existentes.
- Mantidos hosts originais, pasta game, saves/configurações e dependências MSYS2 UCRT64. Scripts BAT de suporte continuam disponíveis.
- Empacotador, teste do ZIP e workflow Windows passam a exigir quatro arquivos EXE (dois launchers e dois hosts).
- **Web não alterada**: segue a 0.9.1, revisão 5d05a13, com progresso não persistente. Bugs visuais de prévia do Códex/ponteiro continuam em PR separado, sem correção publicada nesta versão.
- Testes físicos Windows/iPhone ainda necessários; resultados de CI são verificados separadamente.

## 0.9.1 Beta — Códex integrado e filtros — 24/10/2026

- Códex como décima aba da barra original nos dois hosts PC e na web; moldura, fonte e ícones desenhados pelas rotinas ARM do jogo. Nove abas originais preservadas.
- Grade8×4, retratos frontais, contador por espécie, INFO/ação e páginas de drops/locais/variantes preservados; toque, teclado e controle. Cache evita redesenhar o menu ARM quando nada mudou.
- Web fixada em100%: seletor75/125 removido, preferência antiga ignorada sem apagar outros dados. Configuração PC independente.
- Filtros de equipamentos do contexto/bolsa por tipo/rank/faixas nível/preço e ordenação; identidade revalidada, retorno nativo, sem confirmação automática de equipar/vender. Painel de filtros ainda tem seu visual próprio.
- Não inclui armas/classes novas, favoritos/proteções/automações/reset, persistência web nem otimização térmica. Saves, modos, BAT pessoal e releases anteriores preservados. Teste físico Windows/iPhone permanece necessário.

## 0.9.1 — em desenvolvimento: equipamentos

- Painel interno de filtros nos dois hosts/web, por tipo/rank/faixas de nível e preço; ordenação original/tipo/rank/nível/preço e direção.
- Seleção por pool e conteúdo revalidado; visualização sem reordenar itens/saves. Retorno ao alvo via navegação e prévia nativas, sem equipar/vender automaticamente.
- Interface por toque/teclado/controle,75/100/125, listas vazias e paginação; não confirmar item fora da página nem substituir item removido por vizinho.
- Escopo: equipamentos do contexto/bolsa atual. Demais itens e confirmações usam a interface original. Release0.9 preservada; não conclui toda a0.9.1.

## Web — melhoria dos controles e tela cheia — 24/10/2026

- Uso de gamepad padrão oculta temporariamente botões do jogo e painel touch do site, preservando preferências, HUD/menus e teclado; desconexão restaura a interface touch.
- Botão Tela cheia com saída e tratamento de recusa; Ampliar tela quando a API não existe, sem prometer esconder barras do Safari. Sem alteração dos EXEs PC.

## 0.9 Beta — controles PC, fix1 — 24/10/2026

- Web: Gamepad API padrão com diagnóstico de detecção, ações/menu/pausa, liberação na desconexão e perda de foco; exigir neutro ao retornar. USB/Safari no iPhone físico ainda não validado.
- PC: L3+R3 alterna a opção existente dos controles virtuais e hitboxes durante gameplay em ambos hosts, uma vez até soltar ambos. Preferência por host preserva demais configurações; cliques isolados remapeados disparam ao soltar. HUD/menus/comandos físicos e opção web independentes.

## 0.9 Beta — 24/10/2026

- Português brasileiro passa a ser o padrão nos dois hosts e no navegador; inglês explícito é preservado. CLI aceita `--lang pt|en` sem gravar uma preferência implicitamente.
- README concentra instalação, estado atual e planejamento; histórico detalhado nas Releases.
- Removidos pacotes auxiliares de compilação/testes antigos; scripts ativos permanecem nos fontes e pacotes atuais. Web elimina cópias WIP já integradas.
- Comando remapeável `nomes_drops` (V quando livre) e botão web exibem nomes de drops ativos/visíveis nas cores originais, sem alterar distância de coleta, inventário ou chance de drop.
- FC nos status e comparador inicial com atributos nativos, projeção, buffs iguais e restauração do estado em memória; fórmula documentada emdocs/FC.md.
- Por decisão do autor, o restante do escopo migra para0.9.1: filtros, favoritos, automações, Códex organizado e reset por pergaminho vendido por10.000gold. Não implementados na0.9.

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
