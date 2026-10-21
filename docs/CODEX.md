# Códex de monstros — 0.7 Beta

> **O jogo está em versão Beta e pode apresentar bugs. Faça backup dos saves.**

**Acesso corrigido na branch `main` e no site após a publicação da 0.7.** Os ZIPs originais da tag `v0.7-beta` continuam com o acesso antigo; compile os fontes atuais ou use os pacotes de teste de Actions para conferir esta mudança no Windows.

Durante a partida, abra o **menu do próprio jogo**, o mesmo que contém Craft e Pets. Toque/clique no livro **CODEX**, à direita da barra de ícones. A lista, os contadores e a paginação são desenhados dentro da imagem do jogo; nenhum botão do Códex ocupa a tela de gameplay ou a interface externa do site.

No Windows, use **I** (menu por padrão), o comando de menu configurado ou o botão original. Na web, toque em **Menu** ou no botão original. **VOLTAR**, Esc ou o comando de voltar retorna à opção do menu que estava selecionada. Os botões **ANTERIOR/PROXIMA** ou setas mudam a página. No menu principal, esquerda a partir da primeira aba ou direita a partir da última seleciona o livro; confirmar abre. O atalho F8 só abre com esse menu disponível e sem remapeamento de F8. A partida fica suspensa durante a consulta, preservando a pausa manual anterior.

- **VISTO:** uma espécie foi desenhada pelo jogo, com o centro do monstro dentro da área visível. Não basta carregar os inimigos do mapa.
- **DERROTADO:** a rotina original de confirmação de derrota foi observada, com o monstro em estado de morte e sem vida.
- **MORTOS:** quantidade de derrotas confirmadas daquela espécie. Repetir o processamento do mesmo cadáver não soma outra morte; um novo indivíduo ou reaparecimento pode somar.

Na `main` e no site, cada entrada mostra uma **miniatura original do monstro**, seu nome em inglês, estado e contador; equipamentos continuam pendentes. Quatro monstros por página mantêm as imagens legíveis. Somente espécies descobertas aparecem. O Códex é aberto pelo menu durante a partida e fica vazio até haver descobertas.

## Onde os dados ficam

No Windows, `codex-v1.txt` fica junto de `save0.dat`, `save1.dat` etc., na pasta global de saves configurada. O padrão continua sendo `%LOCALAPPDATA%\Destinia\save`. Copiar somente o EXE ou atualizar a instalação preserva os registros se a mesma pasta continuar configurada. Os backups da 0.5/0.6 incluem o arquivo auxiliar quando ele estiver presente.

O Códex usa o slot de personagem informado pelo jogo. A exclusão bem-sucedida de `saveN.dat` pelo jogo também limpa os registros desse slot; substituir arquivos manualmente ou importar um personagem diferente para o mesmo slot exige manter o `codex-v1.txt` correspondente, pois esta versão não descobre a identidade de um personagem importado. Não misture arquivos de instalações diferentes sem backup.

A gravação usa arquivo temporário e substituição do auxiliar, sob a trava global já existente. Um auxiliar inválido é preservado e desativa a gravação do Códex naquela sessão, com aviso no log. A partida continua; as novas descobertas ficam apenas em memória. O arquivo ARM original não é regravado pelo Códex.

**Não há histórico retroativo:** a contagem começa a partir da 0.7. Zero significa nenhuma morte registrada pelo novo contador, e não prova que você nunca derrotou aquela espécie. Fechar o jogo normalmente grava as alterações; uma interrupção pode perder os eventos desde a última gravação periódica do auxiliar. O Códex registra eventos da sessão, incluindo os de uma partida encerrada sem salvar o progresso ARM; carregar um save anterior não desfaz as contagens acumuladas.

## No iPhone / navegador

Abra o [site privado](https://destinia-web-experimental.leafyheron257.chatgpt.site) com a mesma conta ChatGPT e inicie a partida e abra **Menu → CODEX**, dentro da imagem do jogo. **VOLTAR** retorna ao mesmo menu. O botão e o diálogo externos da versão inicial foram removidos.

Na web, jogo e Códex continuam apenas na memória da sessão: recarregar, reiniciar ou fechar a aba apaga ambos. Não compartilha o progresso do Windows e continua sem áudio. Safari/iPhone físico e campanha completa ainda não foram verificados.

## Compatibilidade e testes

O observador Windows só liga para a biblioteca original examinada, SHA-256 `F12F1D20AB42646EF81DF27516851ACCCB9FFFE7BE07A054DB83890D992DF7E9`; a seleção no host usa uma impressão FNV-1a do arquivo inteiro como verificação de compatibilidade, não como mecanismo de segurança. As estruturas são lidas, sem alterar combate, XP, drops ou o formato do save.

Testes cobrem modelo, deduplicação, reaparecimento, slots, escrita/releitura, recusa de dados inválidos, callback ARM de desenho/derrota em estruturas controladas e exclusão de inimigo fora da tela. Os testes de navegador verificam botão, diálogo, contador com fixture ARM, pausa/retomada e retrato/paisagem. Esses testes não comprovam a campanha completa nem a atribuição de todas as formas de dano e chefes; reporte divergências com o log e um save de teste.

## Verificação da correção do menu

A biblioteca ARM e os nove IDs originais de menu são preservados: o livro é uma extensão do host ligada à tela original de menu, sem substituir Craft/Pets ou gravar um ID novo na memória ARM. Retornar preserva a opção anterior. Os testes de ABI conferem abertura pelo toque, navegação entre as abas e o livro, volta, isolamento de submenus e ausência de botão no gameplay. O teste web confirma que a memória do jogo não avança durante a consulta e que não há diálogo/botão Códex no HTML externo. Chromium com toque verificou retrato, paisagem, contador via fixture ARM e pausa/retomada; Safari físico e campanha completa continuam pendentes.

## Miniaturas originais — alteração após a release 0.7

Os retratos usam os arquivos `monN_V_head.wmb`/`bossN_head.wmb` e as poses `monN.agd`/`bossN.agd` já presentes em `game/assets`. O leitor combina as partes do corpo da primeira pose, sem sombras/efeitos externos, preserva a transparência e reduz a figura proporcionalmente em uma área de 28×28 pixels. As 63 variantes normais e os dez IDs de chefes foram lidos nos testes. O chefe 103 reutiliza a arte do 101, como no carregador original.

No Windows, as imagens são carregadas em cache para as páginas consultadas, sem chamar rotinas ARM nem alterar partida ou `codex-v1.txt`. A leitura de ZCP usa zlib externa, documentada em Dependências. No navegador, as imagens são preparadas durante a inicialização a partir dos recursos já presentes; é necessário suporte a `DecompressionStream` para vê-las. Recurso ausente, corrompido ou formato incompatível mostra `?`, preservando nome/estado/contador. A leitura limita tamanho descomprimido, dimensões e índices antes de desenhar.

As referências de teste incluem figuras completas de Fungus, Scorpion, Dungeon Guardian, chefe 102 e Lava Guardian, verificadas também por hash dos pixels RGB565, além de todas as variantes. Consulta e mudança de página preservam os contadores e a memória ARM. A QA por toque verifica duas páginas, inclusive chefes, em retrato/paisagem no Chromium. Isso não substitui a verificação no Windows com jogo real nem no Safari/iPhone físico.
