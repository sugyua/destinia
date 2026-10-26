# Notas de Lançamento — Destinia 0.10.0 Beta

> **AVISO: Versão Beta experimental. Faça backup dos seus saves antes de atualizar.**

## Destaques da Versão 0.10.0 Beta

1. **Nível 999 & 14 Patentes (Frente 1)**:
   - Aritmética segura de XP em 64 bits (`uint64_t`/`BigInt`) sem overflow ARM.
   - Curva de XP canônica expandida até o limite de 2.035.180.079.455 XP.
   - 14 Patentes de Ranks (Aprendiz até Supremo) com status parametrizados.
   - Suporte a 5 Dificuldades e multiplicadores de NG+ linear.

2. **Conjuntos de Equipamentos com Bônus Progressivos de 2 a 6 Peças (Frente 2)**:
   - 5 Conjuntos temáticos (Set do Dragão, Sombrio, Imperial, Arcano, do Guardião).
   - Bônus cumulativos desbloqueados progressivamente com 2, 3, 4, 5 e 6 peças equipadas.
   - Origens concentradas (drop 100% de 1 boss) e distribuídas (divididas entre chefes e elites).

3. **Economia de Cash por Missões Épicas (Frente 2)**:
   - Remoção integral do ganho passivo de Cash offline por tempo.
   - Cash concedido exclusivamente através de 5 Missões Épicas de marcos de chefes e set completo (10 a 25 Cash), calibrado contra os preços da loja (.gdt).

4. **Mago dos Bosses — Boss Rush (Frente 1 & 2)**:
   - NPC em Lagrant com estilização baseada no sprite `mon_6_0` (manto azul, detalhes em branco/prata e rosto escuro).
   - Tabela canônica de teleporte calibrada em ouro (500 a 50.000 ouro) para os 10 chefes exclusivos (`species 100..109`).

5. **Sistema de Títulos & Título Complecionista (Frente 3)**:
   - Persistência sidecar `titles-v1.txt`.
   - Título "Complecionista" concedido ao alcançar 100% do Códex (73 monstros), garantindo buff de +100% em ATK, DEF, PV e PM.

6. **Centralização da Barra de Menu (10 Ícones)**:
   - Recálculo dos offsets em X para acomodar o 10º ícone (Códex) com alinhamento visual e hitboxes perfeitas nos dois tamanhos de tela.

7. **Correção Crítica de Viewport e Performance Web (800x480)**:
   - Resolução da regressão visual de mapa duplicado e queda de FPS.
   - Centralização e corte de tiles idênticos ao `host_zoom` Windows, mantendo 16 FPS estáveis.

8. **Preservação Integral de Saves**:
   - `save*.dat` originais 100% intocados byte a byte.
   - Toda progressão estendida reside em sidecars versionados (`progression-v1.txt`, `economy-v1.txt`, `titles-v1.txt`).
