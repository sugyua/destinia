> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

# Histórico de alterações

Histórico em português, da versão mais recente para a mais antiga. Somente alterações efetivas são registradas como novidades.

## Não lançado — documentação do planejamento (21/10/2026)

- README e roadmap atualizados com as 37 melhorias aprovadas, organizadas por dificuldade e dependências em metas provisórias de 0.5 a 0.12.
- Log TXT mantido como meta da 0.5; expansão de nível máximo, ranks, itens únicos e conjuntos incluída no planejamento, com investigação de dados, balanceamento e compatibilidade de saves antes da implementação.
- Referências da pesquisa, critérios de verificação e separação entre recursos atuais e futuros documentados. Descrição da arquitetura do host_zoom alinhada ao zoom out já presente na 0.4.
- Atualização somente de documentação: sem alterações de gameplay, nova release ou mudança na versão publicada 0.4 Beta.

## 0.4 Beta — 21/10/2026

### Compilação e inicializadores

- Adicionado `Compilar Destinia.bat` e executor Bash para MSYS2 UCRT64: o duplo clique configura e compila os dois hosts com CMake/Ninja. Os argumentos host_zoom, host e ambos continuam disponíveis.
- Falhas de configuração/compilação interrompem o processo com o código de erro preservado. Caminhos com espaços suportados; sem instalação automática de ferramentas ou dependências e sem limpeza automática do build.
- Inicializadores separados `Abrir Destinia.bat` e `Abrir Destinia Zoom 800x480.bat` incluídos no pacote; usam a mesma raiz MSYS2 configurável por DESTINIA_MSYS2_ROOT e orientam executar o compilador quando falta EXE.
- Arquivos BAT normalizados para CRLF e script Bash para LF. Saves existentes não são migrados ou apagados.

### Zoom out, fundo e UI

- Restaurado o framebuffer 800×480 no host_zoom, mostrando área maior do mapa e UI nativa proporcionalmente menor. O host normal permanece em 400×240.
- Mapas menores que a área visível centralizados por eixo. A centralização atua antes da leitura da câmera pelo ARM, incluindo caminhos com cálculo inline, e preserva a câmera nativa em mapas maiores.
- Área fora do mapa preenchida em cinza médio, com filtro dos tiles externos mantido. O cenário deixa de ser alinhado somente à direita; mapa de 480×480 tem margens de 160 pixels por lado no framebuffer 800×480.
- Margens externas da apresentação SDL também cinzas. Tela cheia proporcional e F11 preservados: área de imagem 2400×1440 centralizada no monitor 3440×1440.
- Fonte do contador de XP ajustada para manter aproximadamente o tamanho visual da 0.3 na mesma resolução de monitor, enquanto a UI original fica proporcionalmente menor. Não há controle configurável de escala da UI nesta versão.
- Diagnóstico atualizado para BMPs 800×480, identificação do novo zoom e pergunta manual sobre UI/XP. EXE desatualizado é recusado com instrução de recompilação.

### Verificações e limites

- Compilação em Linux e testes automatizados de inicialização, BMPs, empacotamento, compilador e diagnóstico; fixture ARM com mapas pequenos/grandes, escrita inline simulada, preenchimento cinza e preservação de pixels internos.
- Verificação de janela/textura, alternância F11, geometria ultrawide simulada e altura do texto XP. A confirmação visual em gameplay no Windows, incluindo cliques na UI, PT e F9, permanece pendente.
- Sem alteração de compras/rede e sem reteste de áudio nesta preparação. Sem EXE Windows pronto ou dependências no pacote de fontes.
- Log TXT detalhado dos eventos do jogo reservado para a **0.5**; não implementado na 0.4. Os relatórios de diagnóstico já existentes permanecem disponíveis.
- Releases 0.2 e 0.3 preservadas. Notas, manifesto e instruções da 0.4 revisados; publicação aprovada pelo usuário.

## 0.3 Beta — 21/10/2026

Esta versão registra o estado atual com enquadramento original ampliado. A restauração do zoom out, o fundo cinza, a centralização dos mapas pequenos e o ajuste da UI ficam para a próxima atualização.

### Tela cheia e enquadramento

