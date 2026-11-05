# Verificação local da candidata 0.13.0 Beta

Data: 28/10/2026. Branch de trabalho: `feat/0130-vanguard-survival-refinement`.

## Escopo implementado

- Arena infinita, até 12 monstros simultâneos, chefes a cada 5 ondas e ondas elite a cada 3.
- Recordes por slot: maior onda, total de abates, maior tempo sobrevivido e pontuação determinística.
- Título desbloqueado na onda 20; +120% nos atributos principais, mitigação de 20% e esquiva de 15%.
- Último Suspiro opcional: limita dano recebido a 60% do PV máximo, deixa o jogador com 1 PV quando seria fatal, concede imunidade por 2 s e recarrega em 120 s.
- Elixir opcional: 20% de mitigação por 300 s. Não consome itens nem moeda nesta candidata.
- Refino garantido até +15, com pedras por faixa e sidecars separados do save nativo.
- As mesmas regras de domínio e persistência estão nos módulos Web; a interface nativa ganhou ações para Último Suspiro e elixir.

## Resultados

- `npm test` em `web/`: **147 passaram, 0 falharam, 1 ignorado**, em 75,8 s.
- `scripts/test_update_0130.ps1`: **19 testes direcionados passaram**, incluindo política/refino, arena, relógios pausados, CAS/atomicidade e adapter Web.
- Smoke ARM `-O0` para `host` e `host_zoom`: **ambos passaram**, com arena até a onda 51, elites, chefes, recompensa, restauração do mapa, refino e Último Suspiro. Smoke sintético; não é gameplay manual.
- CMake/Ninja Release com GCC UCRT64: os dois executáveis compilaram. Artefatos temporários foram mantidos fora da árvore do projeto.
- Pico amostrado durante as validações: CPU **61,4 °C**, GPU principal **40 °C**. Hotspot e VRAM foram excluídos conforme instrução do autor; gatilho permaneceu em 68 °C.
- Os testes C++ imprimiram avisos de `-Wmisleading-indentation` em `common/safe_save.h`; não falharam e esse arquivo não foi alterado nesta rodada.

## Limitações e publicação

- Não foi feita validação visual/jogável prolongada no Windows nem teste físico de USB/controle.
- CI do SHA final ainda não executada; a branch também precisa incorporar a atualização recente de `main` e os commits locais da suíte CI Web.
- Nenhum pacote, checksum, GitHub Release ou deployment Web foi publicado. O site existente não foi alterado.
- Os testes que avançam 51 ondas usam inimigos sintéticos/ARM headless e não provam balanceamento ou qualidade visual em jogo real.

## Próxima ação

Integrar `origin/main` e a correção/ação CI Web já trabalhada em outra branch, revisar o diff curado e rodar novamente testes/builds no SHA resultante. Só então preparar pacote e Release conforme as regras do projeto.
