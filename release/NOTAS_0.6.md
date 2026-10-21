# Destinia 0.6 Beta — candidata para aprovação

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

A 0.6 prepara configurações persistentes e executáveis Windows dos dois hosts. A release pública atual continua sendo a 0.5; esta candidata aguarda aprovação para publicação.

## O que mudou desde a 0.5

- `Configurar Destinia.bat`: vídeo por host, áudio, pausa e perfis em config.ini global, com validação antes de guardar e opção de restaurar padrões preservando a pasta de saves.
- Tamanho de janela e modo tela cheia lembrados ao encerrar. F11 nos dois hosts, proporção original e mouse alinhado à imagem central. Zoom mantém 800×480, mapas centralizados, fundo cinza e UI da 0.4/0.5.
- Teclas e botões por ação e perfis nomeados, com detecção de conflitos. Comandos simultâneos deixam de se anular; perder foco ou desconectar o controle libera entradas presas.
- Música e efeitos com volumes separados e persistentes, combinados com o volume original; silêncio sem foco opcional. Ajustes de volume aguardam a abertura do áudio, evitando divisão por zero no mixer.
- Pausa manual (F10 por padrão) e opcional por perda de foco ou controle, com pausa/retomada do áudio. F10 permite continuar pelo teclado após desconectar o controle.
- Dois EXEs Windows x64 compilados e verificados automaticamente em MSYS2 UCRT64; pacotes de fontes e Windows separados, com checksums SHA-256.
- Diagnóstico usa cópias de save e não grava preferências pessoais. Mantidos log TXT, backups, save compartilhado e limitações de compras/rede.

Não foram alterados nível máximo, itens, regras da campanha, ritmo ARM ou escala nativa da UI; essas melhorias continuam no roadmap.

## Instalação e dependências

Extraia em pasta nova, instale separadamente as dependências de [README.md](../README.md#dependências) e disponha seus dados originais em `game`. O pacote Windows não precisa de compilação; abra um dos inicializadores BAT. Consulte [INSTALL.md](../INSTALL.md) e [PREFERENCIAS.md](../docs/PREFERENCIAS.md).

Os ZIPs não incluem DLLs, instaladores, ferramentas externas, APK/áudio/assets originais, saves, logs ou backups. O progresso continua em `%LOCALAPPDATA%\Destinia\save` ou na pasta global já escolhida, compartilhado entre normal, zoom e novas instalações compatíveis.

## Verificações e limites

Compilações e testes automatizados em Linux e Windows, com biblioteca ARM original, mixer real e saves descartáveis. Resultados e identificação dos artefatos em [RELEASE_0.6.md](../docs/RELEASE_0.6.md). Testes automáticos não substituem campanha manual, controle físico, audição ou inspeção visual no PC do autor.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
