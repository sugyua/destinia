# Saves, backups e logs — 0.5 Beta

> **Versão Beta: faça backup do progresso antes de testar.**

## Continuar a mesma partida entre updates

No Windows, os dois hosts usam por padrão a mesma pasta, independente de onde o update foi extraído:

```text
%LOCALAPPDATA%\Destinia\save
```

Esse padrão também é usado nos fontes publicados da 0.2, 0.3 e 0.4. Atualizar os fontes e recompilar não exige começar uma partida nova. Use **Abrir Destinia.bat** ou **Abrir Destinia Zoom 800x480.bat** para jogar normalmente.

**Testar Destinia.bat usa cópias descartáveis:** o progresso jogado durante o diagnóstico fica na cópia daquela sessão, não no save principal. Se sua partida anterior foi feita em um teste, localize a pasta de save específica daquela sessão; não selecione a pasta `relatorios` inteira como save.

## Escolher uma pasta comum diferente

1. Feche todas as versões do jogo.
2. Abra **Configurar pasta de saves.bat**.
3. Cole o caminho completo da pasta que já contém os arquivos da sua partida, ou pressione Enter para manter o padrão.
4. Abra o jogo pelo inicializador normal ou zoom. O console informa o caminho efetivamente usado. Os inicializadores verificam a identificação do EXE e pedem recompilação se ele ainda for antigo.

O configurador apenas guarda o caminho; não copia, apaga, mescla ou sobrescreve saves. A escolha vale para os dois hosts 0.5 e para as instalações futuras que respeitem essa configuração. Não depende de copiar o arquivo de configuração junto de cada update.

Se quiser compartilhar também com os inicializadores antigos da 0.2–0.4, **mantenha o caminho padrão**. Essas versões antigas não leem a configuração nova. Para trazer uma partida de outra pasta ao padrão, com todos os jogos fechados, preserve uma cópia da pasta de origem e de qualquer save já existente no destino antes de copiar os arquivos completos; não mescle partidas diferentes.

A configuração global é `%LOCALAPPDATA%\Destinia\config.ini`, em UTF-8:

```ini
save_dir=C:\Jogos\DestiniaSaves
```

A ordem de prioridade é:

1. `--save PASTA` informado ao executável, usado pelo diagnóstico para isolar testes.
2. Variável de ambiente `DESTINIA_SAVE_DIR`.
3. `save_dir` no arquivo global `config.ini`.
4. Pasta padrão `%LOCALAPPDATA%\Destinia\save`.

O configurador guarda a configuração anterior em `config.ini.previous`. Caminho configurado precisa ser absoluto e diferente da raiz do disco. Configuração inválida gera erro; o host não troca silenciosamente para outra partida vazia. `--save` explícito continua podendo apontar para uma cópia relativa de teste. Fora do Windows, sem `LOCALAPPDATA`, o padrão legado é `save` no diretório atual; use um caminho explícito estável.

Não abra duas versões simultaneamente na mesma pasta. Os hosts novos usam uma trava de arquivo liberada pelo sistema ao fechar, inclusive em encerramento abrupto; versões antigas não respeitam essa trava. A existência do arquivo `.destinia.lock` após fechar é normal: não significa que a trava continua ativa.

## Backups automáticos

Antes de carregar os arquivos, o host copia e verifica o conteúdo de todo o save selecionado. Os dez snapshots completos mais recentes ficam em uma pasta irmã:

```text
%LOCALAPPDATA%\Destinia\save_backups\backup_DATA_IDENTIFICADOR\...
```

Se escolher `D:\Jogos\partida`, os backups ficam em `D:\Jogos\partida_backups`. Os mesmos backups atendem normal e zoom. O host não segue links dentro do save e não inclui a trava no snapshot. Uma pasta sem arquivos de progresso não gera snapshot vazio.

Se houver erro de permissão, falta de espaço, link interno ou cópia divergente, o host encerra antes de carregar a partida. Os saves ativos e backups concluídos anteriores são preservados; a cópia parcial criada por aquela tentativa é removida. A rotação apaga somente pastas de backup identificadas pelo próprio sistema. Dez snapshots não significam limite fixo em bytes: o espaço depende do tamanho do save.

