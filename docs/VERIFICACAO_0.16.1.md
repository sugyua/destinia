# Candidata 0.16.1 — validação local

- Base: origin/main 865752f; árvore principal e saves pessoais preservados.
- C++: regressão de threshold falhou no baseline anterior e passou na candidata. Teste de selo/acentos/clipping aprovado.
- Builds Release normal e Zoom aprovados; `--version` retorna 0.16.1 e smoke ARM de dez quadros em cada host aprovado, com saves e preferências temporários.
- Guard MSI Afterburner, limite 68 °C: 86 amostras, CPU máxima 48,25 °C, GPU principal 37 °C. Sem parada térmica.
- Teste de versão dos fixtures nativos e wrapper da auto-poção aprovados; definição VERSION adicionada à compilação do fixture. Guard: 36 amostras, CPU máxima 55,25 °C, GPU principal 37 °C.
- Coerência: quatro testes Python aprovados, incluindo regressão do link histórico 0.16.0. Pacotes: sete testes de fontes e dois Windows aprovados.
- Web: 157 aprovados, zero falhas, um ignorado (158 testes), 74,9 s. A suíte foi executada localmente; as máximas térmicas acima pertencem aos jobs nativos monitorados, não a toda a suíte Web.
- CI Windows passa a incluir o teste nativo de threshold da auto-poção. CI remoto ainda pendente.

## Limites

O controle de estoque da auto-poção possui defeitos preexistentes: PC pode consumir outro consumível ou recuperar PV sem estoque; Web não verifica estoque. Esta candidata corrige threshold e PV cheio, não esse contrato. Registrar como próxima correção da assistência opcional; não alegar estoque validado.

Gameplay físico, publicação/download de release, instalação nova e deployment Web desta revisão ainda pendentes. Fonte Web permanece identificada como 0.13.0; o site privado não foi alterado nesta rodada.
