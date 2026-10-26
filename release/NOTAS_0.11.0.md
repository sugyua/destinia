# Notas de Lançamento — Destinia 0.11.0 Beta

> **AVISO: Versão Beta experimental. Faça backup dos seus saves antes de atualizar.**

---

## Destaques da Versão 0.11.0 Beta — Os 3 Pilares

### 1. Catálogo Completo de Equipamentos & Complecionista (F4)
* **48 Equipamentos Canônicos:** 24 Armas e 24 Armaduras (Peitorais) rigorosamente calibrados em 8 tiers de progressão para as 3 classes (Guerreiro, Atirador e Ladino).
* **5 Patamares de Raridade:** Cada item rastreia 5 qualidades (Comum, Mágico, Raro, Épico e Lendário), totalizando 240 entradas colecionáveis.
* **Alternância de Categoria:** Navegação imediata entre Armas e Armaduras pelo atalho `T` ou clique/toque no botão da categoria.
* **Otimização do Título Complecionista:** Regra canônica refinada — registrar uma arma/armadura de cada patamar sem depender de milhares de variantes de afixos secundários. Conclusão integral concede +100% de atributos principais (ATK, DEF, PV, PM).
* **Scan em Tempo Real & Persistência:** Rastreamento contínuo da mochila e persistência atômica no sidecar `weapons-catalog-v1.txt` (PC) e `localStorage` (Web).

### 2. Baú Compartilhado entre Personagens / Shared Stash (F5 / B)
* **Capacidade de 48 Slots:** 2 páginas com 24 slots cada, permitindo armazenar e transferir armamentos, armaduras, elmos, luvas, botas, acessórios e consumíveis entre todos os personagens do jogador.
* **Interface com Destaque por Raridade:** Cores de raridade dinâmicas em cada slot, navegação de páginas (tecla `P` ou abas superiores) e botão de retirada direta para a mochila.
* **Segurança e Isolamento Total:** Gravações atômicas em `shared-stash-v1.txt` sem alterar os arquivos `save*.dat` originais, garantindo compatibilidade total e zero risco de corrupção.

### 3. Sistema de Bounties / Contratos Offline Repetíveis (F8 / C)
* **3 Contratos Diários por Slot:** Desafios de caça a monstros comuns (ex: Soldados Imperiais), chefes lendários do Boss Rush (ex: Diablo) e desafios de caça épica.
* **Contabilização Nativa via Hooks ARM:** Abates detectados de forma leve e instantânea na rotina original de monstros.
* **Recompensas Balanceadas:** Premiações proporcionais em Ouro e Cash offline ao reivindicar contratos concluídos.
* **Interface e Persistência:** Cartões interativos com barras de progresso, botões de resgate e persistência isolada em `bounties-v1.txt` (PC) e `localStorage` (Web).

### 4. Quest Tracker HUD & Arena Gauntlet de Chefes
* **Rastreador de Missões (Q / Toque):** Widget no canto superior direito do HUD com arte dourada Gamevil, ícone de pergaminho pixel-art, exibição dinâmica do objetivo e destino ativo e alternância rápida via tecla `Q`.
* **Arena Gauntlet (10 Chefes em Sequência):** Modo sobrevivência definitivo na 11ª opção do Mago dos Bosses (`species 999`), batalhando sem descanso por 10.000 G.

### 5. Auto-Potion QoL (Consumo Inteligente de Poções) (H)
* **Consumo Automático:** Auto-cura quando PV < 35% e recuperação de PM quando < 25%, respeitando intervalo seguro de recarga de 60 frames (1 segundo).
* **Atalho Rápido:** Tecla `H` ativa/desativa com aviso visual e sonoro nativo.

### 6. Santuário do Poder: Reset de Atributos & Habilidades + Novo Jogo+
* **Reset Offline por Ouro (5.000 G):** Restaura atributos básicos (FOR, DES, CON, INT) e habilidades, devolvendo 100% dos pontos para redistribuição imediata via `GameUI_calcHeroStat`.
* **Ciclos de Novo Jogo+ (NG+1 a NG+5):** Reinicia o mapa da campanha preservando nível, itens, baú, títulos e mascotes, com escalonamento de monstros (+35% HP/ATK) e bônus de XP/Ouro (+50%).

### 7. Mascotes & Companheiros Canônicos + Torre Abissal (50 Andares)
* **4 Mascotes Offline:** Lobo Glacial, Fênix Ígnea, Golem de Pedra e Mini-Diablo com bônus passivos e sprite flutuante com animação e sombra renderizado ao lado do herói.
* **Masmorra Infinita da Torre Abissal:** 50 andares de desafios progressivos com chefes a cada 5 andares e recompensas escalonadas salvas em `abyss-v1.txt`.

### 8. Hotfix Crítico de Performance Web (Mobile / Safari iOS)
* **Aceleração de 7x no Framebuffer:** Implementada LUT RGB565 -> RGBA32 de 256 KB em `Uint32Array`, reduzindo o tempo de render para ~0.35ms.
* **16 FPS Estáveis no Safari iOS:** Pacing do loop em Web Worker acionado diretamente pela janela principal (`nextFrame`), contornando o estrangulamento de 3 FPS do iOS Safari em background.
* **Auto-Avanço de Logos:** Pula sequencialmente as telas da Everple e Gamevil com suporte a toque para avançar ao menu inicial em menos de 1.6s.
* **129 Testes Unitários:** Suíte automatizada Web aprovada com 100% de sucesso.
