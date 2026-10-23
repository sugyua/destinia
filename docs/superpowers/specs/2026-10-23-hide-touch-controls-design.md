# Controles virtuais opcionais — primeira etapa da 0.8

Autor autorizou em conversa a ordem: ocultar controles e seus toques, depois ajustar escala de UI.

## Objetivo
Permitir jogar no PC sem os controles virtuais cobrindo o cenário. Preferência persistente pelo configurador existente; padrão visível preserva instalações atuais. Teclado, controle, menus e Códex continuam disponíveis. Não alterar câmera, zoom, HUD, saves ou ritmo original.

## Abordagens
Escolhida pelo autor: implementação incremental, começando pela visibilidade. Escala independente vem depois. A alternativa de implementar ambas juntas aumenta a superfície de regressão.

O framebuffer não separa UI e cenário. Usar pontos exatos do código ARM para suprimir somente desenho e áreas de toque dos controles em gameplay; preservar transição e estado anterior do toque; omitir também animação dos botões ocultados e rotas de menus. Validar biblioteca conhecida antes de instalar interceptações. Não utilizar hooks globais de memória.

## Escopo e critérios
Preferência por host `interface.<host>.controles_virtuais=0|1`, padrão 1. Exposta em Configurar Destinia, sem alterar BAT local pendente. Implementação confirmada nos dois hosts usando as mesmas rotas ARM; preferências independentes. Reiniciar jogo para aplicar. Não prometer paridade web nesta etapa.

Testar biblioteca ARM real com saves temporários: controles visíveis/ocultos, hitboxes, menus, fluxo de entrada física e estado interno preservado. Confirmar câmera e preferências existentes. Validação manual Windows permanece pendente. Publicação da 0.8 exige aprovação separada.
