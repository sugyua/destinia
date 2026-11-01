# Verificação da candidata 0.17.4

- O motor ARM original exibiu HP 303/505 e MP 202/404 com campos sintéticos distintos, confirmando os campos usados para os limites.
- Testes ARM com saves temporários nos hosts normal e Zoom e na Web verificaram consumo do item correto, recuperação gradual e restauração do atalho. Casos de estoque vazio, item desconhecido, personagem morto, limites e pausas são cobertos separadamente.
- A revisão independente identificou consumo durante a pausa Web; a atualização foi movida para os passos efetivos da simulação e os quatro painéis pausados foram testados.
- A suíte Web da branch original registrou 198 aprovados, 2 ignorados e nenhuma falha. Após integração com a candidata do menu e os ajustes de versão, repetir os testes antes de publicar.
- Não se afirma recuperação de mana já excedente, ressarcimento de contratos ou validação com saves pessoais. CI, pacotes finais, gameplay físico e publicação da Web exigem verificação própria.
