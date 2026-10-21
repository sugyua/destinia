# Códex de monstros — 0.7 Beta

> **O jogo está em versão Beta e pode apresentar bugs. Faça backup dos saves.**

No Windows, clique em **CODEX F8**, no canto superior direito da imagem do jogo, ou pressione **F8**. Se F8 estiver remapeado nas suas preferências, o comando existente é preservado: use o botão do mouse. F8 ou Esc fecha; setas esquerda/direita ou os botões inferiores mudam a página. A partida e os comandos ficam suspensos durante a consulta; a pausa manual anterior é preservada.

- **VISTO:** uma espécie foi desenhada pelo jogo, com o centro do monstro dentro da área visível. Não basta carregar os inimigos do mapa.
- **DERROTADO:** a rotina original de confirmação de derrota foi observada, com o monstro em estado de morte e sem vida.
- **MORTOS:** quantidade de derrotas confirmadas daquela espécie. Repetir o processamento do mesmo cadáver não soma outra morte; um novo indivíduo ou reaparecimento pode somar.

Esta primeira versão mostra nomes originais em inglês, sem retratos ou equipamentos. Somente espécies descobertas aparecem. O Códex pode ser aberto antes da partida; nesse caso ficará vazio até haver descobertas.

## Onde os dados ficam

No Windows, `codex-v1.txt` fica junto de `save0.dat`, `save1.dat` etc., na pasta global de saves configurada. O padrão continua sendo `%LOCALAPPDATA%\Destinia\save`. Copiar somente o EXE ou atualizar a instalação preserva os registros se a mesma pasta continuar configurada. Os backups da 0.5/0.6 incluem o arquivo auxiliar quando ele estiver presente.

O Códex usa o slot de personagem informado pelo jogo. A exclusão bem-sucedida de `saveN.dat` pelo jogo também limpa os registros desse slot; substituir arquivos manualmente ou importar um personagem diferente para o mesmo slot exige manter o `codex-v1.txt` correspondente, pois esta versão não descobre a identidade de um personagem importado. Não misture arquivos de instalações diferentes sem backup.

A gravação usa arquivo temporário e substituição do auxiliar, sob a trava global já existente. Um auxiliar inválido é preservado e desativa a gravação do Códex naquela sessão, com aviso no log. A partida continua; as novas descobertas ficam apenas em memória. O arquivo ARM original não é regravado pelo Códex.

**Não há histórico retroativo:** a contagem começa a partir da 0.7. Zero significa nenhuma morte registrada pelo novo contador, e não prova que você nunca derrotou aquela espécie. Fechar o jogo normalmente grava as alterações; uma interrupção pode perder os eventos desde a última gravação periódica do auxiliar. O Códex registra eventos da sessão, incluindo os de uma partida encerrada sem salvar o progresso ARM; carregar um save anterior não desfaz as contagens acumuladas.

## No iPhone / navegador

Abra o [site privado](https://destinia-web-experimental.leafyheron257.chatgpt.site) com a mesma conta ChatGPT e toque em **Códex**, no topo da página. Ele abre antes ou depois de iniciar o jogo e pausa a partida durante a consulta. Fechar retoma somente se a partida não já estivesse pausada.

Na web, jogo e Códex continuam apenas na memória da sessão: recarregar, reiniciar ou fechar a aba apaga ambos. Não compartilha o progresso do Windows e continua sem áudio. Safari/iPhone físico e campanha completa ainda não foram verificados.

## Compatibilidade e testes

O observador Windows só liga para a biblioteca original examinada, SHA-256 `F12F1D20AB42646EF81DF27516851ACCCB9FFFE7BE07A054DB83890D992DF7E9`; a seleção no host usa uma impressão FNV-1a do arquivo inteiro como verificação de compatibilidade, não como mecanismo de segurança. As estruturas são lidas, sem alterar combate, XP, drops ou o formato do save.

Testes cobrem modelo, deduplicação, reaparecimento, slots, escrita/releitura, recusa de dados inválidos, callback ARM de desenho/derrota em estruturas controladas e exclusão de inimigo fora da tela. Os testes de navegador verificam botão, diálogo, contador com fixture ARM, pausa/retomada e retrato/paisagem. Esses testes não comprovam a campanha completa nem a atribuição de todas as formas de dano e chefes; reporte divergências com o log e um save de teste.
