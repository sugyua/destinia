# Plano executado — 0.9.8 Frente 1: proteger equipamentos

Escopo autorizado: alternar proteção, indicador e impedir seleção destrutiva em 0x91040004, preservando ARM/pools/saves. Worktree isolado fornecido pelo Maestro. Após autorização expressa adicional, persistência passou a requisito da sprint.

Abordagem aprovada: metadados fora da memória ARM por personagem + pool + fingerprint de 20 bytes. Sidecar `inventory-protect-v1.txt` na pasta de save ativa, gravação atômica e formato estrito; web usa o mesmo texto em localStorage por modo. Sem alteração do formato de save*.dat ou do fluxo Google Drive. Gravação falha não confirma alteração; metadados inválidos são preservados e bloqueiam confirmação destrutiva no contexto verificado.

- [x] Testes inicialmente falhando para alternância, identidade e persistência; implementar common/inventory_protection.h e inventory_protection_store.h e equivalente JS.
- [x] Botão PROT/DESPROT, [P], F7 e ação extra; preservar filtros, seleção e equipamento.
- [x] Revalidar contexto/registro/fingerprint e sidecar; guarda na entrada ARM MainMenu_mKeyPressed, sem chamada reentrante ou escrita em Game.
- [x] Integrar nos dois hosts/web; preservar persistência ao iniciar/carregar partida e separar personagens/modos.
- [x] Testar C++, Node e ARM real com saves temporários. Revisão independente e correção dos dois P2 de persistência inválida.
- [x] Atualizar README/CHANGELOG e relatório técnico.
- [x] Commit/push na branch designada, central e relato ao Maestro.

Release, VERSION/manifesto globais e publicação web pertencem à integração das três frentes. Limitações e evidências em docs/INVENTORY_PROTECTION.md; não declarar gameplay físico nem publicação nesta frente.
