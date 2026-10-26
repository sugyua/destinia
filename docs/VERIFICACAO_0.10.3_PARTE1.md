# Verificação 0.10.3 — primeira parte PC

Data: 26/10/2026. Base: d82b7ac4eff8d323391b3cf0ae5f9c79ea085610. Branch: `feat/0103-vanguard-difficulty-ui`. Checkpoint solicitado pelo autor; não representa publicação da release completa.

## Mudança

Sistema preserva suas cinco ações e acrescenta Mudar Dificuldade. Cinco opções com bloqueadas cinza; SAIR/Voltar; teclas, comandos remapeados/controle pelo adaptador existente e toque. Indicador na chamada ARM original de classe (`MenuCanvas_drawMenuWin`, retorno 0x21c7a), condicionado à existência de save do slot. Recursos visuais e fonte nativos; `Dificil` sem acento pela limitação da fonte original.

Diablo (108), confirmado pelo autor, libera somente a dificuldade seguinte. O mesmo cadáver não pode liberar outro nível depois da troca; respawn/mapa iniciam nova identidade. Não há conversão de mortes antigas do Códex. Um `progression-v1.txt` preexistente preserva sua dificuldade atual; em ausência de sidecar de desbloqueios, esse patamar é mantido como teto, sem liberar o seguinte.

`progression-v1.txt` mantém o schema existente, XP e NG+. `difficulty-unlocks-v1.txt` armazena o teto por slot. Escrita conjunta via SaveTransaction, snapshots CAS e trava de progressão. A exclusão nativa de `saveN.dat` inclui a limpeza dos dois sidecars na mesma transação; falhas revertem o lote. Abates pendentes são repetidos durante a sessão em caso de falha temporária. Pedidos de troca capturam o slot de origem.

## Testes executados

- `test_difficulty.py`: regras, isolamento dos slots, cinco dificuldades, abate incorreto, morte repetida, XP/NG+, roundtrip, CAS obsoleto, dados inválidos e rollback quando falha a primeira ou segunda substituição do lote.
- Provas ARM originais dos dois hosts (400x240 e 800x480), escalas 75/100/125, em seis execuções SDL dummy com configurações/saves temporários: caminho MainMenu, fonte chamada pelo BL original, slots vazio/ocupado, toque e fechamento, cinza/bloqueio, Diablo, mesmo cadáver, retry após trava, pedido atrasado de outro slot, exclusão real via import `remove`, pixels sentinela e preservação da memória do personagem durante o desenho.
- `test_progression.py`: C++ e 7 casos Node aprovados; contrato canônico preservado. `test_package_release.py`: 7 testes aprovados (fontes curadas, exclusão de dados e integridade do manifesto).
- `test_codex_native.py`: regressão da barra/Códex nos dois hosts.
- `test_boss_mage.py`: regressão de Mago, Boss Rush, mapas/paleta e barra.
- Builds Windows locais por CMake/Ninja dos dois hosts. Não são compilação CI de release.
- Suíte geral Python: 63 testes, 24 aprovados, 33 ignorados, 5 erros e 1 falha. Os cinco métodos de `test_compile_windows.CompileWindowsTests` não encontraram `bash` no PATH deste Python Windows (`test_build_failure_is_returned_and_stops_second_host`, `test_builds_both_variants`, `test_builds_zoom_in_project_path_with_spaces`, `test_configuration_failure_prevents_build`, `test_invalid_target_does_not_invoke_cmake`). `test_package_windows.WindowsPackageTests.test_runtime_allowlist_and_rejects_wrong_machine` rejeita DLLs que o empacotador existente inclui. Estes arquivos não foram modificados nesta etapa; não se declara suíte geral verde.

## Limitações e publicação

Esta primeira parte implementa UI/persistência. Não conecta nem muda multiplicadores ARM de combate, reset de campanha ou NG+. Não comprova matar Diablo na campanha completa nem controle físico: os callbacks ARM foram exercitados com fixtures temporárias, e as capturas usam SDL dummy. Se o processo encerrar enquanto a gravação de um abate permanece falhando, o evento em memória não é uma liberação persistida.

Release 0.10.3 não publicada neste checkpoint; sem upload de EXEs ou pacotes. Web https://destinia-web-experimental.leafyheron257.chatgpt.site/ preservada, sem alteração/deployment 0.10.3. Paridade web, CI no SHA final, revisão de pacotes e validação física permanecem para a entrega completa.
