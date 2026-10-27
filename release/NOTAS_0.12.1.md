# Destinia 0.12.1 Beta — coerência da distribuição e atalhos Web

> Beta: pode apresentar bugs e travamentos. Faça backup dos saves antes de atualizar.

- Corrigido encaminhamento de F4 (Catálogo), F5 (Baú) e F8 (Bounties) no teclado Web, preservando Tab e os comandos anteriores. Teclas repetidas não reabrem painéis; F5 não recarrega a partida carregada durante pausa.
- README, banner Web, VERSION e manifesto alinhados; roadmap histórico identificado como tal.
- Empacotador Windows inclui obrigatoriamente as notas selecionadas por VERSION. Dependências externas ficam fora do pacote, conforme a diretriz do autor; instale MSYS2 UCRT64, SDL2, SDL2_mixer, Unicorn, zlib e dependências transitivas conforme README/INSTALL.
- Fontes curados excluem game, bibliotecas originais, saves e arquivos pessoais. Releases anteriores preservadas.

## Validação e limites

Resultados executados serão registrados em `docs/VERIFICACAO_0.12.1.md`. Testes automatizados não comprovam campanha completa, gameplay físico, Safari/iPhone ou controle USB.

Web: fontes 0.12.1; deployment no site privado existente não concluído: a chamada Sites devolveu ferramenta indisponível nesta sessão. URL preservada: https://destinia-web-experimental.leafyheron257.chatgpt.site/.

Issue de origem: https://github.com/sugyua/destinia/issues/5. Esta entrega cobre versão/pacote e F4/F5/F8; demais propostas da issue continuam pendentes.
