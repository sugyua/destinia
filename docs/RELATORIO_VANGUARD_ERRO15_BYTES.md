# Relatorio Vanguard: Erro 15 e bytes ARM

Execucao Node/Unicorn com engine.mjs e ELF original. Fixture exclusivamente em RAM; nenhum save de usuario foi lido ou escrito. Slot sem arquivo: nivel da selecao definido em RAM para exercitar confirmacao; isto nao equivale a validar personagem persistido.

LIMITACAO: confirmacao via handler nao publicou pacote; captura abaixo usa Comm_init/Comm_write/Comm_flush ARM diretamente com payload zero controlado, nao request de personagem real.
## Comm.sendBuf cmd=0 (offset 0x70bd0, 111 bytes)
```text
00 00 00 6f 00 00 00 00 00 00 00 00 28 44 45 53 54 49 4e 49 41 2d 57 45 42 2d 54 45 53 54 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

## outPacket: header LE + payload BE (offset 0x6dcb4, 66 bytes)
```text
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

## Comm.pendingSendLen LE (offset 0x73b2c, 4 bytes)
```text
6f 00 00 00
```

Estado apos confirmacao: Menu=8, netState=1, timer=0.
Frame relativo 150: netState=1, timer=150, Menu=0.
Frame relativo 151: netState=2, timer=0, Menu=0.

Mensagem capturada: "Ocorreu um erro de rede.\nCodigo de Erro = 15"
## netData+260 incluindo terminador (offset 0x81cf8, 45 bytes)
```text
4f 63 6f 72 72 65 75 20 75 6d 20 65 72 72 6f 20 64 65 20 72 65 64 65 2e 0a 43 6f 64 69 67 6f 20 64 65 20 45 72 72 6f 20 3d 20 31 35 00
```

Cota de 5h: sem telemetria acessivel; 18% informado pelo usuario no inicio. Nenhuma afirmacao de percentual atual.
