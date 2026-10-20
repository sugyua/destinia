# Destinia (Gamevil, `game.destiniaeng`) — Especificação do host para o port PC

Resultado da Fase 2. Fonte: `classes.dex` (lido com `dexinfo.py`) cruzado com as
strings da `libdestinia_jni.so` (ARMv5TE, ELF32, soft-float, só depende de libc/libm/liblog/libdl/libstdc++).

Legenda de certeza: **[C]** confirmado nas duas pontas (dex + .so) · **[I]** inferido pelo nome/assinatura, falta confirmar no Ghidra · **[?]** desconhecido.

## 1. Modelo geral

- Toda a lógica do jogo está na `.so`. O Java é só "casca": janela, toque, som, vibração, rede, anúncios e compras. **[C]**
- A `.so` faz **renderização por software**: ela escreve o quadro num `byte[]` e o Java copia para um `Bitmap` e desenha escalado na tela (`GameThread.buffer`, `BB`, `mBitmap`, `srcRect`/`dstRect`). Existem `Bitmap.Config.RGB_565` e `ARGB_8888` no dex. **[C]** que é software; **[I]** que o formato do framebuffer é RGB565 (confirmar).
- A `.so` tem o próprio decodificador de `ZCP` (inflate embutido) e de `WBI`. O host só precisa entregar os **bytes crus** do arquivo; não precisa descomprimir nada. **[C]**
- Saves são feitos pela própria `.so` via stdio (`fopen/fread/fwrite/remove`), no caminho `/data/data/game.destiniaeng/files/%s`, com arquivos como `msave.dat` e checagem de CRC. Outros arquivos do Java: `uniquekey.dat`. **[C]** → no PC basta redirecionar esse prefixo para uma pasta local.

## 2. Funções que o host chama na `.so` (Java → nativo)

| Função | Assinatura | Papel provável | Certeza |
|---|---|---|---|
| `GameThread.initGame` | `([BI[BIIIB)V` | inicialização: provavelmente deviceID/modelo (byte[]), tamanho, tela, flags | **[?]** significado de cada parâmetro |
| `GameThread.jniRun` | `([BI)I` | roda 1 quadro e escreve no framebuffer (byte[] + tamanho) | **[I]** |
| `GameThread.jniTouch` | `(III)V` | evento de toque (tipo, x, y) | **[I]** |
| `GameThread.destroyGame` | `()V` | encerra | **[C]** |
| `GameThread.setGamePlaying` | `(B)V` | informa pausa/foco | **[I]** |
| `GameThread.setUniqueID` | `([BI)V` | identificador do aparelho | **[I]** |
| `GameThread.setEarnedCoin` | `(I)V` | moedas ganhas por Tapjoy (log: `setEarnedCoin : %d`) | **[C]** |
| `GameThread.bannerShow` | `()I` | nativo diz se mostra banner | **[I]** |
| `GameThread.procCommMsg` | `(II)I` | resultado/estado de uma mensagem de rede | **[I]** |
| `GameThread.procCommRead` | `([B)I` | entrega pacote recebido do servidor | **[C]** (logs `procCommRead start/end`) |
| `CommManager.commCheck` | `([B)I` | checagem de conexão | **[I]** |
| `CommManager.commGetConType` | `()I` | tipo de conexão atual | **[I]** |

## 3. Funções que a `.so` chama no host (nativo → Java)

Esta é a lista que o host precisa implementar (os nomes e assinaturas vêm das strings da `.so`; o comportamento vem do dex).

| Método Java | Assinatura | O que o host faz no PC | Certeza |
|---|---|---|---|
| `GameThread.getAssetResSize` | `([B)I` | recebe nome do arquivo, devolve o tamanho em bytes | **[C]** |
| `GameThread.getAssetRes` | `([B)[B` | devolve os bytes do arquivo da pasta `assets/` | **[C]** |
| `GameThread.playSound` | `(I)V` | toca música (bg, mp3) ou efeito (ogg) pelo id | **[C]** |
| `GameThread.setSoundVolume` | `(III)V` | volume | **[C]** |
| `GameThread.playVib` | `(I)V` | vibrar → no PC: ignorar (ou rumble de gamepad) | **[C]** |
| `GameThread.gotoURL` | `([B)V` | abrir URL → no PC: ignorar | **[C]** |
| `GameThread.logEvent` | `([B)V` | log/analytics → no PC: ignorar | **[I]** (nome aparece na `.so`) |
| `CommManager.buyCash` | `(I)V` | **pedido de compra** de "cash" → ver seção 4 | **[C]** |
| `CommManager.close` | `()V` | fecha socket → no-op | **[C]** |
| `Destinia.writeStringGV` | `(Ljava/lang/String;I)V` | monta pacote de rede → ver seção 4 | **[I]** |

