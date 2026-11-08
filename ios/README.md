# Destinia para iOS — emulação ARM32

Este diretório contém o port nativo iOS, mantido na branch [`ios/unicorn-poc`](https://github.com/sugyua/destinia/tree/ios/unicorn-poc).

## Versão Atual do Port iOS: v0.4.2 (Build 16)

- **Jogo Android original:** Gamevil/Everple **1.0.6**, package `game.destiniaeng`. A biblioteca `libdestinia_jni.so` e os assets originais são executados na CPU ARM32 via Unicorn TCI com acelerações nativas C++.
- **Projeto expandido Windows:** **0.17.4 Beta**, declarado em [`VERSION`](../VERSION), com mecânicas e interfaces implementadas nos adaptadores `host/`, `host_zoom/`, `web/` e `common/`.
- **Aplicativo iPhone:** Versão **0.4.2 (Build 16)** definida no workflow e Info.plist.

## Acelerações e Recursos Implementados (v0.4.2)

1. **Aceleração Nativa C++ para Dano Crítico (`_Grp_drawScreen16` e `_Grp_drawAlpScreen16`)**:
   - Elimina o gargalo crítico de **1,75 milhão de instruções interpretadas por frame**, reduzindo o tempo de renderização de ~168 ms para microssegundos nativos.
   - Dano crítico, brilhos e overlays de tela executam a **16 FPS estáveis (41.9 ms ARM32)** sem lag ou travamentos.
2. **Acelerações Gráficas Nativas Adicionais**:
   - `Grp_drawClipWImageP`: Cache de paleta de sprites e cópia direta.
   - `_Grp_drawDodge16`: Mistura de cores nativa em RAM/Direct Heap.
   - `ALPHA16` e `AlphaRow`: Cálculo nativo de transparência RGB565.
   - `Grp_fillRect`: Preenchimento direto de retângulos em memória.
3. **Temporização Precisa de 16 Hz**:
   - Agendamento em nanossegundos via GCD / CADisplayLink com tolerância de 0,5 ms.
4. **Empacotamento e Identidade**:
   - Ícones oficiais integrados ao bundle IPA (`AppIcon20x20` até `1024x1024`).
   - Bundle ID canônico: `com.destinia.tcidiagnostic` (preserva saves entre atualizações).
5. **HUD de Diagnóstico em Tempo Real**:
   - Métricas completas de FPS, tempo de frame, tempo ARM32, memória RAM, temperatura do dispositivo e profiling do Top 5 de funções mais custosas.

## Próximo Passo: Portar Funcionalidades do PC (v0.17.4)

- Integrar os adaptadores `common/` (Códex de Monstros e Itens, Sistema de Dificuldades Canônicas, Catálogo de Armamentos, Mural de Bounties e Modo Sobrevivência/Arena) diretamente no runtime iOS `main.mm` e `elf_probe.cpp`.
