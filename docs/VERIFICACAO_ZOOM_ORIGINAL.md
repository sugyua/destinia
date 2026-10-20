# Verificação do zoom com enquadramento original

Alteração: framebuffer e coordenadas nativas de 400×240, janela inicial de 800×480. O SDL amplia a textura com filtro nearest e preserva proporção. Nenhum arquivo do APK é modificado. O host normal permanece inalterado.

## Evidência da falha e teste da correção

O teste `tests/test_zoom_render.py` foi executado contra o binário anterior. Falhou porque a inicialização informava 800×480. Após a alteração, passou, verificando a execução de dez quadros, inicialização 400×240, cabeçalhos/tamanhos dos BMPs e pixels não totalmente pretos. Os testes do empacotador também passaram.

`tests/zoom_native_smoke.cpp` inclui o host apenas no executável de teste. Mantém o emulador aberto após uma janela SDL de teste e chama a rotina ARM original `Map_setMapView` com dimensões sintéticas de 30×30 tiles, em memória temporária, sem usar save pessoal. As posições 0, 240 e 480 produziram os limites esperados X=0/40/80 e Y=0/120/240. A janela criada foi 800×480 e a textura 400×240. No driver SDL dummy/software, clique e soltura em (400,240) chegaram ao JNI como (200,120).

O teste de fluxo PowerShell passou para o modo Camera e para a recusa do EXE antigo de 800×480, sem abrir janela guiada nesse caso. Os sete grupos de testes auxiliares de diagnóstico passaram.

## Como repetir em ambiente de desenvolvimento

Com CMake, compilador, SDL2, Unicorn e os dados originais já disponíveis, compile pelo CMake como descrito em INSTALL.md. Para o teste de captura:

```sh
DESTINIA_ZOOM_EXE=/caminho/absoluto/destinia_host python3 -m unittest discover -s tests -v
```

Sem essa variável, o teste de execução do host fica explicitamente ignorado; os testes do empacotador continuam funcionando. Os saves são temporários.

Em Linux, o teste da rotina ARM e da janela pode ser compilado e executado com:

```sh
c++ -std=c++17 -O2 tests/zoom_native_smoke.cpp $(pkg-config --cflags --libs sdl2 unicorn) -o /tmp/destinia-zoom-smoke
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_RENDER_DRIVER=software /tmp/destinia-zoom-smoke --game "$PWD/game" --save /tmp/destinia-zoom-smoke-save --mute --probe
```

Use uma pasta de save de teste nova. O fixture injeta somente clique/soltura e fechamento da janela; não valida combate, áudio, tradução completa ou travessias reais.

## Limitações

Verificação realizada em Linux, com SDL2 2.32.4 e Unicorn 2.1.4. A compilação local não incluiu SDL2_mixer: áudio não foi retestado. Ferramentas e bibliotecas usadas na verificação ficaram fora do repositório e dos pacotes.

Ainda falta a confirmação visual da passagem relatada no Windows, incluindo F9, PT, HUD e entradas durante gameplay. O mapa passou a usar o enquadramento original: não se tenta inventar cenário para completar uma área maior. Barras externas continuam possíveis em janelas com proporção diferente de 5:3; regiões pretas desenhadas pelo próprio mapa também podem existir.

Nenhuma nova tag/release é criada por esta alteração. O problema de compras/rede permanece separado.
