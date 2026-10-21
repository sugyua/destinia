# Teste geral do Destinia

Execute **Testar Destinia.bat**, na raiz do projeto, com o jogo fechado. O BAT chama `diagnostico/test_suite.ps1` pelo Windows PowerShell 5.1. Nao precisa instalar Python, PowerShell 7 ou Pester para usar o diagnostico.

Sao necessarios os executaveis compilados em `host/build/destinia_host.exe` e `host_zoom/build/destinia_host.exe`, os dados originais em `game` e as dependencias do jogo instaladas separadamente, conforme o README. Se faltar um executavel, o script informa a falha; nao compila nem baixa arquivos. O caminho padrao de runtime e `C:\msys64\ucrt64\bin`.

Ao baixar o pacote de testes, extraia todos os arquivos **na raiz do projeto existente**, mantendo as subpastas diagnostico e scripts (incluindo scripts/shared_saves.ps1). O pacote nao inclui o jogo, executaveis nem dependencias externas.

## Teste da correcao de camera do host_zoom

No duplo clique, escolha **1 — Camera host_zoom**. A opcao 2 conserva o roteiro geral. Argumentos pelo Prompt continuam funcionando sem menu.

Na candidata 0.5 os BMPs do zoom continuam sendo 800x480, como na 0.4. Mapas pequenos devem estar centralizados, com margens cinzas; confira UI menor e tamanho de XP.

Este modo verifica a inicializacao apenas do host_zoom, abre uma copia isolada do save e ativa `--trace-camera` junto com o trace de viewport. A correcao de enquadramento exige recompilar host_zoom com os fontes atuais; consulte INSTALL.md. Um EXE sem identificacao de mapas pequenos centralizados e recusado. Abre em tela cheia proporcional; F11 alterna para janela. Confira o alinhamento do mouse e Alt+Tab. Nao solicita os testes gerais nem reabertura do save.

1. Clique em TAP TO START e carregue a partida. Va ate a passagem com defeito.
2. Pressione **F9 antes da transicao**: captura a tela e inicia a gravacao.
3. Atravesse e, com o defeito visivel, pressione **F9 para parar**.
4. Pressione **F9 novamente, ainda na tela com defeito**, para capturar o estado posterior. Aguarde alguns segundos e pressione F9 para encerrar.
5. Feche o jogo normalmente. Responda se a camera funcionou (N se houve defeito) e descreva a passagem e o problema.

O relatorio informa quantos BMPs foram encontrados e se existem linhas de camera no log. A contagem nao prova que as imagens correspondem a antes/depois; essa comparacao continua manual. Menos de duas imagens fica PENDENTE. Nao abra a loja neste teste.

Envie `relatorio.txt`, `resultados.json`, os logs e `trace_viewport` da nova sessao. **Exclua as pastas com save** antes de compactar; nao precisa enviar sessoes antigas. A pasta exata aparece no inicio e no final do BAT.

## Roteiro geral (opcao 2)

1. Feche todas as janelas do Destinia e abra o BAT.
2. Aguarde as verificacoes automaticas, com 60 quadros sem janela e capturas BMP por host.
3. Digite S quando solicitado para abrir o jogo sobre uma copia do save em uma pasta nova de teste.
4. Confira iniciar/carregar partida, movimento, combate, inventario e equipamento.
5. Troque de mapa e observe a camera. Teste mouse, redimensionamento da janela, DPI 100/150% no Windows, audio, controle/desconexao e Alt+Tab segurando uma direcao. O script nao muda o DPI nem outras configuracoes do Windows.
6. Salve e feche a janela. Responda S, N ou P nas perguntas: passou, falhou ou nao testado.
7. Escolha reabrir a mesma copia para confirmar o progresso. Feche o jogo novamente e responda sobre o save.

Em host_zoom, o teste ativa PT e o trace de viewport. Na falha visual, pressione F9 antes da transicao, atravesse a borda e pressione F9 para parar. Pressione F9 outra vez para capturar o estado posterior e novamente para encerrar. Os arquivos ficam na pasta da sessao.

Compras e rede nao sao acionadas automaticamente. Se voce abrir esses menus durante o teste, o diagnostico passa a destacar mudancas de cash e o ultimo estado de rede observado no log. `rede=1` no fim gera AVISO de operacao aguardando resposta; nao e prova de crash ou espera infinita. Se a ultima sonda voltar a `rede=0`, nao anuncia uma espera ainda pendente. Mudanca de cash nao comprova gravacao em arquivo nem sucesso de compra online.

Esses menus ainda podem travar ou alterar cash local; exigem teste separado e voluntario. O BAT nao injeta fread/fwrite de tamanho zero ou ELF corrompido: esses problemas continuam pendentes de testes de codigo. Nao e necessario abrir a loja para completar o roteiro de gameplay.

## Opcoes pelo Prompt de Comando

Na pasta do projeto:

```bat
"Testar Destinia.bat" -Mode Camera
"Testar Destinia.bat" -Mode Automatico
"Testar Destinia.bat" -Variant host_zoom
"Testar Destinia.bat" -RuntimeDir "D:\msys64\ucrt64\bin"
"Testar Destinia.bat" -SaveSource "C:\copia-do-save" -Variant host
"Testar Destinia.bat" -Fresh
```

