# Destinia 0.2 Beta — primeira release do port

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

**Pré-lançamento em código-fonte, aprovado pelo autor.**

Esta primeira release identifica o port como 0.2 Beta e organiza sua documentação e distribuição em código-fonte. O port utiliza a biblioteca ARM original via Unicorn, com janela/entradas SDL2 e áudio opcional SDL2_mixer.

## O que mudou

- Versão 0.2 Beta no título da janela dos dois hosts.
- Instruções em português para compilar, instalar dependências e executar com saves separados.
- Changelog, relatório de preparação e inventário de dependências.
- Seção Dependências no README: programas e bibliotecas externos devem ser instalados separadamente e não acompanham o arquivo da release.
- Exportação dos arquivos da tag sem o APK Android extraído, backups e logs selecionados; a pasta game permanece no repositório.
- Pacote curado de fontes e arquivo SHA-256, sem saves ou dados originais.

## Download

- `destinia-0.2-beta-source.zip`: fontes dos hosts e material de compilação.
- `destinia-0.2-beta-source.zip.sha256`: verificação de integridade.

**Não é um pacote Windows pronto para executar.** Não contém EXE, DLLs ou dados originais do jogo. Consulte INSTALL.md.

Não houve mudança de gameplay nem correção dos bugs de câmera, compras/rede ou emulação. Os testes executados cobrem o empacotador; a compilação e a execução Windows desta candidata não foram realizadas.

## Estado e escopo

Tag `v0.2-beta`, marcada como **pré-lançamento**. A pasta `game` é o APK Android extraído; não acompanha o ZIP curado nem os arquivos automáticos da tag, controlados por `.gitattributes`. Os dados compatíveis precisam ser fornecidos separadamente pelo usuário.

Leia CHANGELOG.md e docs/RELEASE_0.2.md para a lista completa de alterações, limitações e verificações.
