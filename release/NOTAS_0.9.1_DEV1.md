> **AVISO: versão Beta e checkpoint técnico de desenvolvimento. Pode conter bugs. Esta prévia não inclui a interface dos filtros nem conclui a atualização0.9.1.**

## Download

- **Para jogar no PC:** [baixar os dois EXEs da versão atual — 0.9 controles fix1](https://github.com/sugyua/destinia/releases/download/v0.9-controles-fix1/destinia-0.9-beta-windows-x64.zip).
- **Para desenvolvimento:** baixe `destinia-0.9.1-dev1-source.zip` nos Assets abaixo e confira o `.sha256`. Contém a base dos filtros, testes e relatório técnico. **Não contém novos EXEs.**
- **[Jogar no navegador](https://destinia-web-experimental.leafyheron257.chatgpt.site/)** — acesso privado, progresso temporário. Esta publicação não altera a web.

## O que foi preparado desde a0.9

- Base de projeção de equipamentos separada dos registros originais: filtros por tipo/rank/faixa de nível e ordenação por tipo/rank/nível/preço.
- Seleção por identidade do pool na sessão; não escolher outro item automaticamente quando o selecionado sai da visualização.
- Testes ARM nos dois hosts confirmam equipar/cancelar e vender a cópia correta entre espadas com dados idênticos, preservando a equipada e bloqueando sua venda.
- Venda confirmada credita o preço original/5; cancelamento não consome. Registros com quantidade zero são excluídos da projeção de teste.
- Mapeamento documentado para a integração segura: alterar só o índice visual não atualiza necessariamente a prévia que será confirmada.

## Ainda pendente

Interface dos filtros dentro do jogo, integração ao fluxo do jogador e versão web dos filtros. Favoritos, automações e reset por pergaminho também permanecem pendentes. **Nada disso é apresentado como disponível nesta prévia.**

A base executável continua identificada como0.9: VERSION e títulos não foram artificialmente alterados, pois este checkpoint acrescenta base/testes sem integrar novos controles de inventário ao jogo. O ZIP é uma exportação dos arquivos revisados do commit desta tag; não inclui alterações locais, APK, saves, DLLs ou instaladores. Dependências externas: seção Dependências do README e INSTALL.md.

Verificações:2 testes direcionados, incluindo ARM original nos dois hosts, aprovados;7 testes de empacotamento aprovados; integridade e conteúdo do ZIP conferidos. Não há nova CI Windows nem validação física deste checkpoint. Saves reais preservados; cenários usam pastas temporárias.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
