# SDD ledger — plan: docs/superpowers/plans/2026-10-23-release-08.md

Autor aprovou o desenho e autorizou publicação sequencial0.8/0.9 em GitHub/web. Execução direta pelo agente principal para eficiência; pesquisas independentes de loja e UI sem editar produto.
Pre-flight: Task1 fornece armazenamento seguro usado por Task3/perfis e posteriormente0.9. Root selecionado/travado antes de recuperar,backup ecarregar. Task2 não deve envolver as gravações de proteção sem passar pelo mesmo callFn. Task4 não pode reutilizar perfil de remapeamento como modo de jogo.
Ruling: helpers das skills não estão instalados nofilesystem; ledger/briefs são mantidos neste plano rastreável no repositório, não inventar execução dos helpers.
Ruling: uma chamada ARM do host delimita lote de stdio; journal recuperável antes de carregar protege interrupções entre substituições. É requisito de recuperar save anterior, não promessa de que múltiplos frames da lógica nativa formem uma única transação.
Task1: em andamento; testes de armazenamento serão escritos antes da implementação.

Task1 — implementação local:
- common/safe_save.h prepara escritas e exclusões em lote porcallFn. Modos rb/wb/ab/r+b mantidos; caminhos fora da pasta, links e journals desconhecidos recusados.
- Recovery executado após trava e antes debackup/carregar. Journal em pasta irmã; rollback de originais após interrupção no meio de substituições ou exclusão+escrita. Marcador inválido/truncado é preservado.
- stdio dos dois hosts passa pelo componente. Falha ARM depois de fclose não publica; writer ainda aberto é abortado sem doubleclose no cleanup. Exclusão de personagem notifica Códex somente após confirmação do lote.
- RED→GREEN comprovados para API ausente, integração ausente, journalcorrompido, writeraberto/doubleclose e exclusão fora de rollback.
Revisão independente encontrou P2remove fora de transação; corrigido com tombstone e regressões nativas/deprocesso e Códex. Sem segunda revisão: executar suíte completa depois do fixpass, conforme skill.
Ruling: proteção comprovada para falha/processo interrompido; fsync de arquivos não equivale a durabilidade comprovada de todos os diretórios após queda de energia. Não prometer recuperação de powerloss sem implementação/teste adicional.
Limites atuais: sem campanha de salvamento real/Windows nesta implementação. UI/loja/perfis/fontes e toda0.9 continuam pendentes. Agentes research_offline_store08 e research_ui_scale08 interrompidos por limite de uso do serviço, com apenas achados estáticos parciais; não tratar pesquisas como completas.

Task1 local verification: python -m unittest discover -s tests -v, DESTINIA_NATIVE_TESTS=1,40testes em102,058s:39aprovados e1renderignoradoporEXE não especificado. O render foi executado separadamente com host_zoom recém-compilado -O2/HAS_SDL_MIXER e aprovado em0,467s; total40verificações aprovadas. Novos testes deexclusão e Códex passaram após o fixP2. diff check sem erros. Nenhum teste Windows ou deployment novo foi executado. Ramo de desenvolvimento preserva trabalho sem marcar0.8 ou0.9 entregues.
