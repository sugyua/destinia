# 0.10.3 Parte 2 — Gameplay e multiplicadores ARM

Data: 26/10/2026. Base: `161667fa8089ae7f004f785ade5188a47d716993` (Parte 1). Branch: `feat/0103-vanguard-difficulty-ui`. Checkpoint para revisão do Sentinel, sem publicação da release completa.

## Contrato e pontos verificados

`common/difficulty_arm.h` consome a dificuldade atual do slot e os perfis padrão de `common/progression.h`, ciclo NG+0. Não modifica schema, XP acumulado no sidecar, crescimento NG+, campanha ou curva de nível. Modos clássico/expandido mantêm seus saves separados. Fingerprint FNV-1a esperado da biblioteca: `74b2547839b88777`; símbolos/offsets são verificados antes de instalar hooks. Inicialização inválida recusa executar com dificuldade silenciosamente incorreta.

| Perfil | HP | Dano | XP | Ouro |
|---|---:|---:|---:|---:|
| Normal | 1 | 1 | 1 | 1 |
| Difícil | 1,5 | 1,5 | 1,3 | 1,3 |
| Pesadelo | 2,5 | 2,5 | 1,8 | 1,8 |
| Inferno | 4 | 4 | 2,5 | 2,5 |
| Abissal | 6 | 6 | 4 | 4 |

- **Escopo:** monstros de campanha da matriz `Game+0x14fc`, até 20 registros de 136 bytes, contagem `Game+0x1f9c`, espécie até 109 e tipo abaixo de 200. Objetos fora da matriz (pets) e tipos de rede ficam excluídos.
- **HP:** antes de `Mon_initMonster` (`0x1740c`), escala máximo `+0x24`; o código ARM original o copia para HP atual `+0x20`. Base guardada por instância impede empilhamento em respawns. `Mon_createMonster`/`GameCanvas_LoadGame` limpam identidade; `GameCanvas_mRun` e `procMonster` sincronizam trocas. Vida parcial conserva a proporção com piso inteiro e mínimo 1 para vivos; HP morto não revive.
- **Dano:** join comum `monsterHitProc+0xfe`, endereço `0x133b2`, registrador R8, antes de exibir/subtrair HP do herói. R7 preserva os flags originais de hit. O ponto anterior `0x133b0/R7` foi descartado após o teste detectar o salto dos acertos comuns.
- **XP:** entrada `GameCanvas_getGainExp` (`0x123f0`) guarda contexto pela pilha; retorno `0x12430` escala R0. `gainExpProc` não recebe um segundo fator; missões/prêmios brutos permanecem originais.
- **Ouro:** contexto na entrada `GameCanvas_dropItem` (`0x16b7c`), valor no ramo ouro em `0x16e96`, antes de estreitar para u16; limpeza em `0x16d6c`. `setMyGold`, compras, vendas, saldo carregado e coleta não são multiplicados.
- **Morte:** dificuldade congelada pela identidade viva antes de atualizar um cadáver. XP/ouro tardios e desbloqueio de Diablo usam esse tier, sem inflar recompensas ao mudar o menu durante animação, e sem perder um abate legítimo em Difícil ao voltar ao Normal.
- **Limites:** aritmética canônica inteira, truncamento por piso; HP/dano positivos dentro de signed32, XP u32 e montes de ouro u16. Valores fora do intervalo são recusados, nunca mascarados. HP é validado antes de persistir uma nova escolha.
- **Abissal e itens:** nenhum hook muda tabela, chance, raridade, quantidade de itens ou estado RNG. O fator de ouro atua somente depois da decisão nativa de gerar um monte de ouro.

## Evidências automatizadas

`DESTINIA_NATIVE_TESTS=1 py -m unittest discover -s tests -p test_difficulty_arm.py -v` executa a biblioteca original com SDL dummy, configurações e saves temporários.

Em quatro execuções (host/host_zoom, clássico/expandido), o fixture de cinco perfis produziu:

