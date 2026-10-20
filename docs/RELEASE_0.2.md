# Preparação da primeira release — Destinia 0.2 Beta

Data: 20/10/2026. Estado: **preparação aprovada para publicação da 0.2 Beta em fontes**.

## Base e escopo

Repositório solicitado pelo usuário: https://github.com/sugyua/destinia

Commit consultado: `d9a9352bbbe9902b7b9bc3ef0dac2353b3f26611`, “Backup inicial do projeto Destinia”. A consulta à API de releases retornou uma lista vazia. O repositório permanece na base inicial nesta preparação; a tentativa anterior de publicar a 0.2 havia falhado.

Foi lido o AGENTS.md da raiz; não há outro AGENTS.md na árvore consultada. Ele pede preservação de funcionalidades, mudanças pequenas, revisão de arquivos originais de terceiros e diagnóstico inicial. O usuário já recebeu o diagnóstico e agora autorizou preparar a release, exigindo aprovação antes de sua publicação. A referência antiga a Destinia_PCport foi atualizada para o repositório explicitamente solicitado.

O clone Git via shell falhou por indisponibilidade do proxy na porta 8080. Os arquivos necessários foram obtidos pelo conector GitHub, fixados ao commit acima. O diretório de trabalho é uma seleção local dos arquivos, não um clone integral. Uma cópia dos arquivos originais foi preservada separadamente para comparação. Nenhum save foi baixado ou executado.

## Estrutura e dependências

- C++17, Unicorn e SDL2; SDL2_mixer opcional, configurados por pkg-config nos CMakeLists.
- `host/main.cpp`: 400×240, versão principal do host.
- `host_zoom/main.cpp`: 800×480 experimental; inclui tradução PT, lógica preexistente de bordas e instrumentação de câmera/viewport. Esses mecanismos foram preservados.
- `game`: APK original do Destinia Android extraído, conforme confirmado pelo autor; 828 arquivos em assets, 35 arquivos de áudio em res/raw, biblioteca ARM, DEX e demais recursos Android.
- `Claude/main.cpp`: referência histórica, diferente dos hosts atuais.
- `docs`: histórico útil, porém com afirmações antigas que não representam todas as implementações atuais.
- Não há EXE ou DLL Windows na árvore consultada, nem arquivo de licença do projeto identificado.

## Plano adotado

1. Fixar a candidata em 0.2 Beta e apresentar a identificação na janela dos dois hosts.
2. Preservar todos os comportamentos, CMakeLists, launchers existentes e dados originais.
3. Preparar README, instalação, changelog e notas de release em português, distinguindo implementações existentes de novidades.
4. Gerar ZIP curado de fontes por lista explícita, acompanhado de SHA-256, sem dados originais, saves, logs, backups ou binários.
5. Apresentar verificações e pendências ao usuário.
6. Somente após aprovação e resolução das pendências de publicação, publicar fontes/notas e criar a primeira release como pré-lançamento `v0.2-beta`.

Não foram configuradas atualizações automáticas ou publicações periódicas. A versão 0.2 continua Beta; não foi promovida a 1.0 estável.

## Análise dos launchers

| Script | Constatação |
| --- | --- |
| Abrir Destinia.bat | Procura host/build/destinia_host.exe; exige MSYS2 em C:\msys64\ucrt64\bin; usa save compartilhado no LOCALAPPDATA |
| Abrir Destinia Zoom 800x480.bat | Mesmo modelo de dependências/save; não ativa --lang pt automaticamente |
| Testar traducao PT.bat | Origem host_zoom/save_zoom800; pode não representar o save compartilhado atual; clone é reaproveitado se já existe |
| Testar compra - cancelamento seguro.bat | Usa origem antiga semelhante; apesar do nome, o texto alerta sobre possível crash; não distribuir como teste garantidamente seguro |
| Diagnosticar calor.bat | Ferramenta de desenvolvimento com executável, caminhos MSYS2 e scripts PowerShell; não faz parte do pacote curado |

