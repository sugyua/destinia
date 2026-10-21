# Candidata 0.6 — configurações, comandos e distribuição

Preparação/testes autorizados pelo autor em 21/10/2026, depois de aprovar a publicação da 0.5 com validação manual Windows pendente. A autorização não publica automaticamente a 0.6. Preservar saves, backup/log da 0.5, lógica ARM, câmera, UI e ritmo nativo.

## Decisões de implementação

- Preferências no mesmo `LOCALAPPDATA/Destinia/config.ini`, preservando `save_dir` e chaves desconhecidas. Vídeo por host: tamanho de janela e tela cheia; normal mantém 1200×720 e zoom 800×480/tela cheia por padrão. CLI tem prioridade. Salvar tamanho/modo ao fechar normalmente; o diagnóstico passa `--no-save-preferences` para não alterar preferências pessoais.
- Perfis nomeados de teclado e botões do gamepad. Padrões preservam aliases e códigos WIPI existentes; sticks mantêm direção/habilidades e zona morta atuais. Rejeitar teclas desconhecidas, duplicações conflitantes e F9/F11 reservados. Contar fontes físicas por ação para soltar uma direção somente quando todas as teclas/botões correspondentes forem soltas.
- `Configurar Destinia.bat` oferece vídeo, áudio, pausa, seleção/edição de perfis e restauração de preferências sem apagar ou trocar `save_dir`. Configuração feita com o jogo fechado; gravação temporária/substituição e configuração anterior preservada.
- Música/efeitos 0–100%, multiplicados pelos níveis nativos 0–5. Silenciar ao perder foco opcional; restaurar os volumes ao recuperar foco. Testar consultas e decodificação em SDL2_mixer real com dispositivo de áudio dummy, sem afirmar audição manual.
- Pausa opcional ao perder foco/desconectar controle; desativada por padrão para preservar comportamento. Liberação de entradas e toque ao perder foco ou remover o controle. Exigir sticks neutros antes de retomar. F10 pausa/retoma por padrão e pode ser remapeado. Pausa interrompe jniRun, não acelera nem altera a lógica ARM.
- Tela cheia proporcional/F11 e mapeamento de mouse na imagem central nos dois hosts. Testar redimensionamento e viewport; preservar o enquadramento/câmera/HUD do zoom.
- Compilar os dois EXEs em Windows UCRT64 por GitHub Actions, exercitar inicialização e testes automatizados, reunir pacote Windows sem DLLs/runtimes/dados/saves. Instalação de dependências continua separada e documentada no README. ZIP de fontes separado.

## Arquivos e sequência

1. Testes de preferências/entradas/áudio antes da implementação; adicionar `common/user_settings.h` e `common/input_state.h`, com parsing, perfis e estado de ações verificáveis.
2. Integrar preferências/entradas/foco/áudio aos dois `main.cpp`, preservando hooks ARM e diagnóstico. Regressões: aliases simultâneos, soltar entradas, controle removido, tecla de pausa e CLI sem persistência durante testes.
3. Criar configurador PowerShell/BAT e testes de persistência/reset/perfis, sem tocar no progresso. Atualizar o leitor de saves para coexistir com as novas chaves.
4. Criar empacotador Windows e workflow; validar PE x64 e lista permitida, sem dependências externas no arquivo. Conferir artefatos baixados e seus hashes.
5. Rodar suíte anterior, novas unidades/fixtures e testes de integração com a biblioteca ARM real, compilar Linux com/sem áudio e executar o workflow Windows. Corrigir falhas encontradas; distinguir testes automáticos de gameplay/áudio manual.
6. Atualizar VERSION, changelog, README, instalação, roadmap, notas/manifesto da candidata. Subir branch e ZIPs para revisão. A 0.5 permanece release publicada; solicitar aprovação específica antes de publicar a 0.6.

## Pontos de revisão

Configuração inválida sem reset silencioso de saves; escrita das preferências sem perder `save_dir`; clones do diagnóstico sem escrever preferências pessoais; alias/controller+teclado sem soltura prematura ou direção presa; pausa sem continuar jniRun; volume nativo preservado; EXEs iniciando com DLLs instaladas separadamente, sem DLL dentro do ZIP; ausência de progresso/dados originais nos artefatos.
