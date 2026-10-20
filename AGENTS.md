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

https://github.com/sugyua/Destinia_PCport

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