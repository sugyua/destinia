# Preparação da 0.5 Beta

> **Versão Beta: pode apresentar bugs. Faça backup dos saves.**

Estado: **candidata em validação; publicação não autorizada nesta etapa**. Pedido de implementação inclui logs, backups, correções de estabilidade e configuração compartilhada de save. A versão pública anterior continua sendo a 0.4.

## Conteúdo da candidata

Os dois hosts passam a usar `common/runtime_support.h` (log, backup, trava e configuração) e `common/elf_image.h` (validação da imagem antes das relocações). Gameplay ARM, assets, câmera, zoom, tamanho da UI, F11 e ritmo nativo são preservados. Confira o [plano](PLANO_0.5.md), o [changelog](../CHANGELOG.md) e as [instruções de saves/logs](SAVES_E_LOGS.md).

## Verificações locais

- Suíte anterior executada antes das mudanças: 11 testes Python passaram; um smoke real estava condicionado à disponibilidade do executável.
- Novos testes reproduziram ausência de log, backup/configuração e falhas de arquivo/ELF/SDL/retorno nos dois hosts antes das correções, incluindo crash de tamanho zero e falha emulada retornando sucesso.
- Testes nativos dos dois hosts exercitam log, backup com bytes preservados, falha de backup antes da inicialização, configuração global/ambiente/CLI, configuração inválida sem partida nova, fread/fwrite zero, falhas forçadas de janela/renderer/textura e falha de execução emulada e exceções de stubs contidas antes de cruzar o callback C do Unicorn.
- ELF malformado: cabeçalhos/tabelas truncados, classe/endian incorretos, tamanhos/offsets fora dos limites, overflow de segmento, strings, hash, símbolos, relocações e ausência de terminador dinâmico. Processos devem retornar erro, sem crash.
- Testes de suporte exercitam retenção de snapshots/logs, cópia restaurável, recusa de links, destino de backup bloqueado sem alterar a origem, pastas pessoais preservadas e trava concorrente.
- PowerShell: configurador com caminho contendo espaços, prioridade da variável, configuração anterior e progresso preservados; suíte do diagnóstico e fluxo de câmera com processos simulados, sem usar saves reais.
- Compilação Linux dos dois hosts com SDL2 e Unicorn; inicialização ARM real, BMPs e fixture de janela/F11/viewport ultrawide, centralização da câmera, fundo cinza e tamanho do texto XP.

Fechamento local em **21/10/2026**: **25 testes Python passaram, sem skips**, incluindo o smoke de BMPs com executável real. O teste de ELF inclui **17 entradas malformadas por host**. Os sete grupos PowerShell do diagnóstico, o fluxo de câmera simulado e o configurador também passaram. Mensagens Android nativas são verificadas no TXT sem modo verbose.

Para reproduzir a suíte completa em ambiente de desenvolvimento com compilador, SDL2/Unicorn e dados originais disponíveis:

```sh
cmake -S host -B host/build
cmake --build host/build
cmake -S host_zoom -B host_zoom/build
cmake --build host_zoom/build
DESTINIA_NATIVE_TESTS=1 DESTINIA_ZOOM_EXE="$PWD/host_zoom/build/destinia_host" python3 -m unittest discover -s tests -v
pwsh -NoProfile -File tests/test_diagnostic.ps1
pwsh -NoProfile -File tests/test_camera_mode.ps1
pwsh -NoProfile -File tests/test_save_configuration.ps1
```

Esses comandos descrevem o ambiente Linux usado para desenvolvimento; no Windows, o EXE termina em `.exe` e os scripts de uso normal exigem apenas o Windows PowerShell já documentado. A compilação local não habilitou SDL2_mixer, indisponível no ambiente; áudio não foi retestado. O ZIP final acompanha um arquivo `.sha256`, cuja integridade e reprodução são conferidas separadamente do gameplay.

## Confirmação necessária no Windows

1. Guardar uma cópia da partida atual. Extraia a candidata em pasta separada e compile os dois hosts com `Compilar Destinia.bat`.
2. Para a primeira validação, usar uma cópia descartável via `Testar Destinia.bat`, roteiro geral. Conferir inicialização, gameplay, salvar/reabrir, câmera/UI, mouse/DPI, áudio e controle/foco.
3. Conferir `logs_do_jogo` dentro da sessão: versão 0.5, diretório isolado, inicialização, eventos e encerramento. Conferir snapshots da cópia de save, sem mexer no original.
4. Fechar todos os jogos e validar `Configurar pasta de saves.bat` sobre uma cópia: abrir normal, salvar, fechar e abrir zoom; depois repetir de outra pasta de instalação apontando à mesma cópia. Confirmar continuidade sem começar de novo e testar restauração de um snapshot em cópia descartável.
5. Conferir que dois hosts novos não abrem a mesma pasta simultaneamente. Versões antigas não possuem essa proteção.

Não foram validados gameplay e áudio reais no Windows, execução sob PowerShell 5.1/MinGW nem novos recursos online. O teste local de PowerShell usa uma versão recente no Linux; wrappers SDL e mapas sintéticos não comprovam o resultado visual no PC do autor.

## Distribuição e publicação

Gerar `destinia-0.5-beta-source.zip` e SHA-256 pelo manifesto revisado, incluindo `common`, scripts/configurador e documentação. Não incluir EXEs locais Linux, DLLs, ferramentas, dados do APK, saves, logs ou backups. A seção Dependências permanece no README; instaladores e bibliotecas externos são obtidos separadamente.

Antes de criar tag/release: apresentar o pacote, os resultados e limitações ao autor e obter aprovação, conforme [AGENTS.md](../AGENTS.md). Não alterar a release 0.4 nem seus anexos durante a preparação.
