# Desempenho do Destinia iOS — auditoria de 06/11/2026

## Objetivo

Preservar o jogo Android **1.0.6**, executado pelo Unicorn-TCI ARM32,
e buscar **16 atualizacoes de logica por segundo** (periodo de ~62,5 ms).
Isso nao implica a promessa de 60 FPS; o iPhone pode comportar-se de
modo diferente do runner macOS.

## Evidencias automaticas

- [Teste #42154918777](https://github.com/sugyua/destinia/actions/runs/42154918777):
  perfil BASE e aceleracao de retangulos/sprites; mediana nos quadros 47–49:
  **118 ms** sem aceleracao e **6 ms** com aceleracao (~19,7x).
  O carregamento inicial pesado permaneceu ~3 segundos no macOS.
- [Teste #42155218560](https://github.com/sugyua/destinia/actions/runs/42155218560):
  comparacao de 50 e 360 quadros, 3 modos de memoria; imagens finais
  identicas. Quadros 60–359: mediana ~109 ms (original), ~9 ms (graficos
  nativos), ~8 ms (graficos + heap mapeado). Percentil 90: 135/16/19 ms.
- [Teste #42156713768](https://github.com/sugyua/destinia/actions/runs/42156713768):
  **360/360 hashes RGB565 identicos** nos 3 modos, incluindo quadros
  intermediarios e nao apenas a tela final. O caminho de memoria direta
  registrou **86.411 preenchimentos** e **150.720 colunas de sprites**.
  A compilacao Objective-C++ para iOS passou.

**Limite importante:** todos esses tempos sao medidos em um runner
GitHub macOS ARM64; nao correspondem a FPS medidos no iPhone e nao
garantem estabilidade de todas as cenas de gameplay.

## Otimizacoes e testes

| Hipotese | Experimento | Resultado |
| --- | --- | --- |
| Loop ARM32 pixel-a-pixel e gargalo | Hooks nativos para `Grp_fillRect` e `Grp_drawClipWImageP` | Aprovado; imagens identicas |
| Copia repetida de grandes regioes de memoria | `uc_mem_map_ptr` com heap compartilhado de 64 MiB | Aprovado no macOS; fallback para mapa anterior se indisponivel no dispositivo |
| Alocacoes dinamicas por coluna | Buffers temporarios no stack de tamanho fixo | Compilado e incluido na validacao completa |
| Copias C++ ↔ Unicorn do heap | `memmove` sobre mapeamento direto, quando valido | Incluido na verificacao de 360/360 quadros |
| Alteracoes podem mudar frames intermediarios | FNV64 de todos os quadros, tres configuracoes | **360/360 iguais** |
| FPS real no iPhone desconhecido | Medicao local de FPS e tempo ARM32/RGB565 | Implementado; **ainda precisa de instalacao no dispositivo** |

## Como inspecionar no iPhone

Na proxima compilacao de teste para iOS, toque a tela com **3 dedos**
para mostrar ou esconder a faixa de metricas de desempenho.

- `FPS`: quadros executados por segundo observados pela fila do jogo.
- `ARM32`: tempo medio por quadro gasto em `jniRun` e no retorno dos pixels.
- `RGB`: tempo medio de preparacao/conversao dos pixels antes de enfileirar
  a atualizacao do UIKit.

As metricas sao atualizadas aproximadamente a cada 2 segundos e tambem
emitidas no log do aplicativo. A primeira conclusao concreta sobre
o dispositivo deve usar esses numeros, nao estimativas visuais.

## Proximos gargalos provaveis

- Custo da interpretacao ARM32 das outras rotinas da logica (sem JIT).
- Latencia da exibicao dos quadros em UIKit no dispositivo.
- Carregamento e descompactacao de recursos durante transicoes.
- Reducao de frequencia da CPU em sessoes longas e aquecimento.

Nao reduzir artificialmente a velocidade dos ticks nem descartar
quadros da logica para aumentar o numero exibido no contador de FPS.
