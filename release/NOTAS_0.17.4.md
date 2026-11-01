# Destinia 0.17.4 Beta — candidata

- A auto-poção dos hosts PC lê os campos originais de vida e mana, confirma o tipo e o estoque da poção e usa a rotina nativa de consumo e recuperação gradual.
- O atalho temporário usado para acionar a rotina é restaurado; o jogador mantém seus atalhos configurados. A Web não foi alterada nesta candidata e ainda apresenta o defeito conhecido.
- Tentativas recusadas aguardam um intervalo curto; erros de acesso desativam a autopoção com aviso, e uma restauração de atalho não confirmada interrompe a sessão para proteger os dados.
- A candidata não corrige automaticamente valores de mana que já excedam o máximo em saves anteriores; não altera saves pessoais nos testes.
- O Mago dos Bosses consulta o saldo nativo de ouro, e o rastreador de missões no Zoom foi afastado da faixa superior. A conferência visual em gameplay ainda está pendente.
- O crédito de contratos, ressarcimento de contratos anteriores, mana já excedente, barra de abas duplicada e escala de inventário permanecem pendentes.
- Campanha e controle físico não foram executados. A Web permanece nas revisões previamente verificadas, conforme solicitado.
