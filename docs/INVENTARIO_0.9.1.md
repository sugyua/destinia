# Inventário 0.9.1 — lote de desenvolvimento

**Ainda não há filtro novo disponível para o jogador.** A Release PC e a web atuais permanecem preservadas.

## Base preparada

Projeção somente de equipamentos, separada dos dados ARM: filtrar por tipo/rank/faixa de nível e ordenar por tipo/rank/nível/preço, com desempate estável. Seleção por pool de equipamento da sessão; item filtrado ou ausente não seleciona outro automaticamente. Duplicidade de pool/posição é rejeitada. Não usar esse índice para favoritos persistentes nem dispensar a revalidação do equipamento antes de uma ação.

Testes puros verificam ordenação, filtros, seleção estável e ausência de mutação. A fixture ARM dos dois hosts verifica preços nativos, estado original inalterado durante a projeção e associação entre linha ordenada e slot original desenhado por `_drawEquipDesc`.

## Fatos e limite da integração

- UI+0x54 seleciona a bolsa; UI+0x56 seleciona o slot no estado0x20000002. A projeção não pode gravar sua posição exibida nesse campo como se fosse o slot original.
- O popup0x20000003 desta tela oferece ações nativas1/2/0/3: combinar/desmontar/mover/descartar. **Não é o fluxo de equipar.** A fixture abre e cancela, conferindo itens, seleção e ouro preservados.
- A tentativa inicial de tratar a primeira opção como equipar falhou porque a premissa sobre o menu estava errada. Nenhum fluxo de equipamento/venda foi alterado.

## Equipar/vender — prova adicional

- Aba de equipamento: estados0x10000001/0x10000002; lista de pools emUI+0x2e8, contadorUI+0x324. UI+0x54 é slot do corpo e UI+0x56 é índice na lista (contexto diferente da mochila).
- **Não basta escrever o índice visual.** Navegação nativa também prepara o equipamento de prévia. Alterar só UI+0x56 pode mostrar a descrição correta e confirmar outra prévia. A fixture usa os comandos originais, testa cancelar/restaurar e só então confirma, verificando Game+0x4584 e backupUI+0x2148 pelo endereço do pool.
- Duas espadas com20bytes idênticos em pools distintos: a cópia escolhida é equipada; a outra é vendida sem trocar as identidades. Cenário controlado Warrior, não campanha nem todas as classes.
- Vendedor local, entrada controlada0x91000001: opção vender→0x91040003→0x91040004; UI+0x58 é bolsa e UI+0x5a é slot. A confirmação0x91041005 guarda o registro original emUI+0x2e0. Cancelar não consome; confirmar concede preço nativo/5 (150→30gold) e remove somente a cópia alvo.
- A cópia equipada permanece ativa e inalterada; tentativa de vendê-la produz aviso0x91040106 e não muda ouro/equipamento.
- Após vender, quantidade/flag podem estar zeradas antes de compactar o contador da bolsa. Reconstruir a projeção com registros ativos/quantidade positiva; o pool removido deve perder a seleção, sem selecionar outro automaticamente.

Testes direcionados: fixture ARM original dos dois hosts e projeção pura aprovadas. Saves e autosaves do cenário são somente temporários. Sem teste físico Windows, CI nova, interface filtrada ou deployment web neste lote.

## Próximo lote

1. Integrar o mapeamento comprovado de equipamento/venda com revalidação do pool e prévia nativa; não aplicar índices visuais diretamente.
2. Integrar uma interface dentro do jogo com navegação por teclado/controle/toque, sem escrita da projeção no inventário/save.
3. Portar a apresentação para web; validar filtros vazios, atualização após ação e seleção preservada.
4. Executar integração e CI Windows antes de declarar o item14 entregue. Atualizar manifesto de distribuição quando a integração estiver pronta.

Não adicionar favoritos ou automações a este lote.