Os dois launchers normais foram incluídos como ferramentas de desenvolvimento, sem alterações. Scripts de testes/diagnóstico permanecem no repositório, mas foram excluídos do ZIP. Não foi feita migração de saves.

## Pendências e erros encontrados

| Prioridade | Achado | Evidência e impacto |
| --- | --- | --- |
| Antes de release binária | EXE não disponível e requisitos de runtime não validados | Necessário compilar ambos os hosts e documentar instalação separada das dependências transitivas; nenhum ZIP Windows foi gerado |
| Antes de tag/publicação | Dados originais rastreados e licenciamento pendente | ZIP curado os exclui, mas uma tag do estado atual gera arquivos automáticos do GitHub contendo game; revisar autorização e árvore da tag |
| Antes de release binária | Sem validação desta candidata em Windows | Faltam build nativo, execução e teste em máquina limpa com as dependências documentadas instaladas |
| Alta para evolução do host | fread/fwrite com tamanho zero | O buffer tem sz*cnt bytes, mas a chamada substitui sz zero por um; acesso indevido possível se cnt positivo |
| Alta para evolução do host | Validação insuficiente no carregador ELF | phoff/offset/filesz e tabelas são usados em memcpy sem validar todos os limites; usar somente a biblioteca compatível, não arquivos arbitrários |
| Média | Código de saída após falha emulada | Falha em callFn pode causar break no loop, seguido de return 0; scripts não devem depender só do exit code |
| Média | Criação de recursos SDL sem verificação | Janela, renderer e textura são usados sem checagem de retorno |
| Média | Mouse/DPI em host | Coordenadas da janela são divididas pelo tamanho do renderer; unidades podem divergir; host_zoom faz conversão explícita |
| Conhecida | Câmera e transição em 800×480 | Há investigação anterior, sem causa confirmada do defeito; nenhuma correção anunciada |
| Conhecida | Compras/rede incompletas | Cash pode mudar sem resposta de conclusão da loja, deixando spinner; não é uma compra online implementada |
| Documentação | --scale e documentos antigos | --scale é aceito, mas a janela é criada em 1200×720 no código atual; não anunciar como ajuste funcional de escala |

Os achados de código foram confirmados por leitura, não por reprodução dinâmica nesta sessão. Não foram corrigidos nesta preparação para evitar mudanças de comportamento sem testes do host.

## Arquivos preparados

- VERSION e títulos dos dois hosts: 0.2 Beta.
- README.md, INSTALL.md, CHANGELOG.md e THIRD_PARTY.md.
- Este relatório e release/NOTAS_0.2.md.
- release/manifest.json, scripts/package_release.py e tests/test_package_release.py.
- AGENTS.md atualizado para o escopo atual e aprovação antes de publicar; /dist/ ignorado.

## Verificações realizadas

| Verificação | Resultado |
| --- | --- |
| Estado do GitHub e releases | Commit base confirmado; lista de releases vazia |
| Instruções e árvore | AGENTS.md e estrutura revisados; 828 assets e 35 áudios identificados |
| Comparação dos fontes | Apenas o título da janela mudou em cada main.cpp |
| Comparação de build/launchers | CMakeLists e launchers comparados com originais e preservados |
| Versão e aviso | VERSION=0.2; changelog começa com aviso Beta |
| Testes do empacotador | 6 testes aprovados: allowlist/checksum, caminhos proibidos, symlink externo, entrada ausente, divergência de versão, reprodução/recusa de sobrescrita |
| Sintaxe Python | compileall de scripts e testes sem erro |
| Configuração dos hosts | Tentada para ambos; bloqueada porque cmake não está instalado |
| Dependências nativas | pkg-config não encontrou SDL2, Unicorn nem SDL2_mixer; headers também ausentes |
| Clone via shell | Falhou ao conectar ao proxy; leitura pelo conector funcionou |
| Jogo e scripts BAT | Não executados: ambiente Linux sem runtime Windows/dependências do host |

