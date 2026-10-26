# Mago dos Bosses e barra centralizada — META 0.10

Barra: dez células de 23×24, largura total230, centralizadas; moldura original estendida simetricamente para256px. Mesma geometria para imagens, toque e cursor em400×240/800×480 e UI75/100/125%. O cursor original da barra é suprimido apenas na chamada de desenho correspondente e redesenhado com a arte nativa na posição ajustada; cursores internos permanecem originais. A superfície web emprestada de400×240 também ajusta/restaura temporariamente as dimensões gráficas, e o toque800 usa a origem real do painel.

`common/boss_mage.h` / `web/dist/boss-mage.mjs`: decodificação dos recursos locais `mon6_0_head.wmb`, `mon6.agd` e `shadow.wmb`, reutilizando o decoder WBI/AGD existente. Paleta azul/branco/sombra é aplicada a uma cópia RGB565; transparência/silhueta originais preservadas. Nenhum asset foi alterado ou adicionado à distribuição. Preview local da paleta inspecionado.

`common/boss_mage_runtime.h`: NPC sintético e não bloqueante, fora dos pools ARM. Posição deriva do MPD original: retângulo caminhável3×5 tiles, distância mínima64px de objetos, eventos, saídas e NPCs, sem monstros no mapa. Lagrant (`m0_65`) habilitada em `(328,344)`; deslocamento por câmera nativa, clipping de raster e nome do NPC em overlay. NPCs que se sobreponham dinamicamente desabilitam a instância sintética. Não altera colisão, hitboxes, inventário, progressão ou saves.

**Icarus:** esse nome não consta nas tabelas originais disponíveis (Resistance Camp61, Bergheim62, Refuge Camp63, Fermion64, Lagrant65). Correspondência solicitada ao proprietário, sem inventar um ID ou renomear cidade. Registro adicional disponível por `bossMage.addCity(area,map,mpd)` / `await engine.bossMage.addCity(area,map,bytes)` após confirmação. Bergheim62 também passou na política pura, mas não foi habilitada como substituta de Icarus.

Interação: ação nativa6/17, alcance16px por eixo e direção original do herói. Uma abertura por pressão, com release consumido; menus, transições, falta de foco, ação ocupada, drops próximos e autoloot não iniciam diálogo sintético. O hook só enfileira `openBossRushMenu()`; o dispatch ocorre após retornar de Unicorn, permitindo que o opener chame ARM sem reentrância. Uma troca de mapa invalida pedidos antigos.

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

- `DESTINIA_NATIVE_TESTS=1 py -3 tests/test_boss_mage.py`:2/2, política/arte/geometria e ARM em ambos hosts; Game32056bytes intacto no caminho capturado e pintura, nenhum save ARM chamado, sentinela binária inalterada.
- `DESTINIA_NATIVE_TESTS=1 py -3 -m unittest discover -s tests -p test_codex_native.py`:1/1 com ambos hosts, estado/cursores/corpo nativo preservados.
- Node:31/31 na suíte runtime/geometria existente;4/4 na rodada direcionada de barra+Mago; teste adicional cobre ARM web400/800 e toque dos10ícones nas3escalas. Testes puros novos cobrem paleta, parsing, direção, evento e bordas do raster.
- A execução geral Node obteve100/101 com único erro de ambiente na sonda C++ de progressão (DLL UCRT ausente no PATH); esse caso foi repetido com o PATH correto. Não confundir esse erro de ambiente com aprovação da primeira execução inteira.

README/CHANGELOG/allowlist atualizados; VERSION0.9.9 preservado até integração/release da orquestração. Sem deployment/release ou campanha física completos declarados nesta frente.
