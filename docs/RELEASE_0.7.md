# Preparação e verificações — Destinia 0.7 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs. Faça backup dos saves.**

Publicação solicitada e autorizada pelo autor em 21/10/2026. Escopo ajustado para entregar primeiro o Códex de monstros, incluindo o contador por espécie. Os quatro ajustes maiores de interface antes associados à 0.7 continuam pendentes na meta 0.8; não foram marcados como concluídos.

## Alterações desde a 0.6

- Modelo compartilhado de descobertas, estados visto/derrotado e contador de mortes por espécie e slot.
- Ponte passiva para `drawMonsterOddEff`, `checkQuestMonster`, `Mon_initMonster`, `Mon_createMonster` e `GameCanvas_LoadGame`, com validação da biblioteca original e deduplicação por instância/reaparecimento.
- Botão no canto superior direito, F8 quando livre, painel com paginação e pausa durante a consulta; preserva mapeamentos e a pausa anterior.
- `codex-v1.txt` junto ao save compartilhado, gravação periódica/ao sair com substituição por temporário, proteção de arquivo inválido e reinício do slot excluído pelo jogo.
- Ajuste do script de compilação: impor o PATH UCRT64 somente quando esse ambiente existe; fora do MSYS2, preservar o PATH de quem chama. Corrige interferência de ferramentas do sistema nos testes de compilação.
- Versão 0.7, documentação em português, empacotamento revisado e workflow de testes/publicação.
- Protótipo web separado atualizado com botão Códex, pausa, lista e contador em memória da sessão; não acompanha o ZIP Windows nem substitui o port nativo.

## Verificações

Executar a suíte completa de Python, com `DESTINIA_NATIVE_TESTS=1`, SDL2, Unicorn, SDL2_mixer e dados originais. `test_codex.py` cobre persistência e deduplicação; `test_codex_native.py` executa rotinas ARM reais em estruturas controladas nos dois hosts. `DESTINIA_ZOOM_EXE` habilita também a captura real 800×480.

O workflow Windows compila os dois EXEs com áudio, executa os testes nativos de estabilidade, preferências e Códex e os cinco roteiros PowerShell já existentes. Somente após sucesso empacota fontes/Windows e publica a prerelease `v0.7-beta` com ZIPs e SHA-256. As dependências externas e os dados originais permanecem fora dos pacotes. O resultado de cada execução está no GitHub Actions; não confundir preparação do workflow com execução concluída.

A web passou nos oito testes Node e no teste Chromium de Códex com toque, retrato/paisagem e pausa/retomada. A linha de monstro usada para validar o contador veio de fixture de memória com callbacks ARM originais; não é uma morte de campanha nem dado inventado no produto.

## Limitações

Campanha completa, inspeção visual em Windows 3440×1440, controle físico e Safari/iPhone físico continuam pendentes. Nomes originais em inglês; sem retratos, armas/armaduras no Códex ou importação de mortes antigas. O contador pode incluir mortes de uma sessão que depois foi encerrada sem salvar o progresso ARM; é um registro acumulado de eventos. Importar outro personagem manualmente para o mesmo slot pode exigir o auxiliar correspondente. Nenhuma melhoria de loja/rede ou expansão de conteúdo foi implementada nesta versão.

## Correção posterior: Códex no menu original

Após a publicação, o autor solicitou substituir o acesso externo pelo menu do jogo, junto de Craft e Pets. A branch `main` e o site passam a usar o livro CODEX na barra do menu, painel no framebuffer e retorno à mesma opção. A tag e os ZIPs originais da 0.7 não são regravados. O workflow `verify-codex-menu.yml` compila/testa ambos os hosts Windows e prepara pacotes de teste em Actions, sem publicar uma nova release. Veja `docs/CODEX.md`.
