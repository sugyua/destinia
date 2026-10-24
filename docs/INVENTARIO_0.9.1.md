# Filtros de equipamentos — desenvolvimento0.9.1

Painel interno implementado nos dois hosts e no protótipo web. Os ZIPs da Release0.9 continuam preservados e não incluem este lote; os filtros de equipamentos acompanham o ZIP Windows0.9.1. Demais pendências do roadmap continuam abertas.

## Uso

Na aba de equipamento, na lista da mochila ou na venda do vendedor local, abra **FILTROS**, **F7** (quando livre no PC) ou **X/ação extra** do controle. A lista corresponde aos equipamentos do contexto nativo: slot corporal atual na aba de equipamento, bolsa atual na mochila/venda. Consumíveis e itens de missão continuam na interface original.

- Tipo e rank; faixas de nível (todos, utilizável pelo nível, grupos de10) e preço.
- Ordenação original/tipo/rank/nível/preço, crescente ou decrescente; LIMPAR restaura os critérios. B/P identifica bolsa/posição originais. GOLD mostra o valor nativo; no vendedor mostra o que será recebido (valor original/5).
- Toque nos campos/linhas. No teclado/controle: cima/baixo percorrem campos/lista; esquerda/direita alteram critérios ou avançam a seleção; ação confirma a escolha. VOLTAR cancela.
- **VOLTAR AO ITEM SELECIONADO** destaca o alvo na tela original. Equipar/vender ainda exige a confirmação nativa seguinte. O painel nunca executa essas ações sozinho.
- Filtrar um item, mudar de página ou remover o alvo não autoriza escolher um vizinho. Para confirmar, a seleção precisa estar na página visível.

## Segurança e contratos

A projeção não reordena nem escreve o inventário/save. Coleta preço, nível e nome das rotinas originais, guarda pool e20bytes do equipamento e revalida estado/propriedade/conteúdo antes de navegar. Referências desatualizadas ou dados/ranks não verificados não viram ações. São reconhecidos ranks0–4 já tratados pelo contrato atual; a máscara ARM de3bits não é autorização para criar novos ranks.

O JNI fica suspenso enquanto o painel está aberto, como no Códex. HUD/câmera/atributos não são recalculados pelos filtros; o retorno ao item usa a prévia nativa existente. Escalas75/100/125 têm desenho e hit test correspondentes. Teclas reservadas/remapeadas, foco, desconexão e releases de comandos são preservados.

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


## Validação e limites

Testes puros de projeção, filtros/preço, seleção/páginas e raster protegido; ARM dos dois hosts com itens idênticos, escolha nativa, cancelamento, venda e proteção da equipada; Node/Chromium com toque75, imagem125 e gamepad emulado. Resultado final da CIWindows e revisões estão no HANDOFF da central. Testes automáticos não comprovam campanha/Windows/iPhone físicos.

Filtros de consumíveis, favoritos, automações e reset não são implementados por este lote.
