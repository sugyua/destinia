# Verificacao da sequencia Destinia 0.13-0.16 (28/10/2026)

## Resultados locais

- 0.13: 21 testes Web direcionados e smoke ARM de 51 ondas nos hosts normal e Zoom; aprovados.
- 0.14: teste do painel F12 e builds Release CMake/Ninja dos dois hosts; aprovados.
- 0.15: testes da ficha de atributos e builds Release CMake/Ninja dos dois hosts; aprovados.
- 0.16: desmontagem ARM em lote nos hosts normal e Zoom; prévia cancelada ao fechar o painel e ao resetar; aprovada.
- Correções de revisão: abates da arena usam seu mapa real; inimigos são liberados ao sair; nível dos inimigos avança com as ondas; toque alcança as duas linhas de abas; saves restaurados do sidecar local não são sobrescritos por cópias antigas da nuvem; sidecars inválidos não encerram o host.
- O Site Web privado atualizado permite testar no celular: https://destinia-web-experimental.leafyheron257.chatgpt.site. F12, Ctrl+F3 e desmontagem em lote são exclusivos dos hosts PC.
- Não houve gameplay manual nem leitura térmica durante esta rodada local; os testes ARM foram headless e curtos. O limite pedido pelo proprietário continua sendo 72 °C para CPU e temperatura principal da GPU, sem contar hotspot/VRAM.

## CI

Resultados de CI do commit final e estado da publicação serão acrescentados após a validação no GitHub.

## Limites

Nao foi feito gameplay fisico nem verificacao visual manual no monitor/jogo. O teste do clone do Site encontra fixtures que dependem de caminho absoluto local; a suite principal Web no repositorio Destinia passou.
