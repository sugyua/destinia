# FC — Força de Combate (índice v1)

O FC aparece na aba Status com a fonte original do jogo. No inventário e na seleção de equipamento próprio, o rodapé da caixa de descrição alterna entre **Item** (texto original) e **Comparar** (FC atual, FC previsto, diferença e ATK/DEF/PV/PM máximos/crítico). O nome usa a cor da raridade; ganhos são verdes, perdas vermelhas e valores iguais cinza.

Use clique/toque nos rótulos ou o analógico direito (esquerda=Item, direita=Comparar). Na candidata0.9.9, F6 abre FILTROS e F7/P protegem diretamente o equipamento selecionado; os comandos originais de equipar permanecem independentes. A bolinha verde no canto superior direito do ícone indica ganho válido de FC naquele slot. A página escolhida é mantida na sessão, sem alterar preferências ou o formato dos saves.

A previsão não equipa o item. Itens de outra classe, sem nível suficiente ou não identificados exibem o motivo; ofertas/estruturas não reconhecidas não recebem estimativa inventada. A 0.9.7 muda a apresentação, mantendo os cálculos anteriores.

## Cálculo

O port usa os atributos derivados calculados pela rotina original `GameUI_calcHeroStat`, incluindo equipamentos, efeitos passivos e buffs ativos. Não conta força/agilidade/vitalidade/inteligência novamente, pois esses atributos já influenciam os derivados.

`FC = 4×ATK + 3×DEF + piso(PVmáx/10) + piso(PMmáx/10) + piso(HIT/10) + piso(EVA/10) + piso(CRI/5) + piso(BLD/10)`

HIT/EVA/CRI/BLD usam as unidades internas de décimos percentuais. Cada parcela é arredondada para baixo. Os pesos são uma decisão do port para um índice informativo inicial, não a fórmula oficial do DDTank. Uma pontuação maior não garante que o item seja melhor para toda estratégia, efeito condicional ou inimigo.

Buffs ativos entram nos dois lados da comparação. Sua expiração altera o valor conforme os atributos nativos são recalculados. Perder/recuperar vida ou mana atuais não altera o FC por si só; os máximos são usados.

## Preservação do personagem

A previsão captura o estado em memória, usa os vínculos de equipamento reais e a rotina nativa de atributos, lê o resultado e restaura o estado completo do personagem, inclusive HP/MP, ouro, inventário e timers. Não equipa o item, não gasta pontos/ouro nem grava um novo save para exibir a previsão. O formato dos saves originais não muda. A biblioteca ARM compatível continua sendo exigida.

Os testes verificam atributos nativos, aumento e redução, mesmos buffs/expiração, HP atual independente, incompatibilidades, ouro preservado, estado byte a byte, captura do desenho original e limites dos pixels nas escalas75/100/125, nos dois hosts. Há equivalentes Node e inspeção visual da UI web. Isso não equivale a uma campanha completa no Windows/iPhone.

O detalhamento dos demais atributos e contextos e as proteções/favoritos continuam no roteiro futuro; os filtros de equipamentos já têm uma entrega parcial independente. O painel inicial de comparação trabalha com os equipamentos próprios da mochila; ofertas ou estruturas não reconhecidas não recebem uma estimativa inventada.
