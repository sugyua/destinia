# Leitura do ouro no Mago dos Bosses (PC)

O painel do Mago consultava o sucesso da chamada ARM como se fosse o saldo, mostrando 0 ou 1 em vez da quantia real. Os dois hosts usam agora o valor retornado pela rotina nativa `getMyGold`, com verifica??o de falha e limite do tipo utilizado pela interface.

O teste nativo usa dados tempor?rios e verifica que um saldo de 15113 ? recebido pela mesma fun??o usada pelo painel. Ele n?o altera nem ressarce contratos antigos, n?o prova resgate de recompensa e n?o substitui teste manual do fluxo completo no jogo. A mudan?a foi reaproveitada do commit local `fea83dda192eba6b240c311464c7e86c67473dd4`.
