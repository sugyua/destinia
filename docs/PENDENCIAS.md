# Pendências, próximos testes e questões em aberto

Data desta reconstrução: 2026-10-12.

## Regra para a próxima etapa

Esta etapa foi somente de leitura, exceto pela criação destes documentos em `C:\destinia\docs`. Não foram feitas alterações em `game`, `host`, código, configurações ou builds. Também não foi compilado nem executado o jogo.

Antes de implementar qualquer mudança, preservar:

- `C:\destinia\game`
- `C:\destinia\host`
- `C:\destinia\Claude`
- backup externo: `C:\destinia_backup_20261012`

## Estado confirmado antes das próximas ações

- O host C++ existe e compila no ambiente registrado.
- O executável `C:\destinia\host\build\destinia_host.exe` existe.
- Uma execução headless de 60 quadros está registrada em `C:\destinia\host\log.txt`.
- Frames BMP existem em `C:\destinia\host\frames`.
- O histórico exportado registra que os frames mostram a animação/logotipo Gamevil e a tela de título com `TAP TO START` em modo headless.
- Não foi encontrada evidência de validação de clique/interação em janela real.

## Próximos testes recomendados

### 1. Teste de janela real

**Objetivo:** verificar se a janela SDL abre corretamente, se a escala está correta e se o clique na tela `TAP TO START` chama `jniTouch` na ordem esperada.

**Comando registrado no histórico, ainda não validado nesta análise:**

```sh
cd /c/destinia/host
./build/destinia_host.exe --game /c/destinia/game
```

**O que observar:**

- Se abre janela de 1200x720 por padrão (`400x240` com escala 3).
- Se a imagem corresponde aos frames headless.
- Se clique esquerdo na tela de título avança o estado do jogo.
- Se há crash, travamento, tela preta ou ausência de resposta.
- Se o `log.txt` ou terminal registra chamadas inesperadas a JNI, imports ausentes, rede, compras ou arquivo.

**Importante:** não afirmar que a interação em janela está validada até executar e registrar esse teste.

### 2. Teste de progressão inicial

**Objetivo:** descobrir até onde o jogo avança offline sem servidores Gamevil.

**Roteiro sugerido:**

1. Abrir janela.
2. Clicar em `TAP TO START`.
3. Tentar chegar ao menu principal.
4. Tentar iniciar um novo jogo.
5. Registrar se aparece pedido de rede, restauração de save, ranking, presente, certificação ou erro.

**Evidência atual:** `HOST_SPEC.md` considera rede o maior risco restante e sugere servidor falso em processo caso o jogo exija respostas (`C:\destinia\Claude\HOST_SPEC.md:107-110`).

### 3. Teste de saves

**Objetivo:** confirmar leitura/escrita de save redirecionada para pasta local.

**Estado atual:** o log mostra falha ao abrir `save/msave.dat` para leitura, esperado em primeiro boot sem save (`C:\destinia\host\log.txt:10-11`).

**O que validar:**

- Se o jogo cria arquivos em `host/save` ao iniciar/progredir.
- Se os arquivos são reabertos corretamente em uma segunda execução.
- Se há CRC/validação nativa impedindo saves criados pelo host.

### 4. Teste de áudio

**Objetivo:** implementar e validar música/efeitos depois que janela e progressão estiverem confirmadas.

**Estado atual:** `playSound` e `setSoundVolume` são callbacks conhecidos, mas áudio não toca; o host apenas registra chamadas.

**Evidência:**

- `playSound 50 [audio ainda nao implementado]` durante tela de logo/grade (`C:\destinia\host\log.txt:41-42`).
- `playSound 100 [audio ainda nao implementado]` durante carregamento/título (`C:\destinia\host\log.txt:71-72`).
- `game\res\raw` contém 35 arquivos de áudio (`bg100..bg107.mp3`, `eff00..eff25.ogg`, `eff50.ogg`).

**Pendência:** recriar a tabela Java `getSoundResID(int)` para mapear ids nativos para arquivos de áudio.

### 5. Compras/cash

**Objetivo:** decidir política de port para compras/cash.

**Estado atual:** `buyCash(id)` só é registrado e não responde sucesso/erro ao jogo.

**Opções documentadas em `HOST_SPEC.md`:**

- Responder sucesso (`procCommMsg(1, 0)`) e tornar compra gratuita.
- Responder erro/cancelamento.
- Esconder/desativar o botão no fluxo do jogo, se possível.

