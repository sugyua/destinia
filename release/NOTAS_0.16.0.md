# Destinia 0.16.0 Beta — candidata local

Esta candidata reúne as atualizações locais 0.13–0.16. Faça backup dos saves antes de testar. Não há GitHub Release publicada para esta candidata.

## Desmontagem em lote

- Abra os filtros do inventário com F6 e escolha `LOTE` para revisar itens compatíveis com o filtro atual.
- `[L]` marca um alvo; `[P]` identifica item protegido e `[E]` item equipado.
- Use `CONFIRMAR LOTE` para prosseguir ou `Esc` para cancelar. O lote processa até 50 itens.
- F8 abre a prévia e, numa segunda ação, confirma o lote enquanto o painel estiver aberto.
- Cada alvo é validado novamente antes da desmontagem. Mudanças ou falhas interrompem a execução e informam quantos itens foram processados.

## Verificação local

Suíte Web: 147 aprovados, 0 falhas, 1 ignorado. Builds Release e testes ARM direcionados foram aprovados nos hosts normal e Zoom. Os detalhes, a medição térmica e as limitações estão em [VERIFICACAO_0.16.0.md](../docs/VERIFICACAO_0.16.0.md).

Ainda faltam CI remoto do SHA final, gameplay físico, pacotes Windows finais e checksums, deployment Web e publicação da GitHub Release. Esta é uma candidata local.
