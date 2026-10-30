# Destinia 0.16.1 — validação

- Base: origin/main `865752f`; árvore principal e saves pessoais preservados.
- Regressões de PV e mana reproduzidas antes das correções e aprovadas depois, em C++ e Web. Teste de selo, acentos e clipping aprovado.
- Builds Release normal e Zoom, `--version` 0.16.1 e smoke ARM de dez quadros por host aprovados, com saves e preferências temporários.
- Monitoramento MSI Afterburner, limite de 68 °C: execução inicial com CPU máxima de 48,25 °C e GPU principal de 37 °C (86 amostras); fixtures nativos com CPU máxima de 55,25 °C e GPU de 37 °C (36 amostras).
- Revisão complementar: builds e smokes com CPU máxima de 57,5 °C e GPU principal de 38 °C (74 amostras). Teste nativo de mana com CPU máxima de 43,5 °C e GPU de 37 °C (8 amostras). Nenhuma parada térmica.
- Suíte Web completa: 158 aprovados, 1 ignorado, nenhuma falha (159 testes, 76,2 s). As temperaturas acima correspondem aos testes nativos monitorados.
- Coerência: 5 testes aprovados. Pacotes: 7 testes de fontes e 2 Windows aprovados. ZIPs locais auditados por conteúdo, CRC e SHA-256.
- Manifesto inclui `auto-potion.mjs`, `difficulty.mjs`, `loot-economy.mjs` e `map-viewport.mjs`; imports relativos conferidos no ZIP de fontes.
- Revisão independente sem bloqueadores. CI de `5b5c632`: Node tests, Windows e CodeRabbit aprovados. Conferir novamente os checks desta revisão de documentação antes da publicação.

## Comportamento corrigido

PV exatamente em 50% pode acionar a assistência; PV cheio e PV máximo ausente não acionam a cura de PV. Mana válida permanece independente da ausência de PV máximo. Personagem sem PV atual continua sem assistência.

## Limites

O estoque da autopoção tem defeitos preexistentes: o PC pode consumir outro consumível ou recuperar PV sem estoque; a Web não verifica estoque. Esta versão corrige o limite de PV, a proteção de PV cheio e a independência da mana, sem corrigir o contrato de consumo. Os testes não comprovam identificação de poções nem validação do estoque.

Gameplay físico, publicação/download e instalação final devem ser registrados após sua execução. A Web permanece identificada como 0.13.0; o deployment desta revisão não foi realizado. O selo usa tamanho fixo também em 800×480; a reformulação visual solicitada pelo proprietário será tratada separadamente.