- `-Mode Camera`: teste guiado focado, seleciona somente host_zoom e registra camera/viewport.
- `-Mode Automatico`: sem janelas de gameplay ou perguntas. O BAT ainda pausa no final para leitura.
- `-Variant host`, `host_zoom` ou `Ambos`: seleciona os executaveis.
- `-SaveSource`: origem para copiar. Por padrao, procura `%LOCALAPPDATA%\Destinia\save`. Se a origem padrao nao existir, usa saves vazios e informa o aviso. Origem explicita inexistente e erro.
- `-Fresh`: saves novos, sem copiar progresso.
- `-Frames 60`: quadros sem janela, de 10 a 1000.
- `-TimeoutSeconds 120`: limite por teste automatico, ajustavel ate 600 para PCs lentos.
- `-InteractiveTimeoutSeconds 900`: limite por janela guiada/reabertura, ate 3600 segundos. No limite, apenas o processo criado pelo teste e encerrado, e o resultado e falha.

Para executar sem a pausa final do BAT:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\diagnostico\test_suite.ps1 -Mode Automatico
```

ExecutionPolicy vale apenas para esse processo, sem mudar a politica do sistema.

## Saves e resultados

A origem padrao e obtida da configuracao global de save: variavel DESTINIA_SAVE_DIR, config.ini e depois LOCALAPPDATA/Destinia/save. -SaveSource tem prioridade e -Fresh pede uma partida nova de teste. O diagnostico continua passando --save para os clones, sem usar a pasta principal diretamente.

Cada sessao cria `relatorios/testes_DATA_HORA_ID/`. Ha copias separadas para os testes automaticos e guiados de cada host. Salvar/reabrir reutiliza apenas a copia guiada. Nenhuma pasta antiga de teste e sobrescrita.

O relatorio e criado no inicio, atualizado apos cada resultado e antes de abrir janelas ou fazer perguntas. Enquanto a sessao nao terminar, o texto informa PARCIAL e o JSON registra `Concluido=false`, a etapa atual e `Codigo=null`. Se fechar o terminal, os resultados ja gravados permanecem; nao devem ser tratados como uma sessao aprovada. Cada arquivo e substituido por uma nova versao completa, evitando salvar JSON pela metade durante atualizacoes normais.

Na candidata 0.5, os TXT automaticos do host ficam em `logs_do_jogo/jogo_HOST_DATA/log.txt` dentro da sessao e sao atualizados durante o jogo, com retencao limitada. Esses TXT sao independentes da captura externa. Os logs stdout/stderr do processo continuam sendo gravados ao encerrar cada execucao. Um fechamento abrupto do terminal pode perder os logs dessa ultima execucao, mesmo com o relatorio parcial preservado. A deteccao de compra/rede e feita apos o retorno do processo, nao em tempo real.

O script compara hashes da origem antes/depois. Nao restaura nem migra saves. Nao abra outra instancia sobre o save original durante o teste. Saves com links simbolicos/junctions sao recusados para evitar copiar caminhos externos.

Arquivos gerados:

- `relatorio.txt`: resumo, inclusive erros de arquivos/dependencias ausentes.
- `resultados.json`: resultados estruturados.
- `*.stdout.log` e `*.stderr.log`: logs das execucoes.
- `*_frames/`: BMPs de inicializacao.
- `trace_viewport/`: logs/BMPs de F9, quando usados em host_zoom.
- `logs_do_jogo/`: logs TXT automaticos da candidata 0.5; podem ser enviados para analise.
- `*_backups/`: snapshots dos clones de teste; contem progresso e nao precisam ser enviados.
- `*_save_*`: copias de save; **nao enviar essas pastas**.

`OK` automatico confirma somente a verificacao indicada; `MANUAL_OK` e uma resposta sua. `PENDENTE` e `NAO_TESTADO` nao significam aprovado. BMPs sao verificados quanto a formato, dimensoes e integridade basica, nao a correcao visual da cena. A 0.5 corrige os caminhos de falha emulada testados, mas codigo zero nao e suficiente para validar tudo; por isso o teste tambem exige inicializacao/contagem no log, procura erros conhecidos e verifica as capturas esperadas.

Codigo final 1 indica falha automatica ou manual. Codigo 0 significa que nenhuma falha foi registrada; ainda pode haver AVISOS ou pendencias, destacados no resumo. Codigo 2 e reservado a uma interrupcao detectada antes da conclusao. Um relatorio parcial com `Codigo=null` nao e sucesso. Revise relatorio/logs/capturas antes de compartilhar. Nada e enviado a internet e nenhuma dependencia e instalada.

## Verificacao desta ferramenta

Funcoes testadas com PowerShell 7 em Linux: isolamento e copia de saves, deteccao de erro com exit zero, BMP ausente/incorreto/truncado, captura real de stdout/stderr e timeout do proprio processo. Tambem foram testados os avisos de compra/rede, a atualizacao do relatorio e sua preservacao apos encerrar um processo de teste. O roteiro geral ja foi exercitado pelo usuario no Windows. O novo menu BAT e o modo focado em camera ainda precisam de confirmacao no Windows PowerShell 5.1. O fluxo Camera tambem foi exercitado com processo/janela e respostas simulados: somente zoom, falha manual, ausencia de capturas, sem reabertura e com save original preservado. Nenhum jogo foi executado nesses testes. Nenhuma correcao do host e anunciada por esses testes.


## Candidata 0.6

Use o diagnóstico e os EXEs da mesma candidata. O roteiro continua usando cópias dos saves e agora envia `--no-save-preferences`, para não guardar mudanças de tamanho/modo de janela do teste na configuração pessoal. Pausa, áudio e perfis seguem a configuração global escolhida; use `Configurar Destinia.bat` com os jogos fechados para ajustá-los. O roteiro guiado não equivale aos testes de código do workflow Windows.
