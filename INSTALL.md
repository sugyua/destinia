# Instalação e compilação — Destinia 0.6 Beta

> **Versão Beta: pode conter diversos bugs. Faça backup dos saves. Publicação aprovada pelo autor; campanha manual no Windows ainda pendente.**

## Pacotes

- `destinia-0.6-beta-windows-x64.zip`: dois EXEs do port, inicializadores BAT, configuradores e documentação. **Não precisa compilar.**
- `destinia-0.6-beta-source.zip`: fontes, testes, compilador BAT e documentação para desenvolvimento.
- Cada pacote acompanha `.sha256`. Não inclui dados do APK, saves pessoais, DLLs, instaladores ou ferramentas externas. A compilação Windows foi produzida pelo workflow `Compilar e testar candidata Windows`; os resultados estão em [RELEASE_0.6.md](docs/RELEASE_0.6.md).

## Dependências de execução

Instale separadamente o [MSYS2](https://www.msys2.org/), usando o ambiente **UCRT64**, e os pacotes correspondentes. No terminal UCRT64:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-unicorn mingw-w64-ucrt-x86_64-zlib
```

As compilações da `main` com miniaturas no Códex também precisam de **zlib UCRT64**, instalada pelo comando acima; sua DLL fica no MSYS2 e não acompanha os pacotes. O gerenciador instala também as dependências transitivas. Não misture DLLs de MSYS, MINGW64 e UCRT64. SDL2/Unicorn e suas bibliotecas de execução são obrigatórios; os EXEs da 0.6 são compilados com SDL2_mixer e também precisam de suas DLLs. Não baixe DLLs avulsas: use os pacotes do mesmo ambiente. Os inicializadores acrescentam `C:\msys64\ucrt64\bin` ao PATH apenas naquele processo.

Se o MSYS2 estiver em outro local, defina `DESTINIA_MSYS2_ROOT`, por exemplo no Prompt:

```bat
set "DESTINIA_MSYS2_ROOT=D:\msys64"
"Abrir Destinia.bat"
```

Windows PowerShell 5.1, normalmente presente no Windows, executa os configuradores e o diagnóstico. Instaladores, ferramentas e bibliotecas externas ficam fora dos ZIPs, conforme a seção [Dependências](README.md#dependências).

## Abrir o pacote Windows

1. Extraia em uma pasta nova; não copie executáveis antigos sobre os novos. Confira o SHA-256 com `Get-FileHash arquivo.zip -Algorithm SHA256` no PowerShell.
2. Disponha os dados originais compatíveis, obtidos pelo usuário, na estrutura abaixo. `game` é o APK Android extraído; não é instalado como um aplicativo Windows.
3. Abra **Abrir Destinia.bat** para o normal ou **Abrir Destinia Zoom 800x480.bat** para o zoom. Não execute o BAT de testes para continuar sua partida principal: ele usa cópias.
4. Para ajustes, feche os jogos e abra **Configurar Destinia.bat**. Para selecionar o progresso existente, use **Configurar pasta de saves.bat**.

```text
destinia/
  Abrir Destinia.bat
  Abrir Destinia Zoom 800x480.bat
  Configurar Destinia.bat
  scripts/...
  host/build/destinia_host.exe
  host_zoom/build/destinia_host.exe
  game/
    lib/armeabi/libdestinia_jni.so
    assets/...
    res/raw/...
```

A biblioteca ARM e os assets são necessários para iniciar; `res/raw` fornece música e efeitos. O host não executa classes.dex, manifesto Android nem certificados. Os inicializadores verificam a identificação `Destinia 0.6 Beta` antes de abrir o jogo.

## Compilar os fontes

Somente quem usa o ZIP de fontes precisa de compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf, além das dependências de execução:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf
pkg-config --modversion sdl2 unicorn SDL2_mixer zlib
```

Feche o jogo e abra **Compilar Destinia.bat**. O duplo clique compila ambos os hosts; os argumentos `host`, `host_zoom` e `ambos` selecionam o alvo. O BAT usa o MSYS2 já instalado, preserva os códigos de falha e não apaga caches ou saves nem instala ferramentas.

Alternativa no terminal UCRT64, na raiz do projeto:

```sh
cmake -S host -B host/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build host/build
cmake -S host_zoom -B host_zoom/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build host_zoom/build
```

Os fontes permitem compilação sem SDL2_mixer, com áudio desativado. Isso não remove a dependência do mixer dos EXEs já compilados com áudio. Cache CMake de outra pasta/gerador pode exigir diagnóstico; o BAT não o apaga automaticamente. Python é necessário apenas para testes e empacotamento, não para jogar.

## Progresso entre updates

O padrão continua **`%LOCALAPPDATA%\Destinia\save`**, fora da pasta do jogo. Normal, zoom e novas instalações compartilham esse progresso. Não é necessário começar de novo ou copiar saves para cada atualização.

**Configurar pasta de saves.bat** seleciona uma pasta existente e guarda `save_dir` em `%LOCALAPPDATA%\Destinia\config.ini`. Não copia nem sobrescreve partidas. Para compartilhar também com as versões 0.2–0.4, mantenha o caminho padrão: elas não leem a configuração personalizada nem respeitam a trava dos hosts novos.

Antes de carregar, os hosts criam backups verificados, guardam dez snapshots e impedem abrir a mesma pasta simultaneamente. Logs de sessão ficam em `relatorios/jogo_HOST_DATA/log.txt`. Veja [saves, restauração e logs](docs/SAVES_E_LOGS.md).

Para testar separadamente, faça uma cópia completa do save e use, por exemplo:

```sh
./host/build/destinia_host.exe --game ./game --save ./partida_teste_normal --no-save-preferences
./host_zoom/build/destinia_host.exe --game ./game --save ./partida_teste_zoom --lang pt --no-save-preferences
```

## Vídeo, áudio e comandos

Consulte [PREFERENCIAS.md](docs/PREFERENCIAS.md). Normal abre em janela 1200×720 por padrão; zoom abre em tela cheia proporcional, com janela 800×480 ao alternar por F11. Modo e tamanho são lembrados ao fechar normalmente. Em 3440×1440 a imagem ocupa **2400×1440**, centralizada, com 520 pixels de margem em cada lado. Cliques nas barras não acionam o jogo.

O zoom mantém framebuffer 800×480, mapas pequenos centralizados, fundo cinza, UI nativa menor e contador de XP ajustado. Normal continua 400×240. A 0.6 não altera câmera, UI nativa, progressão, itens ou ritmo da simulação.

Por padrão: WASD/setas movem; Enter/Z/espaço confirmam; Esc/X voltam; I/C abrem menu; F5 envia comando de save; Shift acelera; F10 pausa/retoma. Os perfis permitem remapear teclas e botões. F9 é reservado ao trace do zoom (`--trace-viewport`); F11 alterna tela cheia. Pausa automática e silêncio sem foco são opcionais e começam desativados.

## Diagnóstico e limitações

O ZIP de fontes inclui **Testar Destinia.bat**: opção 1 para câmera, 2 para roteiro geral dos hosts. O diagnóstico clona o save e usa `--no-save-preferences`; a sessão fica em `relatorios`. O pacote Windows enxuto não inclui os testes de desenvolvimento.

- DLL ausente ou falha ao verificar versão: confira MSYS2 UCRT64 e dependências transitivas.
- Biblioteca ARM não encontrada: confira a árvore `game/lib/armeabi`.
- Sem áudio: confira `res/raw`, SDL2_mixer, volumes originais e novos e ausência de `--mute`.
- Preferências inválidas: corrija config.ini com o jogo fechado ou use o configurador. Para validar sem abrir o jogo: `destinia_host.exe --check-settings CAMINHO`.
- Jogo pausado: retome pela ação de pausa (F10 no perfil padrão), devolva o foco ou reconecte o controle, conforme a opção ativada.
- Tela, câmera ou cliques incorretos: informe host, resolução/DPI e passos de reprodução. Testes automáticos não comprovam todas as transições ou legibilidade.
- Compras/rede continuam com limitações, incluindo espera por resposta da loja. Não foram corrigidas nesta versão.

Não envie saves pessoais ou logs de partidas a relatórios públicos sem revisá-los.
