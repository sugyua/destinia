# Mago dos Bosses e barra centralizada — META 0.10

Barra: dez células de 23×24, largura total 230, centralizadas; moldura original estendida simetricamente para 256px. Mesma geometria para imagens, toque e cursor em 400×240/800×480 e UI 75/100/125%. O cursor original da barra é suprimido apenas na chamada de desenho correspondente e redesenhado com a arte nativa na posição ajustada; cursores internos permanecem originais. A superfície web emprestada de 400×240 também ajusta/restaura temporariamente as dimensões gráficas, e o toque 800 usa a origem real do painel.

`common/boss_mage.h` / `web/dist/boss-mage.mjs`: decodificação dos recursos locais `mon6_0_head.wmb`, `mon6.agd` e `shadow.wmb`, reutilizando o decoder WBI/AGD existente. Paleta azul/branco/sombra é aplicada a uma cópia RGB565; transparência/silhueta originais preservadas. Nenhum asset foi alterado ou adicionado à distribuição. Preview local da paleta inspecionado.

`common/boss_mage_runtime.h`: NPC sintético e não bloqueante, fora dos pools ARM. Posições canônicas mapeadas e verificadas para todas as 5 cidades do jogo Destinia:
- **Acampamento da Resistência** (`m0_61.mpd`, Area 0, Map 61): coordenadas `(280, 232)`
- **Bergheim** (`m0_62.mpd`, Area 0, Map 62): coordenadas `(504, 280)`
- **Acampamento dos Refugiados** (`m0_63.mpd`, Area 0, Map 63): coordenadas `(280, 296)`
- **Fermion** (`m0_64.mpd`, Area 0, Map 64): coordenadas `(344, 424)`
- **Lagrant** (`m0_65.mpd`, Area 0, Map 65): coordenadas `(328, 344)`

Deslocamento por câmera nativa, clipping de raster e nome do NPC em overlay. NPCs que se sobreponham dinamicamente desabilitam a instância sintética. Não altera colisão, hitboxes, inventário, progressão ou saves.

Interação: ação nativa 6/17, alcance 16px por eixo e direção original do herói. Uma abertura por pressão, com release consumido; menus, transições, falta de foco, ação ocupada, drops próximos e autoloot não iniciam diálogo sintético. O hook só enfileira `openBossRushMenu()`; o dispatch ocorre após retornar de Unicorn, permitindo que o opener chame ARM sem reentrância. Uma troca de mapa invalida pedidos antigos.

## Contrato para OpenCodes

O desenho/pausa/menu Boss Rush pertence à UI em outra frente. Esta branch não finge ter aberto um menu quando não há consumidor:

```cpp
// Registrar uma vez após inicializar a UI do OpenCodes.
bossMage.menu.opener = [] { openBossRushMenu(); }; // função real da UI
```

```js
new DestiniaWeb(uc, assets, {onOpenBossRushMenu: () => openBossRushMenu()});
// Ou engine.bossMage.menu.opener = () => openBossRushMenu();
```

Sem opener, `menu.pending` conserva o evento enquanto elegível. A UI deve assumir pausa/input e enviar seu ACK somente após abrir; na web em worker, ligar a opção à ponte de mensagens existente da UI, não acessar DOM no worker. Sem UI importada nesta branch, testes usam consumidor explícito e verificam chamada única/diferida. Não há novo formato de save/sidecar.

## Evidência

- `DESTINIA_NATIVE_TESTS=1 py -3 tests/test_boss_mage.py`: 2/2, política/arte/geometria e ARM em ambos hosts; Game 32056 bytes intacto no caminho capturado e pintura, nenhum save ARM chamado, sentinela binária inalterada. Todas as 5 cidades validadas com sucesso.
- `DESTINIA_NATIVE_TESTS=1 py -3 -m unittest discover -s tests -p test_codex_native.py`: 1/1 com ambos hosts, estado/cursores/corpo nativo preservados.
- Node: 30/30 na suíte runtime/geometria existente; 4/4 na rodada direcionada de barra+Mago; teste adicional cobre ARM web 400/800 e toque dos 10 ícones nas 3 escalas. Testes puros cobrem paleta, parsing, direção, evento e bordas do raster em todas as 5 cidades.
