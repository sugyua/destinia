# Destinia 0.17.4 Beta — candidata

- A auto-poção dos hosts PC lê os campos originais de vida e mana, confirma o tipo e o estoque da poção e usa a rotina nativa de consumo e recuperação gradual.
- O atalho temporário usado para acionar a rotina é restaurado; o jogador mantém seus atalhos configurados. A Web não foi alterada nesta candidata e ainda apresenta o defeito conhecido.
- Tentativas recusadas aguardam um intervalo curto; erros de acesso desativam a autopoção com aviso, e uma restauração de atalho não confirmada interrompe a sessão para proteger os dados.
- A candidata não corrige automaticamente valores de mana que já excedam o máximo em saves anteriores; não altera saves pessoais nos testes.
- Recompensas de contratos, leitura de ouro do Mago, barra duplicada e sobreposição do rastreador de missão são frentes separadas e não são declaradas corrigidas nesta candidata.
- Campanha e controle físico não foram executados. A Web permanece nas revisões previamente verificadas, conforme solicitado.
