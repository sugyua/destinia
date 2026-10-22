# Destinia 0.7 Beta — correção fix2

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs. Faça backup dos saves.**

## O que foi corrigido

Miniaturas do Códex montadas com as âncoras e cores RGB originais, nos dois hosts e no navegador. O Imperial Soldier volta a mostrar corpo e arma unidos. Nome, estado, mortes, paginação e arquivo `codex-v1.txt` são preservados.

A câmera do zoom deixava um observador de memória ativo em toda a emulação, apesar do filtro abranger somente quatro bytes. Agora a centralização é aplicada após 16 instruções STRH identificadas em `Map_setMapView`, `moveHero`, `GameCanvas_mPaint`, `procScript` e `GameCanvas_mRun`. As leituras globais deixam de ser instrumentadas. A biblioteca inteira deve corresponder à original 1.0.6 já examinada (SHA-256 `F12F1D20AB42646EF81DF27516851ACCCB9FFFE7BE07A054DB83890D992DF7E9`); a verificação FNV é de compatibilidade, não de segurança.

Zoom out 800×480, centralização dos mapas pequenos, fundo cinza, UI, texto XP e proporção da tela cheia continuam presentes. Não há alteração do intervalo pedido pelo jogo, combate, XP, itens ou saves.

## Evidência de desempenho

Comparação em processos novos no Linux, mesma cena da introdução ARM, sem apresentação SDL, quadro final 402:

| Medida | Observador de memória anterior | Pontos de atualização |
| --- | ---: | ---: |
| Mediana de emulação por quadro | 216,22 ms | 88,93 ms |
| Média de emulação por quadro | 208,68 ms | 98,52 ms |
| Intervalo pedido pelo jogo | 60 ms | 60 ms |

Câmera final `(-160, 0)`, mapa 30×30 tiles. BMP final idêntico entre as implementações: SHA-256 `8057a2d46e2e976ca428cab19c42fa1ead6ab5311774d76ec6a6ba4bd60dc3ad`. Remover o observador depois de iniciar o processo não é uma comparação equivalente: o Unicorn pode manter blocos já instrumentados até a limpeza posterior do cache.

Esses tempos pertencem ao ambiente de teste e à cena examinada. Não medem combate ou FPS no monitor do autor e não garantem a mesma melhora em todas as máquinas. O pedido de 60 ms corresponde a cerca de 16,7 quadros por segundo nessa cena; acelerar o jogo para 60 FPS não faz parte desta correção.

## Verificações

- Comparação web pixel a pixel com `drawAGDImageT` original nos IDs 1, 4, 61, 100, 102 e 109, incluindo transparência; referências RGB565 correspondentes no C++.
- Leitura dos 73 IDs de monstros/variantes/chefes e recusa de recursos truncados ou corrompidos.
- Execução das 16 instruções reais de escrita de câmera, conferindo os opcodes, mapas 30×30, 60×60 e 20×10 e preservação dos eixos que já cobrem a tela.
- Janela/textura 800×480, margens cinzas, área do mapa preservada, XP com 14 pixels de altura e tela cheia proporcional em 3440×1440.
- Suíte nativa, compilação dos dois hosts e verificações Windows no workflow do Códex; pacotes com EXEs novos e checksums.
- Chromium com toque em retrato/paisagem: Códex dentro do menu, soldado/chefes, páginas, contadores e retorno à mesma opção do menu.

A campanha completa, combate prolongado e desempenho no Windows do autor continuam pendentes. Safari/iPhone físico também permanece pendente.

## Atualização e teste no Windows

1. Feche os dois jogos e faça backup da pasta de saves configurada.
2. Baixe o ZIP **windows-x64** da release `v0.7-codex-imagens-fix2` e extraia sobre a instalação, substituindo os arquivos do pacote. Preserve `game` e a pasta compartilhada de saves.
3. As dependências continuam na seção **Dependências** do README; elas não acompanham o ZIP. Os EXEs já estão compilados; não é necessário compilar para usar esse pacote.
4. Abra `Abrir Destinia Zoom 800x480.bat`. Teste andar/lutar com o Códex fechado e confira a miniatura do Imperial Soldier dentro de **Menu → CODEX**.
5. Confira as passagens entre mapas, margens cinzas e F11. Se a lentidão ou câmera persistir, envie o TXT da sessão em `relatorios`; o teste de câmera continua disponível em `Testar Destinia.bat`.

`BUILD_INFO.txt` identifica o commit de compilação; os arquivos `.sha256` verificam a integridade dos ZIPs. As releases anteriores são preservadas.
