> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

# Histórico de alterações

Histórico em português, da versão mais recente para a mais antiga. Somente alterações efetivas são registradas como novidades.

## Próxima atualização — limpeza e diagnóstico, sem nova release

### Limpeza do repositório

- Removidos logs de testes, relatórios de uso de CPU, backups dos fontes, cópias antigas não usadas no build, configuração local do editor, arquivo vazio e material pessoal de continuidade/exportação de conversas.
- Não foram encontrados arquivos de save rastreados na árvore atual. Reforçado o `.gitignore` para pastas de saves, `msave.dat`, formatos de save, relatórios locais, backups e credenciais.
- Preservados os fontes ativos, CMakeLists, recursos do jogo, launchers, ferramentas de diagnóstico, testes e documentação técnica útil.
- A limpeza afeta a branch principal atual; não reescreve commits antigos nem altera a release 0.2 Beta já publicada.

### Ferramenta de diagnóstico

- Adicionado `Testar Destinia.bat` com executor PowerShell para verificações automáticas e roteiro guiado dos dois hosts.
- Saves de teste isolados por sessão e por host, comparação de hashes da origem e proteção contra sobrescrita de cópias anteriores.
- Verificação dos dados ARM, executáveis, logs, contagem de quadros e BMPs; falhas no log não são ignoradas quando o host retorna zero.
- Limite de tempo por processo, captura de stdout/stderr, relatório em texto e JSON e roteiro manual de gameplay, salvar/reabrir, câmera, PT/F9 e entradas.
- Pacote de testes para extrair no projeto existente, sem executáveis ou dependências externas; pastas de relatório/save geradas pelo diagnóstico ignoradas no Git.
- Documentadas as limitações: compras/rede e erros internos de libc/ELF/SDL não são testados automaticamente; sem validação real de Windows nesta etapa.
- Nenhuma alteração nos fontes do jogo ou na release 0.2 Beta já publicada.

## 0.2 Beta — 20/10/2026

### Alterações em relação ao commit inicial

- Identificação “Destinia 0.2 Beta” no título da janela de `host` e `host_zoom`.
- Arquivo `VERSION` com o número 0.2.
- README e instruções de instalação/compilação em português, com distinção entre fontes e pacote executável.
- Este changelog com aviso de Beta no início.
- Notas propostas para a primeira release, plano de publicação e relatório de verificações/pendências.
- Registro dos componentes originais e dependências de terceiros, sem atribuir uma licença não autorizada.
- Identificação de `game` como o APK original do Destinia Android extraído.
- Seção **Dependências** no README e política de instalação separada: instaladores, ferramentas e bibliotecas externas necessárias no PC não serão incluídos no arquivo da release.
- Regras de exportação em `.gitattributes` para manter o APK extraído, backups e logs fora dos arquivos automáticos de código-fonte da tag, preservando esses arquivos no repositório.
- Empacotador de fontes com lista explícita de arquivos, validação de caminhos e versão, ZIP reproduzível no mesmo ambiente, SHA-256 e proteção contra sobrescrita.
- Testes do empacotamento para evitar inclusão de saves, credenciais, dados originais, caminhos externos e artefatos anteriores.
- Atualização das orientações de repositório/versionamento em `AGENTS.md` e inclusão de `/dist/` no ignore.

### Preservado

A única alteração nos fontes C++ é o título da janela. Gameplay, resolução, câmera, biblioteca ARM, renderização, controles, tradução, áudio, saves, compras e instrumentação F9 não foram modificados. Os CMakeLists e launchers existentes permanecem preservados.

### Limitações conhecidas

- Câmera/transição de mapa em 800×480 ainda em investigação.
- Fluxos de compra/rede podem ficar aguardando resposta.
- Falhas emuladas podem encerrar com código zero.
- Tamanho zero nos stubs de fread/fwrite é tratado incorretamente.
- Possível desalinhamento de mouse/DPI em `host`.
- Falta verificação dos retornos de criação dos recursos SDL.
- O carregador ELF não valida todos os limites de offsets/tamanhos antes de acessar o arquivo.
- Launchers dependem do MSYS2 em caminho fixo e compartilham save; scripts antigos de teste podem usar uma origem de save desatualizada.
- Não há novo executável Windows, validação de DLLs ou teste de gameplay desta candidata.

## Base anterior

Commit inicial: `d9a9352bbbe9902b7b9bc3ef0dac2353b3f26611`, “Backup inicial do projeto Destinia”. Não havia releases no GitHub na consulta desta preparação. Não foi inventada uma versão 0.1.

A preparação 0.2 feita anteriormente nesta conversa não chegou ao GitHub; esta candidata a substitui e amplia.
