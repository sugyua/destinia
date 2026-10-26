# 0.10.3 Parte 2 — Gameplay ARM

Pedido autorizado: multiplicadores canônicos de dificuldade ativa para HP, dano ao herói, XP e ouro; Abissal sem bônus de itens; dois hosts; testes ARM e commit para Sentinel. Não publicar release completa nesta etapa de review.

- Usar somente os cinco perfis padrão de common/progression.h, ciclo NG+0. Não acrescentar crescimento NG+ ou alterar regras/saves da Parte 1.
- Validar fingerprint original APK1.0.6 e offsets/símbolos antes dos hooks. Restringir a monstros de campanha da matriz Game+0x14fc (20 registros de136 bytes); excluir pets/rede.
- HP: aplicar antes do reset nativo Mon_initMonster, guardar base por instância, evitar empilhamento no respawn e preservar a fração de vida ao trocar dificuldade no mapa atual. Normal deve manter resultados originais.
- Dano: escalar o dano final em monsterHitProc antes de ser exibido/subtraído do herói, preservando defesa, variação, críticos e bloqueios nativos.
- XP: resultado de GameCanvas_getGainExp para monstros de campanha; não multiplicar novamente gainExpProc ou recompensas de missões. Ouro: apenas valor gerado no ramo ouro de GameCanvas_dropItem; não alterar setMyGold, compra, venda ou missão.
- Nenhuma modificação de probabilidades, qualidade, quantidade de itens ou consumo RNG. Comparar Normal/Abissal com os mesmos seeds em provas ARM.
- Teste primeiro a falha sem hooks; depois provar cinco perfis, slot, respawn/troca de mapa, vida parcial, limites/conversões e drops. Testar regressões da Parte 1 e compilar os dois hosts.
- Atualizar README/CHANGELOG/manifesto/notas e central, com limites de testes automatizados versus gameplay físico. Commit local da Parte 2 e handoff para Sentinel. Web preservada neste checkpoint PC.
