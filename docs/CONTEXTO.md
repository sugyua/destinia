# Contexto do projeto Destinia

Data desta reconstrução: 2026-10-12.

## Objetivo

Portar o jogo Android antigo **Destinia** (`game.destiniaeng`, Gamevil) para PC preservando a lógica original da biblioteca nativa ARM `libdestinia_jni.so`, sem depender dos serviços Android/Google Play/servidores Gamevil originais.

A abordagem atual não reimplementa o jogo: o host C++ carrega a `.so` original em um emulador ARM e fornece uma camada equivalente ao Java/Android necessário para que a biblioteca rode no PC.

## Fontes analisadas nesta etapa

- `C:\destinia\Claude\HOST_SPEC.md`
- `C:\destinia\Claude\main.cpp`
- `C:\destinia\Claude\exports\extraidos\conversas\conversations.json`
- `C:\destinia\Claude\manifest-8868cc6d-993f-4a87-92ad-30d9a9e24bcf-1791778786-eb73fbba-2026-10-12-04-20-04.json`
- `C:\destinia\host\main.cpp`
- `C:\destinia\host\CMakeLists.txt`
- `C:\destinia\host\log.txt`
- metadados de `C:\destinia\host\build\...`
- arquivos relevantes em `C:\destinia\game`, incluindo `classes.dex`, `AndroidManifest.xml`, `lib\armeabi\libdestinia_jni.so`, `assets` e `res\raw`

Observação: `C:\destinia\Claude\conversations\` **não existe** no estado atual. O histórico disponível está em `C:\destinia\Claude\exports\extraidos\conversas\conversations.json`.

## Arquitetura atual do host

O host existente em `C:\destinia\host\main.cpp` executa a biblioteca nativa original ARM 32 bits no PC:

- Usa **Unicorn** para emular ARM (`#include <unicorn/unicorn.h>` em `C:\destinia\host\main.cpp:13`).
- Usa **SDL2** para janela, textura e apresentação de framebuffer (`#include <SDL.h>` em `C:\destinia\host\main.cpp:12`).
- Carrega `game\lib\armeabi\libdestinia_jni.so` por padrão (`C:\destinia\host\main.cpp:442`).
- Implementa um carregador ELF mínimo para mapear segmentos, resolver símbolos exportados e aplicar relocations (`C:\destinia\host\main.cpp:304` em diante).
- Emula imports de libc/libm/liblog/libstdc++ usados pela `.so`, como `memcpy`, `malloc`, `clock`, `fopen`, `fread`, `fwrite`, `remove`, divisões `__aeabi_*` e log Android (`C:\destinia\host\main.cpp:172-230`).
- Fornece um **JNI falso** com arrays Java, `FindClass`, `GetStaticMethodID`, `GetByteArrayElements`, `GetJavaVM`, callbacks de assets, som, vibração, compras e log (`C:\destinia\host\main.cpp:161-300`).
- Chama os símbolos JNI nativos exportados `initGame`, `jniRun` e `jniTouch` (`C:\destinia\host\main.cpp:477-481`).
- Chama `initGame(env,obj, deviceID, len, modelo, len, width, height, isKorean)` com `400x240` e `isKorean=0` (`C:\destinia\host\main.cpp:483-490`).
- Roda `jniRun` a cada quadro e usa o valor retornado como tempo de espera; `0` indica pedido de encerramento (`C:\destinia\host\main.cpp:523-541`).
- Em modo com janela, cria uma janela SDL escalável e apresenta o framebuffer como textura `SDL_PIXELFORMAT_RGB565` (`C:\destinia\host\main.cpp:496-504`, `C:\destinia\host\main.cpp:533-536`).
- Em modo headless, grava BMPs dos quadros 1 e múltiplos de 10 (`C:\destinia\host\main.cpp:529-532`).

## Contrato nativo confirmado

A especificação em `HOST_SPEC.md` afirma que:

- A lógica do jogo está na `.so`; o Java original era uma casca para janela, toque, som, vibração, rede, anúncios e compras (`C:\destinia\Claude\HOST_SPEC.md:10`).
- A renderização é por software: a `.so` escreve em um buffer e o Java/host copia para a tela (`C:\destinia\Claude\HOST_SPEC.md:11`).
- Assets são entregues como bytes crus; a própria `.so` decodifica ZCP/WBI (`C:\destinia\Claude\HOST_SPEC.md:12`).
- Saves são feitos pela `.so` via stdio em `/data/data/game.destiniaeng/files/%s`; no PC basta redirecionar esse prefixo (`C:\destinia\Claude\HOST_SPEC.md:13`).
- O framebuffer é RGB565 (`C:\destinia\Claude\HOST_SPEC.md:88-91`).
- `jniRun` executa um quadro e retorna o tempo até o próximo quadro; `0` indica saída (`C:\destinia\Claude\HOST_SPEC.md:91`).
- `jniTouch` tem ordem provável/confirmada pelo código do host como `(x, y, tipo)`, com tipos 0 pressionar, 1 soltar, 2 mover e 3 tecla/menu (`C:\destinia\host\main.cpp:493-494`).

