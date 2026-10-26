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

### 4. Hotfix Crítico de Performance Web (Mobile / Safari iOS)
* **Aceleração de 7x no Framebuffer:** Implementada LUT RGB565 -> RGBA32 de 256 KB em `Uint32Array`, reduzindo o tempo de render para ~0.35ms.
* **16 FPS Estáveis no Safari iOS:** Pacing do loop em Web Worker acionado diretamente pela janela principal (`nextFrame`), contornando o estrangulamento de 3 FPS do iOS Safari em background.
* **Auto-Avanço de Logos:** Pula sequencialmente as telas da Everple e Gamevil com suporte a toque para avançar ao menu inicial em menos de 1.6s.
* **119 Testes Unitários:** Suíte automatizada Web aprovada com 100% de sucesso.
