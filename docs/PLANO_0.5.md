# Preparação da 0.5 Beta

Escopo autorizado em 21/10/2026: log TXT limitado, backups rotativos, correções de estabilidade e configuração de uma pasta de save compartilhada entre instalações/hosts. Preservar gameplay, câmera, UI e dados originais. A publicação da release aguarda revisão e aprovação do autor.

## Decisões

- Os dois hosts usam o mesmo suporte em `common`, sem reorganizar a lógica ARM existente.
- Log por sessão em `relatorios/jogo_*`, três arquivos de até 2 MiB e dez sessões por host. Registrar inicialização, chamadas observáveis, entradas, arquivos/saves, transições observadas, desempenho resumido e erros; não afirmar que eventos internos desconhecidos foram decodificados. Não registrar cada quadro por padrão.
- Antes de carregar uma partida, criar e verificar uma cópia completa dos arquivos do save; guardar dez snapshots concluídos em `save_backups`, ao lado da pasta de save selecionada. Nunca apagar o save ativo; não seguir links dentro dos saves/backups. Falha no backup de progresso existente impede a abertura para evitar escrita desprotegida.
- Acesso exclusivo por pasta de save durante a execução dos hosts novos, usando trava liberada pelo sistema ao encerrar. Versões antigas não respeitam essa trava; não executar versões simultaneamente sobre a mesma partida.
- Pasta padrão do Windows preservada: `%LOCALAPPDATA%\Destinia\save`. Precedência: `--save` do diagnóstico, variável `DESTINIA_SAVE_DIR`, configuração global `%LOCALAPPDATA%\Destinia\config.ini`, padrão. Um BAT/PowerShell permite selecionar a pasta existente sem copiar ou sobrescrever saves. Versões antigas continuam no padrão e não leem a configuração nova.
- Corrigir retornos de fread/fwrite com tamanho/contagem zero antes de acessar a memória. Validar ELF ARM32 little-endian, intervalos de segmentos/tabelas/strings/relocações e limites de alocação antes de aplicar mudanças na imagem.
- Verificar recursos SDL e propagar falhas de execução (incluindo entradas) no código de saída, com encerramento e mensagens legíveis. Não alterar fluxo da loja nesta versão.

## Implementação e verificações

1. Executar a suíte atual; adicionar testes que reproduzam os erros e a ausência de logs, backups e configuração. Compilar fixtures dos dois hosts com wrappers de falha somente nos testes.
2. Implementar suporte de logs, backup, trava e resolução de save; testar retenção, conteúdo, falhas, links, concorrência e precedência.
3. Integrar eventos observáveis aos hosts e corrigir arquivo/ELF/SDL/retorno, com testes de arquivos malformados e emulação forçada a falhar.
4. Atualizar inicializadores e diagnóstico para respeitar a configuração global, mantendo cópias isoladas. Testar o configurador PowerShell com caminhos contendo espaços.
5. Atualizar VERSION, changelog, instalação, README, roadmap e manifesto da candidata. Criar ZIP curado e SHA-256, sem saves, dados originais ou dependências externas.
6. Rodar testes Python/PowerShell, compilar ambos, executar a biblioteca ARM real sem janela e verificar as fixtures gráficas/câmera. Documentar separadamente o teste manual de gameplay/UI/áudio no Windows que o ambiente não consegue substituir.

## Pontos de revisão

Conferir disco cheio/permissões e backup incompleto sem perder snapshots válidos; caminhos com espaços/configuração inválida sem trocar silenciosamente de partida; diagnóstico ignorando o save global quando recebe `--save`; ELF truncado/overflow/string sem terminador; falhas SDL e emulação retornando erro. Revisar diffs e conteúdo do ZIP antes de solicitar aprovação de publicação.
