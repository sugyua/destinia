# Destinia 0.16.0 Beta

## Desmontagem em lote

- No inventário dos hosts normal e Zoom, filtre os itens elegíveis e abra a prévia do lote.
- Confira a marcação de itens selecionados, protegidos e equipados antes de confirmar.
- O lote processa até 50 itens. Cada item é validado novamente antes de chamar o fluxo ARM original.
- F8 abre e confirma a prévia em duas etapas; Esc ou fechar o painel cancela a prévia.
- Se um alvo mudar ou uma operação falhar, o lote para e informa quantos itens foram concluídos.

## Recursos acumulados

- 0.13.0: arena de sobrevivência e refino +15 em PC e Web.
- 0.14.0: painel de diagnóstico F12 nos hosts PC.
- 0.15.0: ficha de atributos Ctrl+F3 nos hosts PC.
- 0.16.0: desmontagem protegida em lote nos hosts PC.

## Compatibilidade e validação

- A arena dos hosts PC mantém a progressão de nível por onda, contabiliza abates usando o mapa correto e limpa os inimigos da arena ao retornar à campanha.
- A ficha Ctrl+F3 exibe corretamente os sinais de porcentagem. O painel F12 mostra memória zerada quando a consulta não retorna uso de memória.
- Saves locais restaurados do armazenamento dos recursos 0.13 têm precedência sobre cópias antigas da nuvem.
- Sidecars de arena/refino inválidos desativam esses recursos e registram o erro sem impedir o jogo de abrir.
- Consultas finais de CI e publicação são registradas no changelog e no relatório de validação desta sequência.

Faça backup dos saves antes de atualizar. O pacote não inclui os dados originais do jogo nem saves.
