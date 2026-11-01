# Verificação da candidata 0.17.4

- O motor ARM original exibiu HP 303/505 e MP 202/404 com campos sintéticos distintos, confirmando os campos usados para os limites.
- Testes ARM com saves temporários nos hosts normal e Zoom verificaram consumo do item correto, recuperação gradual e restauração do atalho. Casos de estoque vazio, item desconhecido, personagem morto e limites são cobertos separadamente.
- Um cenário ARM adicional exercita HP e MP baixos simultaneamente com outra página de atalhos, verifica o consumo dos dois itens corretos e a restauração do atalho, e respeita o intervalo nativo entre usos.
- Tentativas recusadas aguardam 10 quadros antes de nova tentativa. Falhas de acesso recuperáveis desativam a auto-poção com aviso; a restauração do atalho é verificada, e falha não comprovada interrompe a sessão por segurança.
- O teste nativo utiliza a mesma ligação ARM dos hosts e injeta leitura inválida; testes sintéticos cobrem falha parcial de escrita, falha de restauração e verificação, além do desenho alternativo do aviso.
- Uma correção Web foi investigada em branch isolada, mas foi retirada desta candidata a pedido do proprietário; nenhum código Web ou deployment é alterado aqui.
- Não se afirma recuperação de mana já excedente, ressarcimento de contratos ou validação com saves pessoais. CI, pacotes finais e gameplay físico exigem verificação própria.
- A base 0.17.3 PC foi publicada após CI e auditoria dos pacotes, e instalada com backup e hashes de dados conferidos; esta candidata permanece separada até suas próprias verificações.