Observações:
- O mapeamento id → arquivo de som é feito em Java (`getSoundResID(I)I`, `R$raw`: `bg100..bg107.mp3`, `eff00..eff25.ogg`, `eff50.ogg`). O host precisa recriar essa tabela. **[I]**
- A fonte `dotum08.fnt` é carregada pela própria `.so`. **[C]**
- Há tela de classificação indicativa (`game_grade.wmb`, `grade.png`, `drawScreenGrade`, `GAME_ST_GRADE`) que é uma regra de mercado de 2011; no port pode ser mantida ou pulada. **[I]**

## 4. Anúncios, compras e rede (o que remover/neutralizar)

**Anúncios** — todos no Java; nada disso entra no port:
`com.google.ads` (AdMob), Flurry, Tapjoy (`TJCOffers`, `earnedTapPoints`, `videoReady/Complete/Error`), banners em `adsLayout`, `setShowAds/setShowBanner`, pré-carregamento de anúncio em `gamevil.com/preload/...`. A única ponta nativa é `bannerShow()` e `setEarnedCoin(int)`. → o host nunca chama `setEarnedCoin` e trata `bannerShow` como "não mostrar".

**Compras** — Google Play Billing v1 (`BillingService`, `BillingReceiver`, `PurchaseObserver`, `Security`, `PurchaseDatabase`). Produtos encontrados: `destinia_1000cash`, `destinia_6500cash`, `destinia_14000cash`, `destinia_45000cash`, `destinia_80000cash`, `destinia_170000coin`, `destinia_revive_800cash`. A verificação de assinatura (`Security.verify`) é do Google Play e some junto. → falta decidir o comportamento (seção 5).

**Rede** — `CommManager` abre socket TCP para servidores Gamevil fixos no código, com tipos de conexão: `GAMESERVER`, `GV_CASH`, `GV_CERTI`, `T_CASH`, `DSP`, e um pacote `CHECK_UDID`. Esses servidores já não devem existir. O port **não deve** abrir conexão nenhuma; o host simula as respostas.

## 5. Perguntas em aberto (próximas investigações)

1. Parâmetros de `initGame` e resolução real do framebuffer (largura × altura × formato). **Bloqueia a Fase 3.**
2. O jogo exige resposta de servidor para **iniciar** (UDID/certificação) ou joga offline? Se exigir, quais bytes `procCommRead`/`procCommMsg` esperam.
3. O que é "cash" no jogo e o que ele compra? (Define o que fazer com as compras: liberar, preço zero ou esconder do menu.)
4. Há checagem de licença (`saveLicensedStatus`) que impeça o jogo de rodar?
5. Como `jniRun` controla a taxa de quadros (o Java tem `sleepTime`).
6. O que `procCommMsg`/`commCheck` devolvem em cada estado (`COMM_ST_*`, `NETCB_TYPE_*`).

## 6. Próximos passos

1. Ghidra na `.so`: confirmar `initGame` e `jniRun`; achar a rotina que escreve o framebuffer.
2. Host mínimo em C++ com Unicorn: carregar a `.so`, `initGame`, `jniRun` em loop, mostrar o framebuffer numa janela SDL2.
3. Implementar as funções da seção 3 (asset, som, no-ops).
4. Resolver as perguntas 2–4 com a lógica rodando.

---

# Atualização 1 — resultados do export do Ghidra (`libdestinia_jni_so.c`)

Tudo abaixo vem do código decompilado e **substitui** os itens marcados [I]/[?] acima quando houver conflito.

## A. O jogo é um app **WIPI** (plataforma móvel coreana) com uma camada de compatibilidade
A `.so` contém o jogo original em C para WIPI mais uma implementação das APIs `MC_*` (`MC_grpSetScreenBuffer`, `MC_fsOpen/Read/Write`, `MC_knlExit`, `MC_knlCurrentTime`, `WIPI_init`, `WIPI_finalize`...). Os nomes das funções do jogo (`MMain_run`, `LogoCanvas_init`, `Comm_buyCash`, `getMyCash`...) estão visíveis, o que facilita muito a análise.

## B. Contrato JNI confirmado
- **Framebuffer: RGB565 [C].** `MC_grpGetPixelFromRGB` empacota B>>3 | (G>>2)<<5 | (R>>3)<<11.
- **`initGame(byte[] deviceId, int len, byte[] phoneModel, int len, int width, int height, byte isKorean)` [C].** `width`/`height` vão direto para `mainScreenWidth/Height`; **a resolução vem do host**, não é fixa na `.so`. `isKorean` escolhe idioma.
- **`jniRun(byte[] fb, int)` [C]:** liga o `byte[]` como tela (`MC_grpSetScreenBuffer`, tamanho = width×height×2), executa **um quadro** (`Thread_run` → `MMain_run(dt)`, com `dt` calculado por `clock()/1000`) e devolve o **tempo de espera até o próximo quadro em ms** (padrão 50 ms ≈ 20 fps; devolve 1000 se a `.so` chamou o Java nesse quadro; devolve 0 se o jogo pediu para sair via `MC_knlExit`).
- **`jniTouch`:** o decompilador perdeu os argumentos; pelo código, o **tipo** é o 3º inteiro: 0 = pressionar, 1 = soltar, 2 = mover, 3 = tecla `0x15`. Provável ordem `(x, y, tipo)`. **[I]** confirmar lendo `GameThread.onTouchEvent`.
- `destroyGame` → `WIPI_finalize`. `setUniqueID` copia até 40 bytes para `Comm.deviceID`.
- Acesso a arquivos de save: `MC_fsOpen` usa `fopen("/data/data/game.destiniaeng/files/<nome>")` (modo `wb` para escrita). No PC é só redirecionar o prefixo. `MC_fsAvailable` fixo em ~20 MB.

