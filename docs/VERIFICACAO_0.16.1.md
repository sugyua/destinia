# Candidata 0.16.1 — validação local

- Base: origin/main 865752f; árvore principal e saves pessoais preservados.
- C++: regressão de threshold falhou no baseline anterior e passou na candidata. Teste de selo/acentos/clipping aprovado.
- Builds Release normal e Zoom aprovados; `--version` retorna 0.16.1 e smoke ARM de dez quadros em cada host aprovado, com saves e preferências temporários.
- Guard MSI Afterburner, limite 68 °C: 86 amostras, CPU máxima 48,25 °C, GPU principal 37 °C. Sem parada térmica.
- Teste de versão dos fixtures nativos e wrapper da auto-poção aprovados; definição VERSION adicionada à compilação do fixture. Guard: 36 amostras, CPU máxima 55,25 °C, GPU principal 37 °C.
- Coerência: quatro testes Python aprovados, incluindo regressão do link histórico 0.16.0. Pacotes: sete testes de fontes e dois Windows aprovados.
- Web: 157 aprovados, zero falhas, um ignorado (158 testes), 74,9 s. A suíte foi executada localmente; as máximas térmicas acima pertencem aos jobs nativos monitorados, não a toda a suíte Web.
- CI Windows passa a incluir o teste nativo de threshold da auto-poção. A CI remota do commit `fa552ac` foi aprovada: Node tests, Windows e CodeRabbit. A revis?o complementar abaixo exige nova CI.

## Limites

O controle de estoque da auto-poção possui defeitos preexistentes: PC pode consumir outro consumível ou recuperar PV sem estoque; Web não verifica estoque. Esta candidata corrige threshold e PV cheio, não esse contrato. Registrar como próxima correção da assistência opcional; não alegar estoque validado.

Gameplay físico, publicação/download de release, instalação nova e deployment Web desta revisão ainda pendentes. Fonte Web permanece identificada como 0.13.0; o site privado não foi alterado nesta rodada.


## Revis?o complementar de 30/10/2026

- As regress?es reproduziram falhas no controle de mana com PV m?ximo ausente (C++ e Web) e no manifesto de depend?ncias Web. Os tr?s testes falharam antes da corre??o e passaram depois.
- Um PV m?ximo inv?lido bloqueia apenas a cura de PV; um personagem sem PV atual continua sem assist?ncia. A mana v?lida pode ser recuperada independentemente.
- O manifesto inclui `auto-potion.mjs`, `difficulty.mjs`, `loot-economy.mjs` e `map-viewport.mjs`, fontes locais j? rastreadas. O teste verifica os imports relativos dos m?dulos Web inclu?dos.
- Testes dirigidos: Web, 13 aprovados; coer?ncia, 5 aprovados; teste nativo de auto-po??o aprovado sob monitoramento t?rmico (CPU m?xima de 43,5 ?C, GPU principal de 37 ?C, 8 amostras).
- Novos builds Release normal e Zoom, `--version` e smoke ARM de dez quadros por host aprovados com saves tempor?rios. Monitoramento: 74 amostras, CPU m?xima de 57,5 ?C e GPU principal de 38 ?C.
- Su?te Web completa: 158 aprovados, 1 ignorado, nenhuma falha (159 testes, 76,2 s). Pacotes: 7 testes de fontes e 2 testes Windows aprovados.
- Revis?o independente das corre??es sem bloqueadores. Os ZIPs locais passaram na auditoria de conte?do, CRC e SHA-256; os imports relativos dos m?dulos Web foram conferidos no ZIP de fontes.
- A CI final e a entrega permanecem pendentes. A vers?o Web hospedada n?o foi atualizada nesta revis?o.
- O selo permanece no tamanho fixo previsto pela implementa??o em 800?480. O ajuste visual futuro n?o integra esta corre??o. O defeito preexistente de estoque continua explicitamente documentado.