## Estado atual efetivo

### Código presente

- `C:\destinia\host\main.cpp` e `C:\destinia\Claude\main.cpp` são byte a byte idênticos:
  - tamanho: `31082` bytes em ambos
  - SHA-256: `9c52896ebd2656f9d06ee7f160fc47638f77c0c991c6b33b709682f5164a98b7`
- `C:\destinia\host\CMakeLists.txt` existe e configura C++17, SDL2 e Unicorn via pkg-config (`C:\destinia\host\CMakeLists.txt:1-11`).
- Não foi encontrado `LEIA-ME.md` no estado atual, embora o histórico exportado mencione que ele existia nos artefatos apresentados em uma sessão anterior.

### Build registrado

Há evidências de build local no diretório `host\build`:

- `CMakeCache.txt` foi gerado para `c:/destinia/host/build` por `C:/msys64/ucrt64/bin/cmake.exe` (`C:\destinia\host\build\CMakeCache.txt:1-4`).
- CMake encontrou Ninja em `C:/msys64/ucrt64/bin/ninja.exe` (`C:\destinia\host\build\CMakeCache.txt:92-93`).
- CMake usou `C:/msys64/ucrt64/bin/c++.exe` como compilador C++ (`C:\destinia\host\build\CMakeCache.txt:27-28`).
- SDL2 foi encontrado, versão `2.32.10`; Unicorn foi encontrado, versão `2.1.4` (entradas internas do `CMakeCache.txt`).
- O `build.ninja` contém a regra de link do executável `destinia_host.exe` e linka SDL2 e Unicorn (`C:\destinia\host\build\build.ninja:65-67`).
- `.ninja_log` registra duas compilações/linkagens do objeto e do executável (`C:\destinia\host\build\.ninja_log:2-5`).
- O executável atual `C:\destinia\host\build\destinia_host.exe` existe:
  - tamanho: `907327` bytes
  - SHA-256: `aea6f11d4d37bff687f3cdff288fdc7c5176b528019bb43e4bcb557d7d2b94f8`

### Execução headless registrada

O arquivo `C:\destinia\host\log.txt` registra uma execução headless de 60 quadros:

- A `.so` foi carregada com `568 KB` e `716` símbolos exportados (`C:\destinia\host\log.txt:1`).
- `initGame` concluiu e o loop começou (`C:\destinia\host\log.txt:33`).
- O jogo carregou assets iniciais, incluindo `gv_ani.agd`, `gv555.wmb`, `gv_lgt.wmb`, `gvimg.wpn`, `game_grade.wmb`, `title.wmb` e vários arquivos de UI/texto (`C:\destinia\host\log.txt:36-70`).
- Som ainda não está implementado; os pedidos `playSound 50` e `playSound 100` foram apenas registrados (`C:\destinia\host\log.txt:41-42`, `C:\destinia\host\log.txt:71-72`).
- A execução terminou com `60 quadros executados. Imagens em frames` (`C:\destinia\host\log.txt:73`).

A pasta `C:\destinia\host\frames` contém BMPs de `400x240` gerados pelo modo headless: `frame_00001.bmp`, `frame_00010.bmp`, `frame_00020.bmp`, `frame_00030.bmp`, `frame_00040.bmp`, `frame_00050.bmp`, `frame_00060.bmp`.

O histórico exportado registra que, com base nessas imagens, o jogo chegou a renderizar a animação/logotipo da **Gamevil** e a tela de título com **“TAP TO START”** em modo headless. Isso deve ser tratado como fato confirmado pelo histórico e pelos artefatos de frames existentes, mas **não** como validação de interação em janela real.

## Estado do diretório `game`

Arquivos relevantes presentes:

- `C:\destinia\game\classes.dex`
  - tamanho: `316820` bytes
  - mtime: `2011-12-14T09:49:44`
  - SHA-256: `805e64faa3b34338d5320140c9fbe8d41032bcb238e3a3cbce5d7e4d657b4b45`
- `C:\destinia\game\lib\armeabi\libdestinia_jni.so`
  - tamanho: `284144` bytes
  - mtime: `2011-12-14T09:45:32`
  - SHA-256: `f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9`
- `C:\destinia\game\AndroidManifest.xml`
  - arquivo binário AXML extraído do APK, não XML textual simples
  - contém strings como `game.destiniaeng`, permissões de rede/vibração/telefone, Billing, AdMob e Tapjoy
- `C:\destinia\game\assets`: 828 arquivos, ~2.65 MB
- `C:\destinia\game\res\raw`: 35 arquivos de áudio, ~3.42 MB
- `C:\destinia\game\lib\armeabi`: 1 biblioteca nativa

## Backup

A cópia de segurança indicada pelo usuário fica em:

- `C:\destinia_backup_20261012`

Ela não foi modificada nesta etapa.
