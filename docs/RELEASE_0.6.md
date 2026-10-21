# Verificações da candidata 0.6 Beta

> **Beta: pode apresentar diversos bugs. Faça backup dos saves. A publicação da 0.6 aguarda aprovação do autor.**

## Escopo

Preferências globais persistentes, perfis de teclas/botões, volumes separados, pausa opcional e pacote Windows x64 com dois EXEs próprios. Mantidos os saves da 0.5, backups, TXT limitado, zoom/câmera/UI, gameplay e ritmo ARM. Sem expansão de níveis, itens ou loja/rede nesta etapa.

`Configurar Destinia.bat` valida as escolhas com os dois hosts antes de substituir config.ini. Reset preserva save_dir e chaves desconhecidas. Atualizações de preferências e da pasta de saves usam a mesma trava de configuração. O diagnóstico não grava preferências pessoais.

## Verificações locais

Compilação Linux dos dois hosts em Release com SDL2 2.32.4, SDL2_mixer 2.8.1 (OGG/MP3) e Unicorn 2.1.4. Suíte Python habilitada com biblioteca ARM original, sem usar saves pessoais; testes PowerShell executados separadamente.

- Regressões da 0.5: backups/rotação/travas, fread/fwrite zero, falhas SDL/emulação, TXT e 17 entradas ELF malformadas por host.
- Preferências: padrões, limites, perfis, conflitos e F9/F11 reservados; reabrir janela preserva preferências; opções CLI prevalecem; configuração inválida para antes de abrir save.
- Entradas: aliases teclado/controle compartilham contagem; perda de foco e remoção do controle ativo liberam comandos; remoção de outro dispositivo é ignorada; pausa/retomada verificadas por contagem das chamadas jniRun.
- Áudio: mixer real com driver dummy decodifica bg100.mp3/eff00.ogg, aplica volumes combinados, silencia sem foco e restaura. Isso verifica o backend, não a audição humana.
- Zoom: BMPs reais 800×480; fixture ARM de câmera/mapas sintéticos, fundo cinza, UI/XP e viewport ultrawide preservada. Não representa todas as transições da campanha.
- Configuradores: merge e reset preservam save/valores desconhecidos; alteração inválida preserva config anterior; diagnóstico usa clones e não grava preferências.
- Empacotadores: allowlist, recusa PE de arquitetura incorreta, dois EXEs Windows x64, sem dados pessoais ou dependências externas.

## Windows automatizado

Workflow `.github/workflows/windows-candidate.yml`, runner Windows Server 2022 com MSYS2 UCRT64. Compila os dois hosts com mixer, executa a biblioteca ARM em modo sem janela, testes nativos de estabilidade/preferências/áudio e PowerShell 5.1 e gera pacote x64.

Rodada final em andamento; resultados e identificação do run serão registrados após conclusão. Não há campanha manual, controle físico, inspeção visual ou audição no PC do autor. A validação automática não comprova funcionamento completo em todas as máquinas Windows.

## Distribuição

Pacotes separados de fontes e Windows x64, cada um com SHA-256. Dados `game`, saves, logs, backups, DLLs, instaladores e ferramentas externas excluídos. Dependências em README/INSTALL; dados compatíveis fornecidos pelo usuário.

A candidata fica em branch/PR para revisão e teste. Não criar tag ou release 0.6 sem aprovação separada. A release pública atual é a 0.5 Beta.