A inspeção final do ZIP, hashes e relatório de comparação são produzidos em conjunto com os artefatos locais. Esses resultados não equivalem a compilação ou teste do jogo.

## Conteúdo mínimo de um futuro ZIP Windows

- Executável compilado da versão 400×240; o host 800×480 deve permanecer claramente experimental e separado.
- Não incluir instaladores, SDL2, Unicorn, SDL2_mixer/codecs, DLLs de runtime ou ferramentas externos que o usuário precise instalar; informar os requisitos e a instalação separada na seção Dependências do README.
- Avisos de terceiros e materiais exigidos pelas licenças das versões efetivamente distribuídas.
- Launcher com verificação clara das dependências externas e dos dados do jogo; os BAT atuais exigem MSYS2 no caminho documentado. Não anunciar pacote autossuficiente.
- README de instalação, versão, changelog, notas e checksums.
- Dados originais somente após revisão e autorização; alternativa: exigir dados fornecidos pelo usuário, com instruções claras.
- Nenhum save pessoal, credencial, captura, log de diagnóstico ou backup de código.

## Verificações pendentes para considerar um pacote Windows pronto

1. Compilar em UCRT64 e registrar versões de compilador e bibliotecas, inclusive áudio.
2. Identificar dependências transitivas do executável, registrar versões e instruções na seção Dependências e testar em Windows limpo com somente os requisitos documentados instalados. Não copiar essas dependências para o ZIP.
3. Validar primeiro boot com dados compatíveis e save novo, usando pasta com espaços no caminho.
4. Validar iniciar partida, movimentação, mapa, salvar, fechar e recarregar em save isolado.
5. Validar áudio, teclado, mouse com DPI 100/150%, gamepad e janela redimensionada.
6. Em host_zoom, validar PT, F9/BMP e registrar a limitação da transição de mapa.
7. Revisar licenças, origem dos dados e conteúdo da árvore marcada pela tag.
8. Conferir asset final, checksum e notas; obter aprovação explícita antes de publicar.

## Publicação proposta e aprovação

Recomendação: manter esta entrega como candidata 0.2 Beta em fontes até concluir a revisão dos dados originais. Se o usuário aprovar uma primeira release somente de fontes, explicitar a ausência de binário e marcar como pré-lançamento. Não anunciar instalação pronta ou estabilidade.

Nenhum commit remoto, branch remota, tag ou release foi criado nesta sessão. A permissão de escrita, que falhou com 403 anteriormente, não foi retestada por uma operação de escrita antes da aprovação.


## Aprovação e finalização da publicação

O usuário aprovou a publicação e esclareceu que game é o APK Android extraído. Solicitou instalar as dependências externas separadamente, documentadas no README. Essa orientação foi incorporada.

A integração GitHub passou no teste de escrita após sua instalação. O clone integral do repositório e o acesso pelo GitHub CLI também funcionaram na etapa final, após habilitar o acesso de rede necessário ao comando. Esses resultados substituem os bloqueios de acesso registrados na preparação inicial acima.

O título dos hosts continua sendo a única mudança nos fontes C++. A finalização acrescenta .gitattributes com export-ignore para excluir game, cópias históricas e logs selecionados dos arquivos automáticos da tag. Não apaga os arquivos do repositório. A revisão de distribuição desta release restringe os downloads a fontes do host e documentação; não redistribui o APK extraído ou dependências externas.

Publicação aprovada: pré-lançamento v0.2-beta, com ZIP curado de fontes e SHA-256. A validação inclui testes do empacotador, comparação com o commit base, inspeção do ZIP, git archive e inspeção do arquivo retornado pelo GitHub antes da publicação final. As limitações de compilação e gameplay Windows permanecem.
