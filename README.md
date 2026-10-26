# Destinia 0.11.0 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**[Baixar Destinia 0.11.0 Beta — Catálogo de Equipamentos, Baú Compartilhado e Bounties Offline](https://github.com/sugyua/destinia/releases/tag/v0.11.0-beta)** — ZIP Windows com executáveis standalone portáteis (16 DLLs inclusas), dois inicializadores GUI na raiz, os dois hosts, fontes e checksums. Dependências e dados originais são fornecidos separadamente; consulte **Dependências**. [Release anterior 0.10.3 Beta](https://github.com/sugyua/destinia/releases/tag/v0.10.3-beta).

[Instalação e dependências](INSTALL.md) · [Releases e histórico de mudanças](https://github.com/sugyua/destinia/releases) · [Teste web privado](https://destinia-web-experimental.leafyheron257.chatgpt.site).

**[Jogar a versão web no navegador](http://localhost:8000/)** — teste local na porta 8000 (ou na rede LAN em `http://192.168.1.11:8000/` ou Tailscale em `http://100.108.92.19:8000/`). A versão web foi atualizada com as 3 grandes novidades da 0.11.0 (Catálogo F4, Baú Compartilhado F5 e Bounties F8) e hotfix mobile com 16 FPS no Safari iOS. O deployment no site externo (`chatgpt.site`) permanece pendente de credenciais de publicação.

## Novidades da Versão 0.11.0 Beta — Catálogo de Armamentos, Baú Compartilhado e Contratos Offline (Bounties)

- **Catálogo Completo de Equipamentos e Complecionista (F4)**:
  - 48 equipamentos canônicos catalogados: 24 Armas (Espadas, Pistolas/Canhões e Adagas) e 24 Armaduras (Peitorais leves, médios e pesados), distribuídos em 8 patamares por classe (Guerreiro, Atirador e Ladino).
  - 5 patamares de raridade por equipamento (Comum, Mágico, Raro, Épico e Lendário), totalizando 240 entradas colecionáveis.
  - Alternância imediata de categoria entre Armas e Armaduras via tecla `T` ou toque no botão de categoria.
  - Varredura em tempo real do inventário em busca de novas descobertas, com cálculo proporcional de progresso e sinergia com o título `Complecionista` (+100% de atributos principais ao completar).
  - Persistência transacional e segura no sidecar `weapons-catalog-v1.txt` (PC) e `localStorage` (Web).
- **Baú Compartilhado entre Personagens / Shared Stash (F5 / B)**:
  - Capacidade ampliada para 48 slots divididos em 2 páginas de 24 itens, acessível a qualquer momento ou em cidades.
  - Permite transferir armamentos, elmos, armaduras, luvas, botas, acessórios e consumíveis entre todos os saves do jogador.
  - Preservação estrita da integridade: gravações atômicas em `shared-stash-v1.txt` com checksums e backups, sem tocar na estrutura binária dos arquivos `save*.dat` originais.
  - Interface intuitiva com destaque por cor de raridade, navegação por teclado e toque nas duas páginas.
- **Sistema de Bounties / Contratos Offline Repetíveis (F8 / C)**:
  - 3 contratos diários gerados por slot de personagem: Caça a Monstros Comuns (ex: Soldados Imperiais), Caça a Chefes (ex: Diablo) e Desafios de Caça Épica.
  - Contabilização automática e sem overhead de abates através do hook ARM nativo de derrota de monstros.
  - Resgate instantâneo de recompensas com premiações balanceadas em Ouro e Cash offline.
  - Interface com barra de progresso em tempo real, status de resgate e persistência em `bounties-v1.txt` (PC) e `localStorage` (Web).
- **Otimizações Críticas e Performance Web (Mobile / Safari iOS)**:
  - Resolução definitiva de gargalos em navegadores móveis: tabela de busca (LUT) RGB565 -> RGBA32 de 256 KB, acelerando o desenho de quadro para ~0.35ms (ganho de 7x de performance).
  - Pacing do Web Worker acionado diretamente pela janela principal (`nextFrame`), evitando o estrangulamento de segundo plano do Safari (que limitava a 3 FPS). Framerate cravado em 16 FPS.
  - Avanço automático suave pelas telas de apresentação (Everple / Gamevil / Logos) e suporte a toque para pular direto para o menu inicial.
  - 119 testes unitários automatizados passando com 100% de sucesso.

## Novidades da Versão 0.10.3 Beta — Dificuldades Canônicas e Modo Abissal

- **Sistema Canônico de Dificuldades (Normal, Difícil, Pesadelo, Inferno e Abissal)**:
  - Multiplicadores de HP, Dano recebido, XP de abate e Ouro gerado implementados via hooks ARM nos executáveis nativos `destinia_host.exe` e `destinia_host_zoom.exe`.
  - Normal (1.0x), Difícil (1.5x/1.3x), Pesadelo (2.5x/1.8x), Inferno (4.0x/2.5x) e Abissal (6.0x/4.0x).
  - **Modo Abissal com Zero Bônus de Itens**: Taxas, chances, qualidade e integridade do RNG de drops de itens permanecem rigorosamente idênticos aos da campanha original, funcionando como teste supremo de maestria (validado deterministicamente em 256 seeds).
  - Escala proporcional de vida sem empilhar em respawns/reloads e sem reviver monstros mortos.
  - **Congelamento de Tier na Morte (`deadTier`)**: Dificuldade congelada no instante do abate, impedindo exploits de XP/ouro ou obtenção indevida de liberações ao trocar de menu durante animações de morte.
- **Interface e Navegação**:
  - Menu Sistema inclui o botão **Mudar Dificuldade**, permitindo alternar apenas entre os níveis liberados para o personagem.
  - Indicador da dificuldade atual exibido ao lado da classe na tela de Seleção de Personagem (apenas em slots com save).
  - Desbloqueio progressivo: derrotar o Boss Final Diablo (`species 108`) libera o próximo nível de dificuldade no slot correspondente.
- **Persistência Transacional Segura**:
  - Saves binários originais `save*.dat` 100% preservados. Desbloqueios e configurações gravados atomicamente no sidecar `difficulty-unlocks-v1.txt`.
  - Homologado em testes nativos nos dois hosts e auditado pelo Sentinel (Score 10/10).

## Novidades da Versão 0.11.0 — Expansão Completa de Conteúdo, Suporte Integral a Controles USB e QoL

- **Suporte Integral a Controles USB / Gamepads (PC e Web)**:
  - Todas as funcionalidades, telas secundárias e recursos de QoL agora possuem integração nativa e completa com controles USB/gamepads convencionais (Xbox, PlayStation, genéricos D-Input/X-Input):
    - **Guia Visual no Rodapé (OSD)**: Segurar o botão `SELECT` (ou `BACK`) exibe em tempo real uma faixa translúcida no estilo Gamevil indicando todos os atalhos disponíveis.
    - **`R3` (Clique do Analógico Direito)**: Alterna ativação imediata da **Auto-Poção** inteligente.
    - **`L3` (Clique do Analógico Esquerdo)**: Liga/desliga o **Rastreador de Missões (Quest Tracker)** no HUD.
    - **`SELECT + X`**: Abre o **Catálogo Completo de Equipamentos**.
    - **`SELECT + Y`**: Abre o **Baú Compartilhado entre Personagens (Shared Stash)**.
    - **`SELECT + B`**: Abre o **Mural de Contratos / Bounties Diárias**.
    - **`SELECT + A`**: Alterna o **Rastreador de Missões**.
    - **`SELECT + D-Pad Cima`**: Abre o menu do **Mago dos Bosses** (Boss Rush / Torre Abissal / Mascotes / Santuário).
    - **`SELECT + D-Pad Baixo`**: Alterna a **Auto-Poção**.
    - **Toque rápido em `SELECT`**: Executa o Salvamento Rápido nativo (`touch(0, 0, 3)`).
  - **Navegação Integral por D-Pad e Botões nos Menus**:
    - *Catálogo*: Esquerda/Direita para mudar de página, Cima/Baixo/Y para alternar abas de classe, X/A para alternar entre Armas e Armaduras, B/Back para fechar.
    - *Baú Compartilhado*: D-Pad para percorrer os 24 slots da grade, Botão A para transferir/retirar item, X/Y/LB/RB para trocar entre as Páginas 1 e 2, B/Back para fechar.
    - *Mural de Contratos*: D-Pad Cima/Baixo para selecionar contratos, Botão A para resgatar recompensas, B/Back para fechar.
    - *Mago dos Bosses*: D-Pad Cima/Baixo para navegar, LB/RB para trocar de aba, Botão A para confirmar desafio, B/Back para fechar.

- **Catálogo Completo de Equipamentos e Título Complecionista (F4 / Ctrl+W / SELECT+X)**:
  - Expansão do catálogo para 48 equipamentos canônicos (24 armas e 24 armaduras) em 8 patamares por classe (Guerreiro, Atirador, Ladino).
  - Rastreamento de 5 patamares de raridade por item (Comum, Mágico, Raro, Épico e Lendário) em um total de 240 entradas colecionáveis.
  - Sinergia direta com o título `Complecionista`: desbloqueio concede buff épico de +100% em todos os status principais (ATK, DEF, PV, PM).
  - Persistência atômica no sidecar `weapons-catalog-v1.txt` (PC) e `localStorage` (Web).

- **Baú Compartilhado entre Personagens / Shared Stash (F5 / B / SELECT+Y)**:
  - 48 slots divididos em 2 páginas de 24 slots, permitindo compartilhar armas, armaduras e consumíveis entre todos os saves locais com isolamento absoluto em `shared-stash-v1.txt`.

- **Mural de Bounties / Contratos Offline Repetíveis (F8 / C / SELECT+B)**:
  - 3 contratos diários gerados por slot de personagem com contagem nativa via hooks ARM e recompensas balanceadas em Ouro e Cash offline.

- **Rastreador de Missões no HUD / Quest Tracker (Q / L3 / SELECT+A)**:
  - Widget retrátil no canto superior direito do HUD com moldura Gamevil, pergaminho dourado pixel-art e atualização dinâmica da missão ativa e contagem de alvos.

- **Arena Gauntlet / Boss Rush de Sobrevivência (Mago dos Bosses)**:
  - 11ª opção no Mago dos Bosses (`species 999`), desafiando o jogador a derrotar em sequência todos os 10 chefes de Destinia por 10.000 G.

- **Auto-Potion QoL / Consumo Inteligente (H / R3 / SELECT+D-Pad Baixo)**:
  - Monitoramento contínuo de PV (< 35%) e PM (< 25%), consumindo automaticamente com intervalo seguro de cooldown de 60 frames.

- **Reset de Atributos & Habilidades Offline (Santuário do Poder)**:
  - Devolução integral de pontos acumulados por 5.000 G de ouro no Mago dos Bosses e recálculo seguro no motor ARM nativo (`GameUI_calcHeroStat`).

- **Mascotes & Companheiros Canônicos + Torre Abissal (50 Andares)**:
  - 4 mascotes canônicos (Lobo Glacial, Fênix Ígnea, Golem de Pedra, Mini-Diablo) com buffs passivos e sprite com flutuação e sombra suave desenhado ao lado do herói.
  - Torre Abissal de 50 andares com chefes a cada 5 andares e recompensas escalonadas salvas em `abyss-v1.txt`.

- **Ciclo Novo Jogo+ (NG+1 a NG+5)**:
  - Reinício de campanha preservando atributos, nível, mascotes e baú compartilhado com escalonamento de dificuldade e recompensas.

- **Otimização Crítica e Performance Web (Mobile / Safari iOS)**:
  - Framebuffer acelerado via LUT RGB565 -> RGBA32 de 256 KB (~0.35ms de render), 16 FPS contínuos e auto-avanço das telas de logos.

## Novidades da Versão 0.10.2 — Mago dos Bosses Completo nas 5 Cidades e Teleporte Ativo

- **Mago dos Bosses em Todas as 5 Cidades (Safe Zones com Lojas)**: Presente em todos os mapas que cumprem os requisitos estritos de Safe Zone (bloqueio de ataque/dash) e presença simultânea do Ferreiro e do Vendedor de Poções: Acampamento da Resistência (61), Bergheim (62), Acampamento dos Refugiados (63), Fermion (64) e Lagrant (65).
- **Colisão Física e Hitbox Sólida**: Bloqueio sólido de 16x16 integrado à grade nativa do mapa (`curMapData + 0x68`). O jogador colide como em um obstáculo do cenário e não mais atravessa o corpo do Mago.
- **Placa de Nome Nativa por Proximidade**: O letreiro "MAGO DOS BOSSES" só aparece quando o herói está a menos de 48 pixels de distância, renderizado com fundo escuro semitransparente e cantos arredondados no padrão da UI original.
- **Gatilho de Interação e Menu de Teleporte Ativo**: Pressionar Ação adjacente ao NPC abre o menu interativo de seleção de chefes do Boss Rush (`BossRushUI`), permitindo teleporte calibrado em ouro para os 10 chefes de espécie 100 a 109.

## Novidades da Versão 0.10.0 / 0.10.1 — Nível 999, Sets 2–6 Peças, Códex Completo e Fix Web HTTP/Wi-Fi

- **Progressão NV 999 e 14 Patentes**: Núcleo canônico em [`common/progression.h`](common/progression.h) e [`web/dist/progression.mjs`](web/dist/progression.mjs) com aritmética segura em 64 bits (`uint64_t`/`BigInt`), sem overflow ARM. Curva original estendida até o cap de 2.035.180.079.455 XP.
- **Sistema de Sets de Equipamento (2 a 6 Peças)**: Bônus progressivos cumulativos para conjuntos temáticos (2, 3, 4, 5 e 6 peças equipadas). Sets concentrados (100% das peças em 1 Boss) e distribuídos (peças divididas entre monstros elites e diferentes chefes).
- **Economia de Cash por Missões Épicas**: Remoção integral de Cash offline passivo. Cash concedido exclusivamente como recompensa de missões de alto risco (marcos de abate de chefes e conjunto de 6 peças), calibrado com a loja original (10 a 25 Cash).
- **Mago dos Bosses (Boss Rush)**: NPC teletransportador em Lagrant com paleta azul/branco/sombra (`mon_6_0`), tabela canônica de teleporte calibrada em ouro (500 a 50.000 de ouro) para os 10 chefes exclusivos (`species 100..109`).
- **Sistema de Títulos & Título Complecionista**: Título concedido ao atingir 100% de descobertas no Códex (73 monstros), conferindo buff épico de +100% em todos os status principais (ATK, DEF, PV, PM).
- **Centralização da Barra de Menu (10 Ícones)**: Recálculo dos offsets em X para acomodar a inclusão do 10º ícone (Códex) com alinhamento visual e hitboxes perfeitas em 400x240 e 800x480.
- **Correção Crítica de Viewport Web 800x480**: Eliminação de duplicação do mapa e recuperação de 7 FPS para os 16 FPS originais via centralização de câmera (`Game+18552/18554`), limpeza de bordas com cinza médio (`0x8410`) e descarte imediato de tiles fora do mapa (`0x4000`/`fastReject`).
- **Saves Nativos 100% Intocados**: Todos os dados estendidos residem em sidecars versionados com CAS (`progression-v1.txt`, `economy-v1.txt`, `titles-v1.txt`).

### Roadmap Oficial do Projeto (Cronograma de Versões)

#### Versão 0.10.3 (Próxima — UI de Dificuldade & Polimento da Meta 0.10)

1. **Troca de Dificuldade no Menu Sistema**:
   - Novo botão "Mudar Dificuldade" no menu Sistema.
   - Lista das 5 dificuldades (Normal, Difícil, Pesadelo, Inferno, Abissal).
   - Bloqueio progressivo: apenas libera a próxima dificuldade após derrotar o Boss Final da dificuldade atual. Opções bloqueadas em tom escuro/cinza.
2. **Indicador de Dificuldade na Seleção de Personagem**:
   - Exibição da dificuldade atual do save ao lado do nome da classe (Guerreiro, Atirador, Ladrão) na tela de seleção.
   - Fonte branca nativa da engine; exibido exclusivamente em slots com save existente.
3. **Restrição de Recursos**: 100% de reaproveitamento de molduras, fontes e bitmaps originais do jogo.

#### Versão 0.11.0 (Entregue — Expansão Completa de Conteúdo, QoL & Endgame)
- **Catálogo Completo de Equipamentos e Complecionista (F4 / Ctrl+W)**: 48 equipamentos canônicos (24 armas e 24 armaduras em 8 patamares por classe), 240 entradas colecionáveis por raridade, alternância imediata de categoria (`T`) e sinergia com o título Complecionista (+100% atributos principais).
- **Baú Compartilhado entre Personagens / Shared Stash (F5 / B)**: 48 slots (2 páginas de 24 itens) com persistência isolada em `shared-stash-v1.txt`.
- **Mural de Bounties / Contratos Offline Repetíveis (F8 / C)**: 3 contratos diários por slot de personagem, contagem nativa via hooks ARM e recompensas em Ouro e Cash.
- **Quest Tracker HUD Retrátil (Q / Toque)**: Rastreador no canto superior direito do HUD com moldura Gamevil, pergaminho pixel-art e alternância rápida via tecla `Q`.
- **Boss Rush Gauntlet (Arena de Sobrevivência 10 Chefes)**: 11ª opção no Mago dos Bosses (`species 999`), combate contínuo contra todos os 10 chefes de Destinia.
- **Auto-Potion QoL / Consumo Inteligente (H)**: Monitoramento contínuo de PV (< 35%) e PM (< 25%), consumo com cooldown de 60 frames e atalho `H` para ativar/desativar.
- **Reset de Atributos e Habilidades Offline (Santuário do Poder)**: Devolução integral de pontos acumulados por 5.000 G no Mago dos Bosses e recálculo ARM (`GameUI_calcHeroStat`).
- **Mascotes & Companheiros Offline (Pets System)**: 4 mascotes (Lobo Glacial, Fênix Ígnea, Golem de Pedra, Mini-Diablo) com buffs passivos, níveis até Lv50 e mascote flutuante renderizado ao lado do herói.
- **Ciclo Novo Jogo+ (NG+1 a NG+5)**: Reinício da campanha preservando nível, equipamentos, baú, títulos e mascotes, com escalonamento de dificuldade e recompensas.
- **Torre Abissal / Masmorra Infinita (50 Andares)**: 50 andares de desafios progressivos no Mago dos Bosses com chefes a cada 5 andares.
- **Otimização Crítica Web (Mobile / Safari iOS)**: LUT RGB565 -> RGBA32 (aceleração de 7x), pacing por `nextFrame` a 16 FPS contínuos e 126 testes aprovados.

#### Versão 0.12.0 (Endgame Extremo — O Modo Abissal Supremo)
- **Modo Abissal Hardcore**:
  - Desafio supremo para heróis no Nível 999 equipados com sets de 6 peças.
  - **100% Aura de Elite:** Todos os monstros do mapa nascem compulsoriamente como monstros Elites.
  - **Escalonamento Brutal:** 5× HP base em relação ao modo normal e aumento substancial da agressividade.
  - **Zero Bônus de Drop:** Tabela de recompensas idêntica às dificuldades padrão (sem facilitação ou aumento de ganhos), servindo puramente como o teste supremo de combate, reflexos e teoria de builds.
- **Título Lendário "Senhor do Abismo"**:
  - Título cosmético e honorífico exclusivo para quem derrotar o Boss Final no Modo Abissal.







## META 0.10 — Frente 1 em preparação

Núcleo canônico disponível em [`common/progression.h`](common/progression.h) e [`web/dist/progression.mjs`](web/dist/progression.mjs): curva de XP até NV999 em u64/BigInt, 14 patentes, status parametrizados, cinco dificuldades/NG+ configurável e matemática de bônus cumulativos de sets para 2–6 peças, com composição e verificações de overflow/conversões ARM. Persistência separada em `progression-v1.txt`, com validação, lock e compare-and-swap; **nenhum `save*.dat` é alterado pelo módulo**. [Contrato/API, limites e testes](docs/PROGRESSION_CORE.md).

**Barra e Mago Boss Rush:** dez ícones centralizados com hitboxes correspondentes em400×240/800×480; NPC sintético em Lagrant, arte original mon6_0 recolorida em memória e evento diferido para a UI do OpenCodes. [Contrato e testes](docs/BOSS_MAGE.md). Correspondência de Icarus e ligação do opener real da UI ficam explícitas nesse contrato; nenhum save/asset original foi modificado.

Esta frente permanece desacoplada de ARM/HUD/combate e **não habilita NV999 no jogo nesta etapa**. `VERSION` continua 0.9.9 até a integração pela orquestração; não há release 0.10 publicada por esta branch. **Web:** módulo atualizado e testado localmente, sem deployment desta revisão. URL existente: https://destinia-web-experimental.leafyheron257.chatgpt.site/; nenhuma versão 0.10 nesse endereço foi confirmada por esta entrega.

## Novidades da Versão 0.9.9 — Filtro Visual, Autoloot e QoL de Inventário

O **filtro visual de drops** destaca equipamentos com as cores do rank, materiais em ciano, ouro em amarelo e itens de missão em laranja. Categorias e rank mínimo de equipamento podem ser ocultados **apenas no desenho**, sem apagar drops ou mudar a coleta manual. A coleta automática de **ouro e materiais de fabricação** tem opções independentes do filtro visual: ouro ativo por padrão, materiais desligados por padrão. Mantém o alcance original (até 19 pixels por eixo), usa as verificações nativas de capacidade e não coleta equipamentos, poções, receitas ou itens de missão. A mochila cheia conserva o material no chão; tentativas malsucedidas têm intervalo para evitar repetição de mensagens.

**Atalhos atuais:** F7 ou P protegem/desprotegem diretamente o equipamento do cursor, sem abrir FILTROS. F6, clique em FILTROS ou X/ação extra abrem o painel. Item/Comparar continuam por clique/controle (RS), sem ocupar F6. Uma bolinha verde no canto superior direito do ícone indica ganho confirmado de FC para aquele slot, respeitando os requisitos do comparador e sem equipar.

**PC:** com o jogo fechado, mescle as chaves abaixo no `config.ini` existente em `%LOCALAPPDATA%\Destinia` (ou o arquivo passado em `--settings`), preservando `save_dir` e as demais preferências. Reinicie para aplicar. Os dois hosts compartilham estas opções; não é preciso alterar o BAT pessoal.

```ini
drops.filtro_visual=1
drops.categorias=31
drops.rank_minimo=0
coleta.ouro=1
coleta.materiais=0
coleta.raio=19
```

`drops.categorias` soma **1 equipamentos + 2 materiais + 4 ouro + 8 missão + 16 outros**; 31 mostra todos e 0 oculta todos quando o filtro está ativo. `drops.rank_minimo` aceita 0–4 e só filtra equipamentos. `coleta.raio` aceita 1–19, nunca amplia a distância nativa. O padrão sem configuração é ouro=1 e materiais=0; opções explícitas anteriores são respeitadas. Use 0 para desativar o autoloot de ouro ou 1 para ativar materiais. Opções são gravadas fora dos `save*.dat`; itens coletados entram na memória do jogo pelo coletor original e serão salvos pelo fluxo normal quando o jogador salvar, sem mudar o formato.

**Web:** abra **Itens no chão e coleta** para ajustar as mesmas opções; as preferências ficam neste navegador, separadas dos saves/Google Drive. Pausa, menus/transições e perda de foco suspendem a assistência. **Status web 0.9.9:** código web atualizado e testado localmente (inclusive ponte worker/localStorage para F7 e bolinha verde de FC). URL existente: [teste web privado](https://destinia-web-experimental.leafyheron257.chatgpt.site/). Deployment real no site requer ferramenta externa de deploy configurada com credenciais do serviço de hospedagem. [Testes, contrato ARM e limites](docs/LOOT_QOL.md).

## 0.9.8 — Proteção de equipamentos, separação de chefes no Códex e Web Destinia Zoom

- **Favoritar/Proteger equipamentos (Sidecar)**: Proteção persistente de itens por personagem/pool/fingerprint gravada em `inventory-protect-v1.txt` (PC) e `localStorage` (Web). Itens protegidos recebem o indicador visual `[P]` e o botão alternador `PROT/DESPROT` (tecla F7/atalho extra). Venda rápida ou acidental bloqueada nativamente no estado `0x91040004` com guarda ARM de confirmação. Os arquivos `save*.dat` originais permanecem 100% intocados e compatíveis com a sincronização do Google Drive. [Evidências e limites](docs/INVENTORY_PROTECTION.md).
- **Separação de Monstros Comuns vs Chefes/Bosses no Códex**: Classificação nativa da engine (`species >= 100` para Chefes e `species < 100` para Monstros Comuns). Três abas de categoria implementadas no cabeçalho do Códex: `[TODOS]`, `[COMUNS]` e `[CHEFES]`. Grade de 8x4 e paginação recalculadas por categoria, com destaque visual diferenciado para chefes e navegação completa por toque, teclado e controle.
- **Unificação da Versão Web com o Destinia Zoom**: Versão Web migrada para a base e layout do Destinia Zoom (`host_zoom`, 800x480 fixo), eliminando seletores de zoom dinâmico e unificando proporções e renderização entre PC e Web.
- **Testes Automatizados**: Suíte completa de testes no Node.js (testes de runtime, inventário, Códex e novos casos dedicados) e casos de teste nativos em C++ aprovados. Validação física Windows/iPhone pendente.

**Status web 0.9.8:** código web atualizado e testado localmente. URL existente: [teste web privado](https://destinia-web-experimental.leafyheron257.chatgpt.site/). Deployment real no site requer ferramenta externa de deploy configurada com credenciais do serviço de hospedagem.

## Laboratório local de PvP e Leilão

Disponível um [servidor de simulação gratuito e guia de túnel](tools/network-mock/GUIA_TESTE.md), separado do jogo. **Não habilita o Multi Mode original.** A causa interna do `Codigo de Erro = 15` no Multi Mode (`MMain_run` em `0x104b6` após timeout de 150 quadros em `Comm_flush(cmd=0)`) e o contrato binário `Comm_*` / `Menu_netProc` foram mapeados em [CONTRATO_MULTI_MODE_ARM.md](docs/CONTRATO_MULTI_MODE_ARM.md); a integração do adaptador com o servidor continua pendente. [Estado técnico e próximos passos](docs/MULTI_MODE_REDE.md). Versão e releases do jogo preservadas.

## 0.9.7 — FC e comparador com a interface original

O **FC na aba Status** usa a fonte nativa, sem a faixa azul. No inventário, **Item/Comparar alternam dentro da própria caixa de descrição**, reaproveitando moldura, fonte e cores do jogo. Item preserva a descrição original; Comparar mostra FC atual → previsto e diferenças de ATK/DEF/PV/PM/crítico, com compatibilidade de armas por classe alinhada à tabela nativa `{0, 2, 1}` (`0x40f7c`). Toque nos rótulos, use **F6 livre** ou o **analógico direito** no contexto de equipamentos. Não equipa automaticamente nem altera a fórmula, itens, atributos, gold ou saves.

**Web 0.9.7 publicada**, revisão `3c41f07`, no mesmo link privado e somente100%. Recarregar descarta o progresso temporário.73 testes Node, testes ARM dos dois hosts,7 testes de pacote e Chromium aprovados; validação física Windows/iPhone pendente. [Notas](release/NOTAS_0.9.7.md) · [Uso e limites](docs/FC.md).

## 0.9.6 — setas originais também selecionam o Códex

Corrige o caminho das **setas desenhadas dentro do jogo**, que ainda voltava de Sistema para Status, pulando o livro. Agora elas selecionam a aba, abrem o Códex e navegam na grade; setas do site, teclado e controle continuam disponíveis. Soltar um toque depois de arrastá-lo para dentro da janela libera o comando. Moldura da 0.9.5, nove abas, itens, atributos, gold e saves preservados; sem F8.

**Web0.9.6 publicada anteriormente**, revisão `69e7a97`, no mesmo link privado/100%. Recarregar descarta o progresso temporário.72Node, ARM dos dois hosts,7pacotes e Chromium com setas internas/site/teclado/controle emulado aprovados; físico pendente. [Notas](release/NOTAS_0.9.6.md).

## 0.9.5 — livro inteiro dentro da barra

O Códex agora usa a **mesma moldura contínua das nove abas**, reaproveitando o fundo e o fechamento originais. Corrige o corte da parte esquerda do livro e remove a borda que o separava do menu. Mantém posições dos nove ícones e a mão animada, com clique/toque cobrindo o livro inteiro. Setas/controle: Sistema → direita → Ação/A; sem F8. Não altera itens, atributos ou saves.

**Web0.9.5 publicada anteriormente**, revisão `c859306`, no mesmo link privado/100%. Recarregar descarta o progresso temporário. Testes de raster e hitbox,71Node, ARM dos dois hosts e Chromium com setas/controle emulado aprovados; teste físico pendente. [Notas](release/NOTAS_0.9.5.md).

## 0.9.4 — selecionar o Códex pela página Sistema

Com Sistema aberto, pressione **direita** para selecionar a aba do livro e **Ação/A/Enter** para abrir o Códex. Corrige o caminho que antes ficava no corpo de Sistema e entrava nas configurações ao confirmar. Aplicado aos dois hosts e à web; nove abas anteriores, clique/toque, itens, atributos e saves preservados. Sem F8 e sem mudança de desempenho.

**Web 0.9.4 publicada anteriormente**, revisão `e39b4ff`, mesmo link e acesso privado, somente100%. Recarregar perde o progresso temporário desta sessão. Testes:70Node, ARM dos dois hosts e Chromium com setas do site, teclado e controle emulado aprovados; controle físico Windows/iPhone pendente. [Notas](release/NOTAS_0.9.4.md).

## 0.9.3 — barra e acesso ao Códex corrigidos

A barra mantém o espaçamento original das nove abas e ganha uma posição para o Códex, sem comprimir ícones. A mão nativa aparece inteira por cima do menu. Clique na última aba para abrir o Códex, inclusive enquanto navega no inventário/equipamento. A seleção por teclado/controle mostra a prévia; clicar nela também funciona. **Sem atalho dedicado F8.** Contadores, seleção, atributos e saves preservados.

**Web0.9.3 publicada anteriormente**, revisão`0da736c`, no link acima, somente100% e com o mesmo acesso privado. [Notas e validação](release/NOTAS_0.9.3.md). O ZIP Windows preserva `Destinia.exe` e `Destinia Zoom.exe` na raiz. A versão0.9.2 e releases anteriores permanecem disponíveis.

## 0.9.2 — abra direto pelo .exe da raiz

O pacote Windows passa a incluir **`Destinia.exe`** (normal) e **`Destinia Zoom.exe`** (zoom), ambos na **raiz da pasta extraída**, ao lado do `VERSION`. Dê duplo clique no executável desejado, sem abrir um `.bat`. Os dois inicializadores localizam os hosts existentes em `host/build/` e `host_zoom/build/`, configuram o caminho de DLLs MSYS2 UCRT64, conferem a versão instalada e iniciam no diretório correto. **Não mova os hosts** das subpastas. Os `.bat` continuam disponíveis para diagnóstico e configurações.

O ZIP Windows da Release contém os quatro executáveis: os dois lançadores na raiz e os dois hosts nas respectivas subpastas. O ZIP de fontes contém `launcher/main.go` e os scripts de compilação, **não** os executáveis pré-compilados. Não são incluídos dados do jogo ou DLLs externas; veja [INSTALAÇÃO](INSTALL.md).

**Web:** atualizada para **0.9.3 Beta**, revisão `0da736c`, [site privado](https://destinia-web-experimental.leafyheron257.chatgpt.site/). Esta atualização `0.9.2` altera apenas o modo de iniciar a versão Windows e **não foi implantada na web**. O progresso web continua temporário.

## 0.9.1 — Códex no menu e filtros de equipamentos

O **Códex ocupa a décima aba da barra original**, usando a moldura, fonte e ícones do jogo, nos dois hosts e na web. Abra Menu e toque no último ícone dourado (pergaminho); selecione um monstro e use **INFO/ação**. SAIR/Voltar retorna à grade/menu, e as outras nove abas continuam disponíveis. Descobertas, retratos frontais, mortes e detalhes por categoria preservados. **Na web, somente100%**, sem seletor75/125 ou reaplicar essa preferência antiga; escalas do PC permanecem independentes. [Uso e limites](docs/CODEX_NATIVO_0.9.1.md).

Painel **dentro do jogo**, na aba de equipamentos, mochila e venda do vendedor local. Abra **FILTROS**, **F6** livre ou **X/ação extra** do controle. Filtre por tipo/rank/faixas de nível e preço; ordene por tipo/rank/nível/preço, crescente ou decrescente. Toque ou navegue com as setas; escolha **VOLTAR AO ITEM SELECIONADO** e confirme equipar/vender na tela original. A visualização não reordena os itens e não escolhe outro automaticamente se o selecionado desaparecer.

**Escopo dos filtros: equipamentos do contexto/bolsa atual.** Consumíveis e itens de missão permanecem na interface original. O ZIP0.9.1 inclui os filtros, que ainda usam seu painel próprio; esta revisão visual muda o Códex. [Uso, controles e limites](docs/INVENTARIO_0.9.1.md). Favoritos, proteções, assistência e reset continuam pendentes; esta release não os declara entregues.

## Novidades da 0.9

- **FC nos status** e comparador de equipamentos próprios no inventário: atual → previsto, ganho/perda e diferenças de ATK/DEF/PV/PM máximos/crítico. Inclui buffs ativos; não diminui ao perder vida atual. [Fórmula e limites](docs/FC.md).
- **PT-BR padrão**; inglês em **Configurar Destinia → 9 — Idioma**, no seletor web ou `--lang en`. Tradução parcial, escolha explícita preservada.
- **Nomes de drops:** segureV (remapeável como `nomes_drops`); web também tem botão. Não altera alcance de coleta ou os itens.
- README enxuto e redundâncias removidas, preservando saves, dados necessários, testes e releases anteriores.

[Notas completas e instalação](docs/RELEASE_0.9.md). As funcionalidades ainda não concluídas estão na **0.9.1**, abaixo; não acompanham a 0.9.

## Correção 0.9 — controles PC (fix1)

- **Web já disponível:** ao usar gamepad reconhecido, oculta automaticamente os botões dentro do jogo e o painel touch do site; ao desconectar, restaura o painel e a preferência anterior, mantendo HUD/menus e teclado. Botão **Tela cheia**; em navegadores sem suporte, **Ampliar tela**, com possíveis barras do Safari. Controle reconhecido pela Gamepad API com mapeamento padrão; diagnóstico no rodapé. Direcional/analógico esquerdo move, A ação, B voltar, X extra, Y menu, ombros/gatilhos ou analógico direito habilidades, Start/Options pausa. Solte os comandos ao reconectar/voltar à aba. Teclado e touch preservados. **USB no iPhone depende do controle/adaptador/Safari e ainda exige teste físico.**
- **PC no ZIP atualizado:** L3 + R3 mostra/oculta controles virtuais e suas áreas de clique durante gameplay, nos dois hosts. Solte ambos para alternar novamente; HUD, menus e entradas físicas permanecem. Reutiliza a opção6 de **Configurar Destinia**, salva separadamente por host e não muda a web. Remapeamentos isolados de L3/R3 disparam ao soltar, para evitar conflito com a combinação. `--no-save-preferences` mantém a alteração só na sessão.
- Baixe o ZIP **windows-x64** da Release corretiva para jogar sem compilar. A Release0.9 original permanece preservada. Histórico da correção preservado; filtros de equipamentos chegam na0.9.1.

**[Prévia técnica da 0.9.1 — fontes e testes](https://github.com/sugyua/destinia/releases/tag/v0.9.1-dev1)**: checkpoint de desenvolvimento, sem novos EXEs ou interface dos filtros. Para jogar no PC, use o ZIP0.9.1 Windows indicado no início.

## Releases em ordem de lançamento

Da mais recente para a mais antiga. As releases anteriores e seus downloads são preservados.

| Release | Lançamento (Brasília) |
| --- | --- |
| [0.9.9 Beta — QoL, Autoloot, Indicador FC e Atalhos](https://github.com/sugyua/destinia/releases/tag/v0.9.9-beta) | 26/10/2026 |
| [0.9.8 Beta — Proteção de equipamentos e chefes no Códex](https://github.com/sugyua/destinia/releases/tag/v0.9.8-beta) | 26/10/2026 |
| [0.9.7 Beta — FC e comparador nativos](https://github.com/sugyua/destinia/releases/tag/v0.9.7-beta) | 25/10/2026 |
| [0.9.6 Beta — setas nativas do Códex](https://github.com/sugyua/destinia/releases/tag/v0.9.6-beta) | 25/10/2026 |
| [0.9.5 Beta — moldura integrada do Códex](https://github.com/sugyua/destinia/releases/tag/v0.9.5-beta) | 25/10/2026 |
| [0.9.4 Beta — navegação do Códex](https://github.com/sugyua/destinia/releases/tag/v0.9.4-beta) | 25/10/2026 |
| [0.9.3 Beta — barra do Códex corrigida](https://github.com/sugyua/destinia/releases/tag/v0.9.3-beta) | 24/10/2026 |
| [0.9.1 Beta — Códex integrado e filtros](https://github.com/sugyua/destinia/releases/tag/v0.9.1-beta) | 24/10/2026 |
| [0.9.1 dev1 — prévia técnica, somente fontes](https://github.com/sugyua/destinia/releases/tag/v0.9.1-dev1) | 24/10/2026 |
| [0.9 Beta — controles PC, fix1](https://github.com/sugyua/destinia/releases/tag/v0.9-controles-fix1) | 24/10/2026 |
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

### Pendências após este lote da 0.9.1 — complementos de inventário, coleta e assistência opcional

Reduzir tarefas repetitivas e tornar os equipamentos mais fáceis de entender e proteger.

- [ ] **13. Complementar o comparador:** a 0.9 entrega FC/ATK/DEF/PV/PM máximos/crítico; ampliar o detalhamento dos outros atributos e contextos de itens.
- [ ] **14. Organização e filtros do inventário:** primeiro lote de equipamentos implementado em PC/web, com filtros/ordenação, navegação nativa e proteção da seleção. Outros tipos continuam na interface original; validação física pendente. [Uso e limites](docs/INVENTARIO_0.9.1.md).
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

## Política de atualização e distribuição

Quando uma correção ou funcionalidade do Destinia estiver **implementada e validada**, ela deverá ser comunicada no README e no CHANGELOG, registrada na [central das IAs](https://github.com/sugyua/ai-progress-shared/issues/4) e publicada como **nova versão em [GitHub Releases](https://github.com/sugyua/destinia/releases)**, com arquivos Windows pertinentes, fontes e checksums. Código experimental em branch ou PR **não é** uma versão jogável publicada. As Releases anteriores permanecem disponíveis; testes automatizados e testes reais são identificados separadamente.

**Versão web local:** disponível em `http://localhost:8000/` e na LAN em `http://192.168.1.11:8000/` (revisão `c850181`, com 5 Dificuldades, Hotfix Abissal e Catálogo de Armas F4). O deployment no domínio externo [chatgpt.site](https://destinia-web-experimental.leafyheron257.chatgpt.site/) permanece na revisão estável anterior por ausência de ferramenta remota de deployment configurada. O progresso do personagem na web ainda não é persistente após recarregar/fechar. A cada mudança futura na web, atualizar aqui a revisão/deployment e as novidades **somente depois da publicação confirmada**; se a versão web estiver pendente, informar explicitamente.

**Correção visual em preparação:** a prévia da décima aba e a mão apontadora do menu estão em [PR #3 (rascunho)](https://github.com/sugyua/destinia/pull/3), **não incluído na 0.9.1 Beta publicada** e não implantado na web. Precisa de testes e release própria antes de ser anunciado como corrigido.

## Publicação

Pacotes oficiais e histórico detalhado ficam em [Releases](https://github.com/sugyua/destinia/releases). A `main` contém os lotes de desenvolvimento indicados acima; a versão0.9 tem o escopo das notas de release e os complementos foram transferidos à 0.9.1 por decisão do autor. O autor confirmou testes da base no PC/iPhone; isso não comprova campanha completa ou validação manual de cada mudança nova.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).


### Sistemas de Loot e Economia (Meta 0.10 — Frente 2)

- **Drops Exclusivos de Boss:** 10 itens lendários exclusivos para cada chefe (species >= 100), integrados aos detalhes do Códex e registrados em sidecar.
- **Sets de Equipamento Expandidos:** 5 conjuntos temáticos com bônus progressivos cumulativos para **2, 3, 4, 5 e 6 peças equipadas** (2: ATK/DEF, 3: DEF/HP, 4: HP/CritDmg/Resistências, 5: Crítico/Evasão/Atributos, 6: Bônus Supremo de Set).
- **Origens de Drops:** Categorização em **Sets Concentrados** (ex: Set do Dragão em Diablo/Lava Guardian e Set do Guardião no Dungeon Guardian) e **Sets Distribuídos** (peças espalhadas entre chefes regionais e monstros elites do mapa).
- **Economia de Cash e Missões Épicas:** Ganho automático de 1 Cash a cada 30 minutos offline (teto de 24h = 48 Cash) e recompensas de Cash por Missões Épicas de marcos de chefes e conjunto completo (10, 25, 50 e 100 boss kills, set de 6 peças), calibradas com a loja original (item_en.gdt ~20 Cash por item premium).
- **Saves:** Saves originais save*.dat 100% intocados (persistência atômica segura no sidecar DESTINIA_ECONOMY_2 / economy-v1.txt). Detalhes em [LOOT_ECONOMY.md](docs/LOOT_ECONOMY.md).