| Perfil | HP (base 101) | Dano (base 81) | XP (base 62) |
|---|---:|---:|---:|
| Normal | 101 | 81 | 62 |
| Difícil | 151 | 121 | 80 |
| Pesadelo | 252 | 202 | 111 |
| Inferno | 404 | 324 | 155 |
| Abissal | 606 | 486 | 248 |

- Respawn repetido conserva a base; vida 75/151 vira 300/606, cadáver permanece com HP 0; preflight rejeita `INT32_MAX*6` sem mutação.
- Slot alternativo, cópia fora da matriz e tipo 200 permanecem neutros.
- Acertos com defesa 0/60/500 e estado nativo de bloqueio 0/1 mantêm os fatores finais corretos.
- XP recebido por `gainExpProc` é o valor ajustado uma vez; prêmio bruto de 17 continua 17. `setMyGold(12345)` continua 12345. Coleta credita o valor gerado uma vez, em Normal e Abissal.
- 256 seeds iguais Normal/Abissal por host: **98 casos de ouro, 109 de itens e 49 sem drop**. Flags e estado final RNG iguais, cada registro de item igual byte a byte, ouro exatamente 4x. Isto comprova equivalência nessa amostra determinística, não estima probabilidades populacionais.
- XP/ouro de mortos Normal→Abissal, Difícil→Normal e Abissal→Normal permanecem no perfil da morte. Diablo morto em Normal não ganha abate de Difícil ao trocar menu; abate em Difícil continua liberando Pesadelo após voltar ao Normal.
- Carregamento real do mapa com RNG igual: HP nativo 240 em Normal e 1440 em Abissal, sem acumular no respawn. Boss 108 também passa pelo inicializador nativo com fixture controlado.

## Regressões e builds

- Parte 1: `test_difficulty.py` aprovado, incluindo seis execuções ARM de UI nos dois hosts e escalas 75/100/125, mais regras/persistência.
- `test_combat_power.py`, `test_loot_qol.py`, `test_boss_mage.py` e `test_codex_native.py`: aprovados nos dois hosts; mantêm atributos/comparador, coleta, Mago/Boss Rush e navegação da barra.
- Suíte geral Python executada nesta Parte 2: **64 casos, 24 aprovados, 34 ignorados, 5 erros e 1 falha**. Mesmos bloqueios da Parte 1: os cinco métodos de `test_compile_windows.CompileWindowsTests` não encontram bash no PATH do Python Windows; `test_package_windows.WindowsPackageTests.test_runtime_allowlist_and_rejects_wrong_machine` rejeita DLLs incluídas pelo empacotador existente. Os nomes dos cinco métodos e a descrição estão no relatório da Parte 1; esses arquivos/scripts não foram alterados. Não se declara suíte geral verde. Os testes nativos direcionados acima foram executados separadamente, com `DESTINIA_NATIVE_TESTS=1`.
- Builds locais CMake/Ninja dos dois hosts: aprovados; ambos respondem `Destinia 0.10.3 Beta` em `--version`. Não são builds CI nem gameplay físico.
- Testes novos adicionados ao workflow de candidata Windows. A configuração foi atualizada, mas o workflow remoto não foi disparado neste checkpoint local.

## Limitações e entrega

A prova de Diablo usa fixture na matriz de monstros; um save temporário sem progresso de história não comprova o aparecimento do chefe em sua etapa da campanha. Não foi jogada a campanha completa nem testado controle físico no Windows. Dano ambiental/efeitos sem origem nesse caminho de ataque, pets e PvP mantêm seu comportamento original.

Revisão independente de apoio examinou código/disassembly e não apontou novos problemas importantes após as correções; não substitui o review solicitado do Sentinel. Builds e regressões locais são registrados no checkpoint da central; CI no SHA final ainda não executada.

README, CHANGELOG, manifesto e notas atualizados; VERSION permanece coerente em 0.10.3. Sem tag, pacote ou Release 0.10.3 neste checkpoint. Web https://destinia-web-experimental.leafyheron257.chatgpt.site/ preservada, sem deployment desta Parte 2 PC. Dependências externas permanecem documentadas em README/INSTALL, sem ativos, DLLs ou saves pessoais no commit.
