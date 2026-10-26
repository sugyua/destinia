# Mago dos Bosses e centralização da barra

Escopo aprovado: opção A (arte mon6_0 com paleta azul/branco/sombra), NPC sintético host-side em posições verificadas de cidades, evento `openBossRushMenu()` para a UI do OpenCodes; centralizar dez ícones e suas hitboxes em C++/web 400×240 e 800×480. Worktree exclusivo wt-vanguard-010, branch feat/010-vanguard-progression; merges pela orquestração em meta-010-integration.

1. Diagnóstico somente leitura: confirmar recursos mon6_0_head.wmb/mon6.agd, mapnames e NPCs/portais/collision tiles originais; identificar contrato da UI com OpenCodes. Nunca presumir Icarus=Lagrant ou renomear mapas.
2. Escrever testes de geometria: dez células de 23×24 centradas, moldura 256px centralizada e mesma transformação para raster/hits, ambas resoluções e escalas. Corrigir painter nativo/JS, redesenhar cursor nativo na posição nova sem duplicar o antigo; manter nove estados ARM originais.
3. Módulo de arte usa decoder WBI/AGD existente e remapeamento RGB565 em memória; mantém transparência/silhueta e não modifica assets originais. NPC ocupa apenas estado host-side, sem incrementar pools ou alterar hitboxes/collision/save ARM.
4. Validar posição por mapa e obstáculos/nativos antes de habilitar; interação manual próxima/facing, sem interceptar NPCs originais, coleta automática ou transições. Evento diferido fora de uc_emu_start, exatamente uma abertura por pressão, entregue à UI `openBossRushMenu` por callback explícito.
5. Testes puros/paridade e ARM nos dois hosts + web, isolamento de paleta, buffers/estado/pools/saves e toque/teclado. Capturas locais para verificar aparência. Documentar cidades realmente identificadas, contrato e limitações.
6. README/CHANGELOG/allowlist, central Issue4/HANDOFF, commits/push e aviso ao Maestro/Sentinel. Sem main/release/deployment nesta frente de integração.
