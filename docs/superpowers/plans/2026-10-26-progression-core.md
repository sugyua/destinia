# Frente 1 — núcleo de progressão 0.10

Escopo confirmado pelo proprietário: núcleo matemático e sidecar, desacoplados de ARM/HUD/combate. Branch `feat/010-vanguard-progression`; não publicar release ou alterar main nesta frente.

1. Investigar o host existente: XP corrente em `Game+0x48FC` (u32), nível em `Game+0x48FA` (u16), limite lógico 99; stats em `Game+0x48BA` lidos como i16. Preservar a curva `(n+2+n²/10)²+20` com divisões inteiras.
2. Testes C++ e Node antes dos módulos: bordas dos 14 ranks, todos os 999 níveis, XP máximo, overflow, status parametrizados, dificuldade/NG+ e guardas de conversão. Conferir falha inicial por ausência dos módulos.
3. Implementar `common/progression.h` e `web/dist/progression.mjs`: funções puras com XP u64/BigInt, status u32, multiplicadores em pontos-base e consultas para outros agentes.
4. Implementar `common/progression_store.h`: arquivo fixo `progression-v1.txt`, parser limitado/versionado, isolamento por modo/slot, lock, compare-and-swap e substituição atômica. Web usa codec idêntico e adaptador Storage com Web Locks e confirmação antes de reconhecer a gravação.
5. Exercitar falha de disco/quota, snapshot obsoleto, conteúdo inválido e preservação byte a byte de um save sintético. Comparar saída C++ e JS para todos os níveis e configurações representativas.
6. Atualizar README/CHANGELOG/allowlist e contrato dos hooks; executar suites, registrar central e enviar commit/push para revisão do Sentinel/Maestro. Não alegar gameplay NV999 nem deployment nesta etapa.

Decisão configurável, explicitada ao Maestro: NG+ linear de +25% HP/ATK e +10% XP/ouro por ciclo como padrão inicial; nenhum reset de campanha ou nível é executado pelo módulo. Dificuldades conforme tabela aprovada. Não existe fórmula nativa de status nova presumida: o chamador fornece a curva.