- `host_zoom` renderiza em 400×240 e amplia a apresentação, com pixels nítidos e proporção preservada; a janela inicial é 800×480.
- Opção `--fullscreen` para tela cheia sem bordas na resolução atual do monitor. F11 alterna entre tela cheia e janela. Launcher do zoom e teste focado iniciam em tela cheia.
- No monitor 3440×1440, a imagem ocupa 2400×1440, centralizada, com barras externas de 520 pixels por lado. Não é renderização nativa ultrawide.
- Mapeamento do mouse acompanha a área central e o redimensionamento; cliques nas barras externas não acionam o jogo.
- PT, HUD, comandos e F9 preservados. As capturas do zoom passam a ter 400×240.
- O zoom out anterior de 800×480 foi retirado nesta versão. A ausência da faixa preta/duplicação no enquadramento original não significa correção da câmera em visão ampliada.

### Diagnóstico e proteção de saves

- Adicionados `Testar Destinia.bat`, executor Windows PowerShell 5.1 e roteiro em português; nenhuma instalação, download ou compilação automática.
- Menu com teste focado de câmera do host_zoom e roteiro geral dos dois hosts; opções por linha de comando preservadas.
- Saves copiados para pastas isoladas por sessão/host, recusa de sobrescrita/links e comparação de hashes da origem antes/depois.
- Verificação de ELF ARM, assets, executáveis, logs, contagem de quadros e integridade básica dos BMPs. Erros conhecidos no log não são ignorados quando o processo retorna zero.
- Captura de stdout/stderr e timeout somente dos processos iniciados pelo diagnóstico.
- Relatórios TXT/JSON desde o início, atualizados durante o roteiro, com etapa, indicação de sessão parcial, falhas, avisos e pendências.
- Roteiro manual de gameplay, salvar/reabrir, câmera, entradas, áudio, PT/F9; modo focado evita repetir os demais itens e registra a descrição da falha.
- Registro passivo de câmera, orientação de capturas F9 antes/depois e contagem de imagens. EXE antigo de 800×480 é identificado e exige recompilação.
- Avisos de alterações locais de cash e rede aguardando na última sonda; recuperação posterior da rede evita aviso de espera ainda pendente. Compras não são validadas automaticamente.
- Pacote separado do diagnóstico, com BAT, executor e instruções, sem dados originais, saves, executáveis ou dependências.

### Organização, documentação e distribuição

- Removidos logs locais, backups, cópias antigas de fontes, configuração do editor, arquivos vazios e material pessoal de continuidade/exportação de conversas.
- Reforçado o ignore para saves, relatórios, backups e credenciais. Nenhum save pessoal estava rastreado na árvore limpa; commits antigos não foram reescritos.
- Mantidos fontes ativos, recursos originais no repositório e dados necessários à compilação; arquivos de distribuição continuam excluindo o APK extraído e saves.
- BAT de diagnóstico normalizado para CRLF no Windows; launcher do zoom e instruções atualizados.
- Adicionado contato para colaboradores no README, mantendo a seção Dependências e instalação separada de ferramentas/bibliotecas.
- Versão 0.3 Beta no VERSION, manifesto e títulos dos dois hosts; notas, verificações, ZIP curado de fontes e SHA-256 preparados para revisão.

### Verificações e limites

- Compilação, inicialização ARM, capturas e testes do empacotador/diagnóstico verificados em Linux. Teste da rotina ARM da câmera com mapa de 30×30 tiles; janela/textura, F11 e geometria ultrawide simulada também exercitados.
- No Windows, o usuário informou troca de mapa sem defeito na sessão de 20/10 às 20:58, com inicialização 400×240, código zero e hashes da origem inalterados. Não houve captura F9 nessa sessão; o resultado manual não aprova todos os recursos.
- A UI original foi ampliada junto com o cenário e continua grande; o usuário considera o contador de XP a referência desejada para o ajuste futuro. O contador já existia antes da 0.3.
- Compras/rede continuam pendentes: alteração local de cash e rede aguardando foram observadas. Não foi confirmada compra online nem persistência do saldo.
- Permanecem limitações de libc/ELF/SDL já documentadas na 0.2. Sem novo EXE Windows distribuível, instalador ou validação em máquina limpa; áudio não foi retestado em Linux nesta preparação.

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
