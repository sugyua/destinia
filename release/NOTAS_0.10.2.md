# Notas de Lançamento — Destinia 0.10.2 Beta

> **AVISO: Versão Beta experimental. Faça backup dos seus saves antes de atualizar.**

## Destaques da Versão 0.10.2 Beta

1. **Mago dos Bosses em Todas as 5 Cidades (Safe Zones com Lojas)**:
   - Conforme a diretriz de design, o Mago dos Bosses agora está presente em todas as cidades completas do jogo que possuem Safe Zone (bloqueio de ataque/dash) e a presença de ambos os vendedores (Armas e Poções):
     - **Acampamento da Resistência (Resistance Camp - Mapa 61)**: X=280, Y=232.
     - **Bergheim (Mapa 62)**: X=504, Y=280.
     - **Acampamento dos Refugiados (Refuge Camp - Mapa 63)**: X=280, Y=296.
     - **Fermion (Mapa 64)**: X=344, Y=424.
     - **Lagrant (Mapa 65)**: X=328, Y=344.

2. **Hitbox Física e Colisão com o Cenário**:
   - O Mago dos Bosses agora possui corpo físico sólido no mapa (`curMapData + 0x68`), comportando-se exatamente como um NPC nativo ou obstáculo do cenário. O herói não consegue mais atravessá-lo.
   - O alcance de interação (`mageFacing`) foi calibrado para até 28 pixels, permitindo interação natural adjacente ao bloco de colisão.

3. **Placa de Nome Nativa e Proximidade (UI Original)**:
   - O letreiro "MAGO DOS BOSSES" agora só é exibido quando o herói está próximo (distância $\le 48$ px).
   - Renderização no padrão visual dos NPCs originais, com fundo escuro semitransparente e cantos arredondados, sem poluir a visão do mapa à distância.

4. **Gatilho de Interação e Menu de Teleporte Ativo**:
   - Pressionar o botão de Ação (ou tecla de ataque) de frente para o Mago abre o menu interativo de seleção de Boss Rush (`BossRushUI`), permitindo teleporte calibrado em ouro (500 a 50.000 G) para os 10 chefes do jogo (`species 100..109`).

5. **Compatibilidade e Preservação**:
   - Compatível com o host padrão (`destinia_host.exe`), host zoom 800x480 (`destinia_host_zoom.exe`) e versão Web.
   - Saves nativos `save*.dat` 100% preservados sem qualquer alteração de formato.
