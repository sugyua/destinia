# Verificacao da candidata 0.17.5 PC

Testes com saves sinteticos exercitam navegacao e desenho nativo, requisitos originais, desbloqueio de Cacador de Fungos, bonus de critico e recarga de titulos. Testes de recompensa da sobrevivencia cobrem 51 ondas, trava, alteracao externa, interrupcoes durante substituicoes do lote e recuperacao. Testes nao simulam queda de energia nem sincronizador externo nao cooperativo e nao substituem gameplay real.

Veterano, Mestre do Combate e Matador de Chefes aguardam criterios; visual, controles fisicos e campanha ainda precisam de validacao em partida. A instalacao exige CI e download verificados, jogo fechado e backup de saves.

O ramo critico ARM foi testado com sorteio fixo antes e depois de equipar o titulo. Fungos da sobrevivencia contam, e o Codex pendente participa da transacao da recompensa. Se o processo terminar enquanto uma trava impedir a gravacao, a recompensa ainda em memoria pode ser perdida; o ultimo save confirmado e preservado. Isso nao foi validado com queda de energia.
