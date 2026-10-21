# Instalação e compilação — Destinia 0.3 Beta

> **Versão Beta: pode conter diversos bugs. Faça backup dos saves.**

## O que está disponível nesta release de fontes

`destinia-0.3-beta-source.zip` contém os fontes dos dois hosts, seus CMakeLists, launchers normais de desenvolvimento e documentação. **Não contém executável Windows nem os dados originais do jogo.** Extrair o ZIP não instala um jogo pronto para jogar.

Não há um instalador, assinatura digital ou pacote de DLLs validado nesta entrega. Um ZIP Windows executável só deve ser publicado depois das verificações em máquina limpa descritas em `docs/RELEASE_0.3.md`.

## Requisitos para compilar no Windows

Os programas e bibliotecas externos são instalados separadamente, conforme a seção [Dependências do README](README.md#dependências). Seus instaladores, runtimes e ferramentas não serão anexados ao arquivo da release.

- Windows 64 bits com MSYS2, usando o terminal **UCRT64**.
- Compilador C++17, CMake 3.16 ou superior, Ninja e pkg-config/pkgconf.
- SDL2 e Unicorn para o mesmo ambiente UCRT64.
- SDL2_mixer com suporte a OGG/MP3, se desejar áudio.
- Arquivos compatíveis do Destinia Android obtidos pelo usuário.

Não misture DLLs e bibliotecas de MSYS, MINGW64 e UCRT64. O ambiente histórico do projeto registrou SDL2 2.32.10 e Unicorn 2.1.4; isso não é uma nova certificação dessas versões.

Instale as ferramentas pelo gerenciador `pacman` do MSYS2. Verifique os nomes disponíveis dos pacotes com `pacman -Ss SDL2`, `pacman -Ss unicorn` e `pacman -Ss SDL2_mixer`. Antes de configurar, confirme:

```sh
cmake --version
g++ --version
pkg-config --modversion sdl2 unicorn
pkg-config --modversion SDL2_mixer
```

A ausência do mixer permite compilar sem áudio. A ausência de SDL2 ou Unicorn impede a configuração.

## Compilar os dois hosts

No terminal UCRT64, entre na pasta extraída, por exemplo `/c/destinia`:

```sh
cmake -S host -B host/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build host/build
cmake -S host_zoom -B host_zoom/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build host_zoom/build
```

Saídas esperadas após compilação bem-sucedida:
- `host/build/destinia_host.exe`
- `host_zoom/build/destinia_host.exe`

Essas compilações ainda precisam ser executadas no Windows para validar esta candidata.

## Dados necessários em execução

A pasta `game` corresponde ao APK original do Destinia Android extraído. Disponha esses dados compatíveis ao lado de `host` e `host_zoom`:

```text
destinia/
  host/build/destinia_host.exe
  host_zoom/build/destinia_host.exe
  game/
    lib/armeabi/libdestinia_jni.so
    assets/...
    res/raw/...
```

A biblioteca ARM e os assets são necessários para iniciar. `res/raw` fornece música e efeitos. O host não executa `classes.dex`, o manifesto Android nem os certificados do APK. Não baixe DLLs avulsas de sites desconhecidos: use a mesma instalação UCRT64 da compilação.

## Primeiro teste com save separado

A partir da raiz, no terminal UCRT64:

```sh
./host/build/destinia_host.exe --game ./game --save ./save_release_02_host
./host_zoom/build/destinia_host.exe --game ./game --save ./save_release_02_zoom --lang pt
```

Essas pastas são criadas pelo host. Para testar progresso existente, copie primeiro o save completo para uma nova pasta e aponte `--save` para essa cópia. Não teste a candidata sobre o único save de uma partida.

Os launchers normais `Abrir Destinia.bat` e `Abrir Destinia Zoom 800x480.bat`:
- procuram o binário em `host/build` ou `host_zoom/build`;
- exigem `C:\msys64\ucrt64\bin\SDL2.dll` e acrescentam essa pasta ao PATH;
- usam o save compartilhado `%LOCALAPPDATA%\Destinia\save`;
- não compilam automaticamente e não ativam tradução PT no launcher de zoom.

Os scripts antigos de teste de tradução/compra do repositório completo ainda procuram `host_zoom/save_zoom800`. Essa origem pode não refletir a partida atual. Eles não estão no ZIP curado; não migre nem sobrescreva saves para satisfazer esses scripts.

## Controles

WASD/setas: movimento; Enter/Z/espaço: confirmar/ação; Esc/X: voltar; I/C: menu; F5: comando de save; Shift: aceleração manual. Gamepad também é suportado. O mapeamento e o funcionamento precisam ser confirmados durante a validação interativa.

Em `host_zoom`, F9 inicia/para a gravação quando iniciado com `--trace-viewport`. Preserve os BMPs e logs ao relatar o defeito visual de troca de mapa.

## Diagnóstico

- DLL ausente: confira o ambiente UCRT64 e suas dependências transitivas.
- Biblioteca ARM não encontrada: confira `--game` e a árvore `lib/armeabi`.
- Sem áudio: verifique SDL2_mixer, codecs, `res/raw` e a ausência de `--mute`.
- Tela ou clique incorreto: informe host, resolução da janela, escala DPI do Windows e passos de reprodução.
- Compra aguardando indefinidamente ou mapa duplicado: são problemas conhecidos; não são anunciados como corrigidos.
- O código de saída atual pode ser zero mesmo após uma falha emulada; confira também o log.

Não inclua saves pessoais em um relatório público sem revisá-los.

## Atualização do zoom: enquadramento original

O host_zoom agora renderiza o jogo em **400×240** e amplia a imagem para uma janela inicial de **800×480**, com pixels nítidos e proporção preservada. A imagem mostra o enquadramento original; deixa de mostrar uma área maior do mapa. Janelas com outra proporção ainda podem ter barras externas para preservar a imagem. PT, F9 e os comandos existentes continuam disponíveis.

**É necessário recompilar o host_zoom**, mesmo que já exista um EXE na pasta. Substituir somente o BAT não atualiza o jogo. Depois de extrair os fontes atuais sobre a pasta do projeto, execute no terminal MSYS2 UCRT64:

```sh
cd /c/destinia-main
cmake -S host_zoom -B host_zoom/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build host_zoom/build
```

Feche o jogo antes de compilar. Depois abra `Testar Destinia.bat`, escolha **1** e repita apenas a passagem que apresentou a faixa preta. Os BMPs e as coordenadas F9 agora são de 400×240; a ampliação ocorre somente na apresentação. O diagnóstico recusa o EXE antigo de 800×480 e pede recompilação. O host normal não foi alterado.

## Tela cheia no monitor ultrawide

O launcher `Abrir Destinia Zoom 800x480.bat` inicia o host_zoom com `--fullscreen`, em tela cheia sem bordas, usando a resolução atual do monitor. O nome antigo do BAT foi mantido para preservar atalhos. **F11** alterna entre tela cheia e janela. Pelo terminal, omita `--fullscreen` para iniciar em janela.

A proporção original é preservada, sem esticar nem cortar. Em um monitor 3440×1440, a área do jogo ocupa **2400×1440**, centralizada, com barras pretas externas de **520 pixels em cada lado**. O framebuffer e as capturas F9 continuam em 400×240. O mouse considera a área central, e cliques nas barras não acionam o jogo. Não é suporte nativo a ultrawide.

Recompile o host_zoom após atualizar os fontes. Para testar o mesmo modo no diagnóstico de câmera, escolha a opção 1 do `Testar Destinia.bat`: ela agora abre em tela cheia. Na janela guiada, confira também F11, clique em TAP TO START, Alt+Tab e o alinhamento do mouse.

## Compilação pelo BAT

Depois de instalar as dependências, feche o jogo e abra **Compilar Destinia.bat** na raiz do projeto. Escolha 1 para host_zoom, 2 para host ou 3 para ambos. O BAT configura e compila com os mesmos comandos CMake/Ninja da seção manual, reaproveitando o build existente e parando na primeira falha. Ele mantém a janela aberta para leitura do resultado e mostra o caminho do EXE. Não limpa caches, instala ferramentas, abre o jogo ou acessa saves.

O BAT precisa do arquivo `scripts/compile_windows.sh` junto ao projeto. O caminho padrão é `C:\msys64`; se o seu MSYS2 estiver em outro local, execute pelo Prompt:

```bat
set "DESTINIA_MSYS2_ROOT=D:\msys64"
"Compilar Destinia.bat" host_zoom
```

Também aceita `host` e `ambos`. Sem argumento, apresenta o menu. Se o CMake informar incompatibilidade de gerador ou de pasta de origem em um cache antigo, envie o erro para análise; o BAT não apaga o build automaticamente. A primeira compilação pode demorar mais; nas seguintes, o Ninja recompila somente o necessário. Mudanças apenas na documentação ou no diagnóstico não exigem recompilar o jogo.

Os testes do executor foram realizados em Linux com ferramentas simuladas: caminhos com espaços, ambos os hosts, recusa de alvo inválido, falha de configuração e falha de build com propagação do código e interrupção. O menu e a execução completa pelo Bash do MSYS2 ainda precisam de confirmação no Windows.
