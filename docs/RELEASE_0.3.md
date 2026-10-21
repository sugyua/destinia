# Preparação da release 0.3 Beta

## Escopo aprovado

O usuário escolheu registrar a versão atual: fontes baseados no commit 7e66e82c89a80c9b0be5eb36822cacc4909016a5, com framebuffer 400×240 ampliado, tela cheia e F11. Novos ajustes de zoom out, cinza, centralização dos mapas pequenos e UI ficam para depois.

Esta candidata altera somente identificação de versão, documentação, notas e manifesto em relação a essa base. O comportamento existente é preservado. A 0.2 Beta permanece intacta. Não há novo EXE Windows distribuível.

## Distribuição

- Tag proposta: v0.3-beta; pré-lançamento.
- ZIP curado: destinia-0.3-beta-source.zip, com SHA-256.
- Base de comparação: fa92ed696470ac68f2ad8274d9d2f9259f97f3ce (0.2 Beta).
- Arquivos incluídos definidos em release/manifest.json. APK extraído, saves, logs e dependências excluídos.
- Revisar também a exportação automática da tag, que tem regras próprias em .gitattributes.
- Publicação de tag/release depende da aprovação final do usuário após revisão das verificações.

## Validação do usuário

Sessão testes_20261020_205829_7498ef8f: inicialização 400×240, mapa/câmera marcado MANUAL_OK, nenhum BMP F9, dois avisos de loja/rede e hashes do save de origem inalterados. Código zero não valida os itens pendentes. A captura enviada depois evidencia a UI grande; não é anunciada correção de escala da UI.

## Verificação local da candidata

- Ambos os hosts compilados pelo CMake em Release, com GCC 14.2, SDL2 2.32.4 e Unicorn 2.1.4. SDL2_mixer não disponível neste ambiente: áudio não retestado.
- Sete testes Python passaram, incluindo inicialização real do host_zoom e BMPs 400×240 não vazios em dez quadros.
- Host normal inicializado separadamente com suas opções suportadas: dez quadros e BMPs 400×240 confirmados. Uma tentativa inicial de reaproveitar o smoke exclusivo do zoom falhou por argumentos não suportados e expectativa de log diferente; o teste específico acima substitui essa tentativa, sem alterar o host.
- Sete grupos de testes PowerShell do diagnóstico passaram; fluxo Camera e recusa de binário antigo também passaram.
- Diff dos fontes C++ em relação à versão atual do usuário: somente os dois títulos passam de 0.2 Beta para 0.3 Beta.
- Testes anteriores de janela/F11/câmera ARM e geometria ultrawide são documentados em VERIFICACAO_ZOOM_ORIGINAL.md; essa lógica está inalterada nesta candidata.
- ZIP curado conferido contra todos os arquivos do manifesto, sem game/saves/EXE/DLLs, com checksum SHA-256 válido. A exportação automática é conferida após o commit da candidata.

Não há teste em Windows nativo neste ambiente. As dependências de desenvolvimento locais não entram no pacote.
