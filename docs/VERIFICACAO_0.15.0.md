# Verificação da candidata local 0.15.0

## Escopo

Ficha somente leitura por Ctrl+F3 nos hosts normal e Zoom. Ela mostra PV e PM máximos, ATK, DEF, HIT, EVA, CRI e BLD lidos do bloco de 13 atributos que o cálculo nativo do jogo mantém em `Game + 0x48ba`. HIT/EVA/CRI/BLD são exibidos em porcentagem a partir das unidades nativas de décimos. O índice 4, sem semântica documentada, é deliberadamente omitido.

O painel não chama o recálculo de atributos, não altera equipamento, personagem, save ou sidecar. Os valores podem refletir o último cálculo nativo aplicado pelo próprio jogo.

## Verificações locais

- Teste C++ de mapeamento, conversão para décimos de porcentagem, alternância e renderização.
- Compilação Release CMake/Ninja dos hosts normal e Zoom após a integração.
- O acesso é Ctrl+F3; F10 continua reservado à pausa.
- Temperaturas foram acompanhadas usando CPU e temperatura principal da GPU: pico 55,25 °C e 40 °C nesta rodada; hotspot e VRAM não entram no limite.

## Limites

Não houve teste físico de gameplay nem inspeção visual durante uma partida. Não foi alterada a versão Web. Pacote curado, checksums, CI do SHA final, GitHub Release e deployment Web não foram executados.
