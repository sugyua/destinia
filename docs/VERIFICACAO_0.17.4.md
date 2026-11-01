# Verificação da candidata 0.17.4

- O motor ARM original exibiu HP 303/505 e MP 202/404 com campos sintéticos distintos, confirmando os campos usados para os limites.
- Testes ARM com saves temporários nos hosts normal e Zoom verificaram consumo do item correto, recuperação gradual e restauração do atalho. Casos de estoque vazio, item desconhecido, personagem morto e limites são cobertos separadamente.
- Uma correção Web foi investigada em branch isolada, mas foi retirada desta candidata a pedido do proprietário; nenhum código Web ou deployment é alterado aqui.
- Não se afirma recuperação de mana já excedente, ressarcimento de contratos ou validação com saves pessoais. CI, pacotes finais e gameplay físico exigem verificação própria.
- A base 0.17.3 PC foi publicada após CI e auditoria dos pacotes, e instalada com backup e hashes de dados conferidos; esta candidata permanece separada até suas próprias verificações.
