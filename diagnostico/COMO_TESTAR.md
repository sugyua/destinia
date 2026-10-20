# Teste geral do Destinia

Execute **Testar Destinia.bat**, na raiz do projeto, com o jogo fechado. O BAT chama `diagnostico/test_suite.ps1` pelo Windows PowerShell 5.1. Nao precisa instalar Python, PowerShell 7 ou Pester para usar o diagnostico.

Sao necessarios os executaveis compilados em `host/build/destinia_host.exe` e `host_zoom/build/destinia_host.exe`, os dados originais em `game` e as dependencias do jogo instaladas separadamente, conforme o README. Se faltar um executavel, o script informa a falha; nao compila nem baixa arquivos. O caminho padrao de runtime e `C:\msys64\ucrt64\bin`.

Ao baixar o pacote de testes, extraia todos os arquivos **na raiz do projeto existente**, mantendo a subpasta diagnostico. O pacote nao inclui o jogo, executaveis nem dependencias externas.

## Duplo clique: modo guiado

1. Feche todas as janelas do Destinia e abra o BAT.
2. Aguarde as verificacoes automaticas, com 60 quadros sem janela e capturas BMP por host.
3. Digite S quando solicitado para abrir o jogo sobre uma copia do save em uma pasta nova de teste.
4. Confira iniciar/carregar partida, movimento, combate, inventario e equipamento.
5. Troque de mapa e observe a camera. Teste mouse, redimensionamento da janela, DPI 100/150% no Windows, audio, controle/desconexao e Alt+Tab segurando uma direcao. O script nao muda o DPI nem outras configuracoes do Windows.
6. Salve e feche a janela. Responda S, N ou P nas perguntas: passou, falhou ou nao testado.
7. Escolha reabrir a mesma copia para confirmar o progresso. Feche o jogo novamente e responda sobre o save.

Em host_zoom, o teste ativa PT e o trace de viewport. Na falha visual, pressione F9 antes da transicao, atravesse a borda e pressione F9 para parar. Pressione F9 outra vez para capturar o estado posterior e novamente para encerrar. Os arquivos ficam na pasta da sessao.

Compras e rede nao sao acionadas automaticamente. Ainda podem travar ou alterar cash local; exigem teste separado e voluntario. O BAT nao injeta fread/fwrite de tamanho zero ou ELF corrompido: esses problemas continuam pendentes de testes de codigo.

## Opcoes pelo Prompt de Comando

Na pasta do projeto:

```bat
"Testar Destinia.bat" -Mode Automatico
"Testar Destinia.bat" -Variant host_zoom
"Testar Destinia.bat" -RuntimeDir "D:\msys64\ucrt64\bin"
"Testar Destinia.bat" -SaveSource "C:\copia-do-save" -Variant host
"Testar Destinia.bat" -Fresh
```

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

Cada sessao cria `relatorios/testes_DATA_HORA_ID/`. Ha copias separadas para os testes automaticos e guiados de cada host. Salvar/reabrir reutiliza apenas a copia guiada. Nenhuma pasta antiga de teste e sobrescrita.

O script compara hashes da origem antes/depois. Nao restaura nem migra saves. Nao abra outra instancia sobre o save original durante o teste. Saves com links simbolicos/junctions sao recusados para evitar copiar caminhos externos.

Arquivos gerados:

- `relatorio.txt`: resumo, inclusive erros de arquivos/dependencias ausentes.
- `resultados.json`: resultados estruturados.
- `*.stdout.log` e `*.stderr.log`: logs das execucoes.
- `*_frames/`: BMPs de inicializacao.
- `trace_viewport/`: logs/BMPs de F9, quando usados em host_zoom.
- `*_save_*`: copias de save; **nao enviar essas pastas**.

`OK` automatico confirma somente a verificacao indicada; `MANUAL_OK` e uma resposta sua. `PENDENTE` e `NAO_TESTADO` nao significam aprovado. BMPs sao verificados quanto a formato, dimensoes e integridade basica, nao a correcao visual da cena. Como o host pode retornar zero apos uma falha, o teste tambem exige inicializacao/contagem no log, procura erros conhecidos e verifica as capturas esperadas.

Codigo final 1 indica falha automatica ou manual. Codigo 0 significa que nenhuma falha foi registrada; ainda pode haver pendencias. Revise relatorio/logs/capturas antes de compartilhar. Nada e enviado a internet e nenhuma dependencia e instalada.

## Verificacao desta ferramenta

Funcoes testadas com PowerShell 7 em Linux: isolamento e copia de saves, deteccao de erro com exit zero, BMP ausente/incorreto/truncado, captura real de stdout/stderr e timeout do proprio processo. O duplo clique em CMD/Windows PowerShell 5.1 e o gameplay real ainda precisam ser confirmados no Windows. Nenhuma correcao do host e anunciada por esses testes.