**Pendências:**

- Mapear id de produto para quantidade de cash.
- Verificar se menus de compra são acessíveis offline.
- Decidir design antes de implementar.

### 6. Rede e servidores Gamevil

**Objetivo:** garantir que o port não abra conexões externas reais e que o jogo funcione com respostas simuladas quando necessário.

**Estado atual:**

- O Java original tinha rede e servidores Gamevil.
- `HOST_SPEC.md` recomenda não abrir conexão nenhuma e simular respostas (`C:\destinia\Claude\HOST_SPEC.md:61`).
- O host atual implementa `close` como no-op e não implementa fluxo completo de `commCheck`, `procCommRead` ou `procCommMsg`.

**Questões abertas:**

- O jogo exige resposta de servidor para iniciar ou apenas para recursos opcionais?
- Quais comandos aparecem após `TAP TO START`?
- Como responder a restauração de save, ranking, presentes, revive, certificação e cash?

### 7. Licença/DRM

**Objetivo:** verificar se há checagem de licença que impeça progressão.

**Estado atual:** `HOST_SPEC.md` lista `saveLicensedStatus`/checagem de licença como pergunta aberta (`C:\destinia\Claude\HOST_SPEC.md:68`).

**Pendência:** validar durante progressão inicial ou por análise adicional do dex/.so.

### 8. Robustez do emulador/host

**Pendências técnicas:**

- Confirmar todos os imports usados pela `.so` em fluxos posteriores, não apenas nos 60 primeiros quadros.
- Verificar se stubs de `JNIEnv`/`JavaVM` cobrem chamadas que surgem em menus, gameplay, compras e rede.
- Melhorar diagnóstico sem alterar comportamento original.
- Validar timing de `clock()` e `jniRun`; o host usa tempo real em microssegundos no stub de `clock` (`C:\destinia\host\main.cpp:185-189`).
- Verificar se `sleepMs` e retorno `1000` de `jniRun` reproduzem bem o comportamento do Java (`C:\destinia\host\main.cpp:537-541`).

### 9. Documentação ausente

O histórico exportado menciona artefatos `LEIA-ME.md`, `main.cpp` e `CMakeLists.txt`, mas no estado atual só foram encontrados `main.cpp` e `CMakeLists.txt` em `C:\destinia\host` e `main.cpp` em `C:\destinia\Claude`.

**Pendência:** se necessário, recuperar `LEIA-ME.md` do histórico/backup, mas não presumir que ele existe no estado atual.

## Problemas conhecidos

- ~~Áudio não implementado.~~ **RESOLVIDO:** som via SDL2_mixer (`eff*.ogg`/`bg*.mp3` de `game/res/raw`), com cache de chunks/músicas e volume por canal.
- ~~Aquecimento de CPU.~~ **RESOLVIDO (2026-10-18):** `--fps` virou teto; ritmo honra o `wait` do jogo. Ver DECISOES_TECNICAS.md `[2026-10-18]`.
- Compras: cash local via `getMyCash`/`setMyCash`; resposta da loja ainda nao simulada (spinner pode ficar preso).
- Rede: superfície mapeada (`Comm_*`, buffer `SOCKETBUFFER`) mas nao implementada. Menu de rede (ranking/compra/presente/batalha) travaria em `rede=1` se acessivel; caminho single-player ate o menu principal nao exige rede.
- Progressão alem do menu principal ainda nao confirmada por teste interativo automatizado; a sonda mostra o jogo parado em `menu.estado=2` do quadro ~100 ao ~4700.
- AndroidManifest e layouts em `game` estão em formato binário AXML; leitura textual direta produz strings e bytes misturados.
- `Claude/main.cpp` (31 KB) esta defasado em relacao a `host/main.cpp` (~52 KB); a afirmacao antiga de que sao identicos nao vale mais.
- `C:\destinia\Claude\conversations\` não existe; usar o JSON exportado como fonte de histórico.

## Hipóteses a não tratar como fato sem novo teste

- Que o jogo é jogável além da tela `TAP TO START`.
- Que cliques/mouse funcionam corretamente em janela real.
- Que compras podem ser ignoradas sem travar menus.
- Que rede é totalmente opcional.
- Que saves funcionam até criação/carregamento real.
- Que todos os imports nativos necessários já foram exercitados pelos 60 quadros headless.