## C. Anúncios e moeda de anúncio
- `setEarnedCoin(n)`: soma `n` ao "cash" do jogador (`setMyCash(getMyCash()+n)`) e salva. É o que o Tapjoy fazia. **O host simplesmente nunca chama.**
- `setGamePlaying(1)`: zera o estado de jogo e, se o valor for 1, **liga um bit no save e dá +100 de cash** (recompensa única; origem a investigar, provavelmente promoção/avaliação).
- `bannerShow()`: devolve 10/1/2/0 conforme a tela, só indicando que banner mostrar. **Ignorar.**

## D. Compras: o caminho Google Play (`T_CASH`, tipo 0x1e) é **100% local**
- `Comm_buyCash(id)` coloca o tipo de conexão em `0x1e`, marca o comando `0x20d` e chama `CommManager.buyCash(id)` no Java.
- O resultado volta por `procCommMsg(estado, erro)` → `Comm_resultCB(estado, erro)`. Com `erro == 0` e tipo `0x1e`, a **`.so` monta sozinha o pacote de sucesso** (comando `0x20e`) e chama o callback do jogo. Com erro, monta um pacote de falha com a mensagem em inglês ("The purchase has been cancelled." para `erro == -10`, "An error has occurred." para os demais).
- **Consequência para o port:** não há servidor nem verificação de assinatura do lado nativo para esse fluxo. O host pode (a) responder `procCommMsg(1, 0)` na hora (compra grátis), (b) responder com erro, ou (c) esconder o botão. A decisão de design é sua. **[C]**
- A lista de produtos (`destinia_1000cash`... `destinia_revive_800cash`) mapeia para os ids passados a `buyCash` (1 a 6 e outros). Falta mapear id → quantidade de cash.

## E. Rede (outros tipos de conexão): **ainda é um risco**
- `Comm_connectReal` na `.so` é um **stub que retorna 0**: quem abre socket é o Java (`CommManager`). O protocolo é: a `.so` escreve o pacote em `Comm+0x34`; o Java lê com `commCheck(byte[])` e envia; os bytes recebidos voltam via `procCommRead(byte[])` (anexados a `SOCKETBUFFER`); eventos de conexão voltam via `procCommMsg`.
- O jogo usa muitos comandos de rede (`Comm_init(0x1..0x20f, 0x402)`), com tipos de conexão `0`, `10` (0xa), `11` (0xb), `0x1e`. Já identifiquei **um** fluxo na tela de logo (`Logo_netProc`, comandos `0x14`/`0x15`/`0x16`): pelo nome `LogoCanvas_makeSaveData` parece ser **restauração de save pelo servidor**, e parece ser uma escolha do jogador (sim/não), não obrigatória. **[I]**
- **Pergunta crítica ainda aberta:** quais comandos de rede o jogo exige para funcionar offline (ranking, presentes, eventos, "revive")? Plano: o host implementa um **servidor falso em processo** que responde "sem dados"/"erro de conexão" e vemos o que o jogo faz.

## F. Resolução
- Não está fixa na `.so`; o host passa `width/height` em `initGame`. Evidências sugerem tela pequena (fonte `dotum08.fnt` de 8 px; imagens de ilustração de 200×120 em `title.wmb`/`dlgillu0.wmb`), mas **o tamanho de projeto original ainda não foi confirmado**. Método: testar 240×320 e 320×480 e ver qual encaixa nos elementos de UI. **[?]**

## G. Plano de host revisado (Fase 3)
1. Carregar a `.so` ARM (Unicorn), resolver as importações da libc que ela usa (`fopen`, `malloc`, `clock`, `sprintf`, `memcpy`...) com implementações do host.
2. `JNIEnv` falso com só: `FindClass`, `GetStaticMethodID/GetMethodID`, `CallStaticVoidMethod/IntMethod/ObjectMethod`, `GetByteArrayElements`, `GetArrayLength`, `ReleaseByteArrayElements`, `GetJavaVM`, `AttachCurrentThread`, `NewGlobalRef` (os deslocamentos usados: 0x18, 0x1c4, 0x234, 0x2ac, 0x2e0, 0x300, 0x36c, 0x54).
3. `initGame` → loop `jniRun`, converter o `byte[]` RGB565 em textura SDL2 (`SDL_PIXELFORMAT_RGB565`), dormir o valor retornado.
