> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

# Destinia 0.3 Beta

Esta versão registra o port atual, com o enquadramento original ampliado e tela cheia proporcional. **O zoom out anterior não está ativo nesta versão.** Fundo cinza, centralização dos mapas pequenos com zoom out e UI menor ficam para uma atualização posterior.

## Mudanças desde a 0.2

- host_zoom com framebuffer original 400×240, ampliado com pixels nítidos. As capturas também são 400×240. PT, F9, HUD e controles preservados.
- Tela cheia sem bordas por `--fullscreen`, alternância por F11 e launcher do zoom abrindo em tela cheia.
- Imagem centralizada e proporcional: em monitor 3440×1440, área de 2400×1440 com barras laterais de 520 pixels. Mapeamento do mouse acompanha essa área.
- BAT de testes com roteiro geral ou teste focado de câmera, logs, BMPs, comparação de hashes e saves isolados.
- Relatórios TXT/JSON atualizados durante o teste, com indicação de sessão parcial, falhas, avisos e pendências.
- Avisos para compra observada, alterações locais de cash e rede aguardando resposta; isso não corrige nem valida a loja.
- Limpeza de logs, backups e material pessoal; proteção adicional contra envio de saves e credenciais.
- Documentação, contato para colaboradores, manifesto de distribuição, testes automatizados e versionamento atualizados. [Changelog completo](CHANGELOG.md).

## Instalação e dependências

Distribuição **em código-fonte**, sem EXE Windows pronto para executar. O ZIP curado não inclui APK, biblioteca ARM, assets, áudio, saves, DLLs, instaladores ou ferramentas externas.

Instale separadamente as dependências da seção **Dependências** do README: ambiente MSYS2 UCRT64, compilador C++17, CMake/Ninja, pkg-config, SDL2 e Unicorn; SDL2_mixer para áudio. O diagnóstico usa Windows PowerShell 5.1. Disponha dos dados originais compatíveis e siga INSTALL.md. Quem já usa o projeto precisa recompilar para receber a identificação 0.3 nos títulos.

## Verificações e pendências

Compilação e inicialização verificadas em Linux; testes do empacotador e diagnóstico, rotina ARM da câmera e alternância F11 exercitados. A geometria 3440×1440 foi simulada, não validada em monitor físico Linux. O usuário informou ausência de defeito de mapa no Windows com o enquadramento 400×240; a sessão não continha BMPs F9.

A UI original continua grande após ampliação. A câmera com zoom out ainda não está corrigida; o enquadramento original evita o caso que produzia a faixa lateral. Compras podem alterar cash local e ficar aguardando rede. Não foram validados compra online, persistência desse saldo, todos os casos internos de emulação, áudio nesta preparação ou instalação Windows em máquina limpa.

O ZIP curado e o SHA-256 estão na página da release no GitHub. Nenhum instalador ou dependência externa está incluído. A publicação foi aprovada pelo usuário; o jogo permanece Beta.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
