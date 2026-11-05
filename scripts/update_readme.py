with open('README.md', 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
skip = False
for i, line in enumerate(lines):
    if '#### Versão 0.11.0' in line:
        new_lines.append('#### Versão 0.11.0 (Entregue — Expansão Completa de Conteúdo, QoL & Endgame)\n')
        new_lines.append('- **Catálogo Completo de Equipamentos e Complecionista (F4 / Ctrl+W)**: 48 equipamentos canônicos (24 armas e 24 armaduras em 8 patamares por classe), 240 entradas colecionáveis por raridade, alternância imediata de categoria (`T`) e sinergia com o título Complecionista (+100% atributos principais).\n')
        new_lines.append('- **Baú Compartilhado entre Personagens / Shared Stash (F5 / B)**: 48 slots (2 páginas de 24 itens) com persistência isolada em `shared-stash-v1.txt`.\n')
        new_lines.append('- **Mural de Bounties / Contratos Offline Repetíveis (F8 / C)**: 3 contratos diários por slot de personagem, contagem nativa via hooks ARM e recompensas em Ouro e Cash.\n')
        new_lines.append('- **Quest Tracker HUD Retrátil (Q / Toque)**: Rastreador no canto superior direito do HUD com moldura Gamevil, pergaminho pixel-art e alternância rápida via tecla `Q`.\n')
        new_lines.append('- **Boss Rush Gauntlet (Arena de Sobrevivência 10 Chefes)**: 11ª opção no Mago dos Bosses (`species 999`), combate contínuo contra todos os 10 chefes de Destinia.\n')
        new_lines.append('- **Auto-Potion QoL / Consumo Inteligente (H)**: Monitoramento contínuo de PV (< 35%) e PM (< 25%), consumo com cooldown de 60 frames e atalho `H` para ativar/desativar.\n')
        new_lines.append('- **Reset de Atributos e Habilidades Offline (Santuário do Poder)**: Devolução integral de pontos acumulados por 5.000 G no Mago dos Bosses e recálculo ARM (`GameUI_calcHeroStat`).\n')
        new_lines.append('- **Mascotes & Companheiros Offline (Pets System)**: 4 mascotes (Lobo Glacial, Fênix Ígnea, Golem de Pedra, Mini-Diablo) com buffs passivos, níveis até Lv50 e mascote flutuante renderizado ao lado do herói.\n')
        new_lines.append('- **Ciclo Novo Jogo+ (NG+1 a NG+5)**: Reinício da campanha preservando nível, equipamentos, baú, títulos e mascotes, com escalonamento de dificuldade e recompensas.\n')
        new_lines.append('- **Torre Abissal / Masmorra Infinita (50 Andares)**: 50 andares de desafios progressivos no Mago dos Bosses com chefes a cada 5 andares.\n')
        new_lines.append('- **Otimização Crítica Web (Mobile / Safari iOS)**: LUT RGB565 -> RGBA32 (aceleração de 7x), pacing por `nextFrame` a 16 FPS contínuos e 126 testes aprovados.\n\n')
        skip = True
        continue
    if skip:
        if line.startswith('#### Versão 0.12.0'):
            skip = False
            new_lines.append(line)
        continue
    new_lines.append(line)

with open('README.md', 'w', encoding='utf-8', newline='') as f:
    f.writelines(new_lines)
print('README.md written successfully.')
