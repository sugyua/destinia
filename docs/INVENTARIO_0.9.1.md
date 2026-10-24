# Inventário 0.9.1 — lote de desenvolvimento

**Ainda não há filtro novo disponível para o jogador.** A Release PC e a web atuais permanecem preservadas.

## Base preparada

Projeção somente de equipamentos, separada dos dados ARM: filtrar por tipo/rank/faixa de nível e ordenar por tipo/rank/nível/preço, com desempate estável. Seleção por pool de equipamento da sessão; item filtrado ou ausente não seleciona outro automaticamente. Duplicidade de pool/posição é rejeitada. Não usar esse índice para favoritos persistentes nem dispensar a revalidação do equipamento antes de uma ação.

Testes puros verificam ordenação, filtros, seleção estável e ausência de mutação. A fixture ARM dos dois hosts verifica preços nativos, estado original inalterado durante a projeção e associação entre linha ordenada e slot original desenhado por `_drawEquipDesc`.

## Fatos e limite da integração

- UI+0x54 seleciona a bolsa; UI+0x56 seleciona o slot no estado0x20000002. A projeção não pode gravar sua posição exibida nesse campo como se fosse o slot original.
- O popup0x20000003 desta tela oferece ações nativas1/2/0/3: combinar/desmontar/mover/descartar. **Não é o fluxo de equipar.** A fixture abre e cancela, conferindo itens, seleção e ouro preservados.
- A tentativa inicial de tratar a primeira opção como equipar falhou porque a premissa sobre o menu estava errada. Nenhum fluxo de equipamento/venda foi alterado. A confirmação real de equipamento/venda com projeção continua pendente.

## Próximo lote

1. Mapear a aba de equipamentos e validar o destino real de equipar/vender com equipamentos repetidos e troca de seleção.
2. Integrar uma interface dentro do jogo com navegação por teclado/controle/toque, sem escrita da projeção no inventário/save.
3. Portar a apresentação para web; validar filtros vazios, atualização após ação e seleção preservada.
4. Executar integração e CI Windows antes de declarar o item14 entregue. Atualizar manifesto de distribuição quando a integração estiver pronta.

Não adicionar favoritos ou automações a este lote.
