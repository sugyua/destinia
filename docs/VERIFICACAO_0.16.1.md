# Candidata 0.16.1 — validação local

- Base: origin/main 865752f; árvore principal e saves pessoais preservados.
- C++: regressão de threshold falhou no baseline anterior e passou na candidata. Teste de selo/acentos/clipping aprovado.
- Builds Release normal e Zoom aprovados; `--version` retorna 0.16.1 e smoke ARM de dez quadros em cada host aprovado, com saves e preferências temporários.
- Guard MSI Afterburner, limite 68 °C: 86 amostras, CPU máxima 48,25 °C, GPU principal 37 °C. Sem parada térmica.
- Teste de versão dos fixtures nativos e wrapper da auto-poção aprovados; definição VERSION adicionada à compilação do fixture. Guard: 36 amostras, CPU máxima 55,25 °C, GPU principal 37 °C.
- Coerência: quatro testes Python aprovados, incluindo regressão do link histórico 0.16.0. Pacotes: sete testes de fontes e dois Windows aprovados.
- Web: 157 aprovados, zero falhas, um ignorado (158 testes), 74,9 s. A suíte foi executada localmente; as máximas térmicas acima pertencem aos jobs nativos monitorados, não a toda a suíte Web.
- CI Windows passa a incluir o teste nativo de threshold da auto-poção. CI remoto do commit fa552ac aprovado: Node tests, Windows e CodeRabbit. A revisao complementar abaixo exige nova CI.

## Limites

O controle de estoque da auto-poção possui defeitos preexistentes: PC pode consumir outro consumível ou recuperar PV sem estoque; Web não verifica estoque. Esta candidata corrige threshold e PV cheio, não esse contrato. Registrar como próxima correção da assistência opcional; não alegar estoque validado.

Gameplay físico, publicação/download de release, instalação nova e deployment Web desta revisão ainda pendentes. Fonte Web permanece identificada como 0.13.0; o site privado não foi alterado nesta rodada.


## Revisao complementar de 30/10/2026

- Reproduzidas falhas no controle de mana com PV maximo ausente (C++ e Web) e no manifesto de dependencias Web. As tres regressoes falharam antes da correcao e passaram depois.
- PV maximo invalido bloqueia apenas a cura de PV; personagem sem PV atual continua sem assistencia. Mana valida pode ser recuperada independentemente.
- Manifesto inclui auto-potion.mjs, difficulty.mjs, loot-economy.mjs e map-viewport.mjs, todos fontes locais ja rastreados; teste verifica os imports relativos dos modulos Web incluidos.
- Testes dirigidos: Web 13 PASS; coerencia 5 PASS; nativo de auto-pocao PASS sob guard (CPU maxima 43,5 C; GPU principal 37 C; 8 amostras).
- Novos builds Release normal/Zoom, --version e smoke ARM dez quadros por host aprovados em saves temporarios. Guard: 74 amostras, CPU maxima 57,5 C e GPU principal 38 C.
- Suite Web completa: 158 PASS, 1 SKIP, 0 FAIL (159 testes, 76,2 s). Pacotes: 7 testes fontes e 2 Windows PASS.
- Nova CI e entrega final permanecem pendentes nesta revisao.
- Revisao de tamanho do selo em 800x480: permanece no tamanho fixo previsto pela implementacao; ajuste visual futuro nao integra esta correcao. Estoque continua como limite preexistente explicitamente documentado.