Para restaurar:

1. Feche o jogo e confirme no console qual é a pasta ativa.
2. Renomeie a pasta ativa para guardar seu estado atual.
3. Crie novamente a pasta ativa e copie para ela os arquivos completos do snapshot escolhido. O marcador `.destinia-backup` é metadado, não precisa ser copiado.
4. Abra o jogo e confira a partida. Guarde a pasta renomeada até confirmar o progresso.

Backups de inicialização não substituem gravação transacional durante a partida; essa mudança continua no planejamento posterior. A 0.5 não converte formatos de save.

## Logs TXT automáticos

Cada abertura cria uma pasta na raiz do projeto:

```text
relatorios\jogo_host_DATA_IDENTIFICADOR\log.txt
relatorios\jogo_host_zoom_DATA_IDENTIFICADOR\log.txt
```

São registrados versão/host, pasta de save, backup, inicialização, arquivos e recursos carregados, mensagens Android da biblioteca (mesmo sem `-v`), escritas/fechamentos de save observados, comandos de tecla/toque, áudio solicitado, mudanças observadas de menu/rede/nível/XP/dimensões de mapa, desempenho resumido e erros. O conteúdo exato de cada arquivo de save não é despejado no log.

A lógica é nativa ARM: o logger registra os eventos que o host consegue observar. Não decodifica automaticamente todas as missões, drops, diálogos, inimigos ou ações internas. Escrita/fechamento de arquivo observados não comprovam que toda a transação de salvamento nativa terminou.

Por sessão, há no máximo três arquivos de até **2 MiB**: `log.txt` (mais recente), `log.1.txt` e `log.2.txt`. São mantidas dez sessões por host; sessões com trava ativa são preservadas e podem temporariamente exceder essa contagem. As margens de retenção equivalem a cerca de 120 MiB para os dois hosts sem sessões extras ativas. Pastas pessoais sem o marcador do logger não entram na limpeza.

Não há despejo de cada quadro por padrão. O resumo de estado é amostrado a cada 16 quadros e o de emulação a cada 80. Eventos críticos e encerramento são descarregados; as outras entradas são descarregadas periodicamente. Um encerramento forçado pode perder a parte ainda em buffer. As capturas/trace F9 existentes continuam opcionais e têm sua própria retenção; os limites acima não limitam os relatórios antigos do diagnóstico ou BMPs.

O diretório pode ser escolhido com `--reports PASTA`. O diagnóstico aponta esse argumento para `logs_do_jogo` dentro da própria sessão, sempre com um `--save` isolado. Falha na criação/escrita do log gera aviso no console e não altera o save nem impede jogar; falha de backup do progresso existente impede iniciar para preservar a partida.

Os TXT podem conter caminhos locais e informações da partida. Logs, configurações e backups não acompanham o ZIP de distribuição. Ao relatar um problema, envie o TXT da sessão e o relatório do diagnóstico; não é necessário enviar seus saves.


## Candidata 0.8 — gravação recuperável e modos

As escritas e exclusões feitas numa chamada ARM do host são preparadas em temporários. O jogo verifica fechamento e sincronização dos arquivos antes de substituir os ativos; se o processo for interrompido durante as substituições, a próxima abertura recupera o lote anterior antes de carregar a partida. O journal fica numa pasta irmã terminada em `.transaction`. Não apague essa pasta para tentar corrigir uma falha: um journal desconhecido/corrompido é preservado para análise. Os backups rotativos continuam complementando essa proteção.

A proteção foi testada com falhas injetadas e encerramento de subprocesso. Ela não é garantia comprovada contra falha física do disco ou queda de energia, nem transforma ações que atravessam vários quadros numa transação única.

O modo clássico mantém a pasta atual. O expandido usa a pasta irmã `_expandido`, com identificação, trava, journal e backups próprios. A cópia de progresso entre eles exige uma ação explícita no configurador e um destino ainda inexistente; não há conversão silenciosa nem alteração do formato binário. Veja [preferências](PREFERENCIAS.md). O diagnóstico respeita o modo selecionado, mas continua usando cópias descartáveis.
