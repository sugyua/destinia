# Prioridades de controle — 0.9.1

Especificação: pedido do autor de24/10. Execução econômica, uma melhoria por vez; sem nova discussão de requisitos já definidos.

1. Web: confirmar presença da Gamepad API e controles expostos antes de adaptar. Reutilizar InputSources/FrameInputs e códigos ARM; não há leitura gamepad atual na web. Implementar polling padrão, diagnóstico de ausência/API bloqueada/mapeamento desconhecido, analógico/direcional, ações/menu/pausa; soltar na desconexão/foco e exigir neutro. Não tentar contornar permissões nem assumir compatibilidade USB. Testes Node e integração Chromium emulada; USB/Safari físico permanece pendente.
2. PC: reutilizar interface.<host>.controles_virtuais e os hooks ARM de desenho/hitbox já testados. Tornar a visibilidade dinâmica, adicionar L3+R3 com trava até ambos soltos e suprimir bindings individuais desses botões durante a combinação; preservar HUD/menus/teclado/controle, dois hosts e configurações alheias. Testar entradas SDL e ARM, incluindo foco/desconexão/duas posições iniciais.
3. Checkpoint separado por melhoria, atualizar README/central com resultados e limites. Depois retomar filtros do inventário; sem ampliar automações agora.

Arquivos: web dist/gamepad.mjs, app.mjs/index.html + testes; PC common/virtual_controls.h, player_controls.h, host/main.cpp, host_zoom/main.cpp + testes existentes. Sem dependência nova. Reusar instalação/servidor/testes disponíveis; não mexer em saves ou BAT pessoal.
