AGENTS.md — Projeto Destinia

Objetivo geral

Recuperar e modernizar o jogo Destinia, originalmente desenvolvido pela Everple e publicado pela GAMEVIL.

Existem dois projetos independentes:

1. Destinia para Windows: port já parcialmente desenvolvido, com funcionalidades implementadas e alguns bugs.
2. Destinia para iOS: projeto futuro para iPhone 15 Pro Max, desenvolvido inicialmente no Windows e compilado posteriormente em ambiente macOS.

Prioridade atual

Analisar, preservar e melhorar o port existente para Windows.

Antes de qualquer modificação:

* Investigar a estrutura, linguagem, engine e dependências.
* Identificar o que já funciona e os problemas existentes.
* Não apagar nem recriar funcionalidades que já foram implementadas.
* Fazer alterações pequenas, verificáveis e reversíveis.
* Solicitar autorização antes de mudanças estruturais importantes.
* Nunca assumir que código não compreendido é desnecessário.

Referências originais

Foram identificadas as seguintes fontes para análise:

* Destinia Android APK 1.0.6.
* Destinia iOS IPA 1.0.2.
* Port Destinia Windows, desenvolvido pelo usuário.

Análises anteriores dos APKs apontaram recursos em formatos como .wmb, .mpd, .agd, .gdt e .tdt, além de possíveis dados comprimidos com ZCP/zlib.

Essas descobertas precisam ser verificadas nos arquivos originais antes de implementar decodificadores ou conversores.

GitHub

Repositório Windows:

https://github.com/sugyua/destinia

Somente o projeto Windows deverá ser enviado ao repositório existente.

Não enviar arquivos do projeto iOS, chaves, credenciais ou arquivos originais de terceiros sem revisão e autorização.

Fluxo de desenvolvimento

1. Fazer diagnóstico do projeto existente.
2. Produzir um relatório técnico.
3. Identificar bugs e priorizar correções.
4. Fazer backup antes de alterações relevantes.
5. Corrigir problemas individualmente.
6. Testar as funcionalidades afetadas.
7. Registrar mudanças para revisão e commits.

Primeira tarefa

Analise a pasta atual do projeto Destinia para Windows em modo somente leitura.

Identifique linguagem, engine, organização do código, funcionalidades implementadas, dependências e possíveis problemas.

Não modifique nenhum arquivo nesta primeira etapa.

Apresente um relatório e aguarde instruções para continuar.

Preparacao de release — orientacao atual do usuario (20/10/2026)

* A etapa inicial de diagnostico foi concluida. O usuario solicitou preparar a primeira release do port existente, preservando funcionalidades.
* Versao proposta: 0.2 Beta, com numero em VERSION e identificacao coerente nas janelas de host e host_zoom.
* Manter CHANGELOG.md em portugues, com aviso de Beta e possiveis bugs no inicio, a cada atualizacao relevante.
* Registrar apenas mudancas efetivas; distinguir testes do empacotador de compilacao e testes reais do jogo.
* Antes de publicar tag/release, apresentar plano, verificacoes e limitacoes e obter aprovacao do usuario.
* Revisar dados originais, licencas e arquivos automaticos da tag antes da publicacao. Nao enviar saves, chaves ou credenciais.
* Nao configurar atualizacoes periodicas sem uma tarefa explicitamente definida.
* A pasta game e o APK original do Destinia Android extraido, conforme esclarecido pelo usuario.
* Nao incluir no arquivo da release programas, instaladores, ferramentas ou bibliotecas externos que o usuario precise instalar no PC. Documentar esses requisitos na secao Dependencias do README do GitHub.

Orientacao atual — candidata 0.4 (21/10/2026)

* Preparar compilador BAT dos dois hosts e inicializadores separados; incluir zoom out, fundo cinza, centralizacao de mapas pequenos e UI proporcionalmente menor.
* Manter contador XP aproximadamente no tamanho visual da 0.3, preservando tela cheia proporcional e F11.
* Log TXT detalhado de eventos reservado para a 0.5; nao implementar na 0.4.
* Nao confundir testes ARM com mapas sinteticos e testes locais com validacao de gameplay/UI no Windows. Publicacao da release 0.4 aprovada em 21/10/2026; manter solicitacao de aprovacao para novas releases.

Orientacao atual — candidata 0.5 (21/10/2026)

* Implementacao autorizada: log TXT limitado, backups rotativos, correcoes verificaveis de estabilidade e configuracao global de save compartilhado entre hosts/instalacoes.
* Manter o padrao LOCALAPPDATA/Destinia/save para compatibilidade com a 0.2–0.4. Versoes antigas nao leem config.ini novo nem respeitam a trava; documentar sem prometer conversao de saves.
* Diagnostico continua usando copias. Preservar gameplay, zoom, UI, ritmo e fluxo de loja/rede nesta etapa.
* Preparar candidata 0.5 com verificacoes e pacote revisado. A solicitacao de implementar nao autoriza publicar tag/release: apresentar resultados e limitacoes e aguardar aprovacao do autor.
