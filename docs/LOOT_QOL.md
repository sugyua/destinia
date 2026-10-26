# Loot QoL — candidata 0.9.9 / Frente 4

## Comportamento e configuração

Filtro visual e materiais são opt-in; coleta automática de ouro ativa por padrão, conforme autorização posterior do proprietário. Opção explícita desativada continua respeitada. Configuração no config.ini do PC (`--settings` continua suportado) e localStorage/controles da página web. Nenhuma configuração é anexada ao formato save*.dat.

| Chave PC | Default / limites | Efeito |
| --- | --- | --- |
| drops.filtro_visual | 0 / 0–1 | Ativa marcadores e filtro apenas de desenho |
| drops.categorias | 31 / 0–31 | Bits: 1 equipamento, 2 material, 4 ouro, 8 missão, 16 outros |
| drops.rank_minimo | 0 / 0–4 | Rank mínimo somente de equipamento |
| coleta.ouro | 1 / 0–1 | Coleta automática de ouro |
| coleta.materiais | 0 / 0–1 | Coleta automática de materiais de fabricação confirmados |
| coleta.raio | 19 / 1–19 | Proximidade em pixels por eixo, dentro do limite original |

Marcadores: equipamentos usam branco/verde/azul/roxo/amarelo conforme rank original; material ciano, ouro amarelo, missão laranja, outros branco. Desabilitar categorias/ranks não remove drops nem impede coleta manual. Autoloot e filtro visual são independentes: material ocultado ainda pode ser coletado se essa assistência foi ativada. Não altera taxas de drop ou valores concedidos. Itens coletados entram normalmente no estado do jogo e participam de um save futuro pelo fluxo original; o código novo não grava diretamente save*.dat.

Web persiste em `destinia.lootQol.v1`, na janela, e envia opções ao worker. Não depende de localStorage dentro do worker. Falha de armazenamento mantém opção só na sessão com mensagem explícita. Entradas de configuração não encaminham teclas ao jogo. Pausa interrompe ticks; perda de foco interrompe assistência; menus/transições/canvas diferente não são elegíveis. Não há conversão ou nova sincronização Drive.

## Contrato verificado na biblioteca original

Biblioteca ARM Android 1.0.6, FNV-1a `74b2547839b88777`, SHA-256 `f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9`. Nativo verifica hash integral; web verifica bytes dos pontos e destinos de hooks e mantém a verificação integral já existente no worker.

- Game+0x3e76: 20 flags signed byte; >0 significa drop ativo. Registros em Game+0x3e90, stride60; coordenadas +40/+42, tipo +48, valor/ID +50.
- Ouro é tipo5. Material elegível é tipo1 cujo índice na tabela original item_en.gdt tem categoria 14/15/16/17/22 (metais/couros/tecidos/plantas/pedras de forja). Tipos/IDs não confirmados não são auto-coletados. Equipamentos, consumíveis, receitas e itens de missão ficam manuais.
- Classificação lê a tabela ORIGINAL, antes de traduzir. O tradutor existente pode mudar bytes de identificadores em ao menos uma linha não material; esta frente não altera o tradutor nem usa esse dado traduzido para decidir elegibilidade.
- `_checkItemNearby` em 0x12444 verifica flag e diferenças inclusivas de ±19 em x/y. A assistência nunca amplia esse teste. Game+0x4890/+0x4892 fornece posição do jogador; +0x4878/+0x487a câmera.
- Autoloot: um alvo elegível a cada quatro frames; round-robin para evitar que mochila cheia bloqueie ouro de outro slot. Falha mantém item e retarda nova tentativa desse slot em 32 frames. Antes de chamar, exige canvas GameCanvas, estado Game+0x4854=0 e subestado +0x4866=0, foco e assistência ativa.
- Invoca `GameCanvas_mKeyPressed(6)` fora de qualquer callback. Durante essa chamada, limita `_checkItemNearby` ao alvo somente no retorno da chamada 0x208f0. O próprio ARM aplica distância, gold/checksum, capacidade, empilhamento, remoção do drop e mensagens/efeitos. Não insere itens ou apaga flags manualmente.
- Guarda de fase: 0x208d2→0x208e2 evita seleção automática de alvo; 0x209e4→epílogo 0x2019c encerra antes de NPC/ataques. `autoTarget` sempre é restaurado após a chamada, inclusive exceções. Coleta manual fora dessa fase continua original.
- Filtro de sprite: 0x1a0ca→0x19a64, DEPOIS do processamento original de expiração. Nomes filtrados só durante drawDropItemName, com retorno separado e sem alterar chamadas externas de `_checkItemNearby`. Não escreve flags/pools/saves para ocultar; marcadores só alteram o framebuffer entregue pelo host.

## Evidências executadas

- Política C++/Node: defaults, categorias confirmadas, exclusões, rank mínimo, raio19/20, limites de preferências e marcadores com redzones/clipping.
- ARM nativo em Windows: host400×240 e host_zoom800×480, SDL dummy, biblioteca e mapa originais2/49. Ouro20000→20050 uma única vez; material79 inserido pela rotina original; poção/equipamento/quest e ouro a20pixels permanecem. Mochila cheia mantém material80. Menu/ausência de foco não altera Game. Marcadores não mudam Game; desenho nativo com/sem filtro produz pixels diferentes e Game final idêntico ao processamento nativo. Hooks de NPC/ataque/save observados: zero no autoloot. Sentinel `save0.dat` temporário preservado byte a byte.
- ARM web: os mesmos casos em400/en,800/en e800/pt, incluindo pausa de tentativas após falha, `saved` intacto e zero chamadas de NPC/ataque/save. Comparação do mPaint original prova supressão visual com Game equivalente. Suíte base: 84/84; após os ajustes aprovados de atalhos, FC e ouro/ponte de persistência, suíte final 89/89 testes Node aprovada, sem falhas.
- Preferências C++ e PowerShell: validação, persistência das chaves durante ajustes de janela, reset de QoL preservando save_dir/chaves desconhecidas, nenhuma abertura de progresso.
- Browser Maestri Portal/Chromium local: controles reais, seleção de rank/raio, persistência após reload, PT-BR inicializando e frames avançando; sem overflow horizontal em430px. Preview isolado em http://127.0.0.1:8879, portal “QoL Vanguard”. Não é deployment público.
- Baseline da branch2682c8c tinha4 falhas Node de resolução: testes400×240 usavam o novo default800×480. Fixture legada agora declara400×240; padrão do produto800×480 mantido, com novos casos cobrindo as duas bases. Não foi mascarada falha alterando a resolução do produto.

Comandos: PATH com C:\msys64\ucrt64\bin; `DESTINIA_NATIVE_TESTS=1 py -3 -m unittest discover -s tests -p test_loot_qol.py -v` (compila e executa os dois probes em saves temporários). Para Node definir DESTINIA_GAME_ROOT e executar `node --test web/tests/*.test.mjs`. Preferências: `py -3 -m unittest discover -s tests -p test_preferences.py -v` e `powershell -NoProfile -File tests/test_game_preferences.ps1`.

## Limites e próximo responsável

Não houve campanha/controle USB/Windows ou iPhone físico. Taxas/valores originais preservados, mas coleta automática muda a conveniência e deve ser opt-in; nenhuma promessa de concluir todas as situações de combate/transição foi feita. Não coleta missão, consumíveis ou equipamento. Matérias-primas desconhecidas permanecem manuais. Ajustes PC exigem fechar/reabrir o jogo; a web aplica em tempo real.

Revisão independente designada ao Sentinel, seguida por integração do Maestro. VERSION permanece0.9.8 nesta branch de implementação; allowlist do manifesto inclui as novas fontes para a integração, sem gerar ou publicar artefato0.9.9. CI do SHA integrado, versão/manifesto final, pacotes, checksums, release e deployment pertencem à orquestração. Site existente https://destinia-web-experimental.leafyheron257.chatgpt.site/ não foi atualizado por esta frente. BAT pessoal preexistente preservado; nada foi commitado na main e nenhum dado original/privado é incluído no diff.

## Ajustes de inventário aprovados em 26/10

F7/P agora alternam diretamente a proteção do equipamento apontado pelo cursor nativo, independentemente de filtros antigos. Não abrem o painel nem selecionam outro item; key-up não alterna novamente. F6 abre/fecha FILTROS; clique/X continuam. O comparador preserva clique/RS, sem anunciar F6 no rodapé.

Ícones originais capturados em `_drawEquipIcon` (x/y, equipamento), sem emulação recursiva no hook. Depois da emulação, `fcCompareEquipment(candidate,true)` compara contra os slots de backup originais do menu. Só comparação válida e FC projetado maior recebem círculo verde em x+20/y+3, com transformação da UI e clipping; painel filtrado recebe o mesmo sinal na linha. Itens incompatíveis/não identificados/nível insuficiente não recebem previsão inventada. Cache por candidato é invalidado pelo estado Game e pelos seis slots de backup; simulações restauram o snapshot. Testes dirigidos validam ganho, ausência de ganho/classe errada, desaparecimento ao equipar, cursor preservado e Game idêntico.

A proteção F7/P na web usa ponte janela/worker com ACK de gravação, resolvendo a indisponibilidade de localStorage dentro do worker. Teste real de worker no Portal confirmou persistência/ACK/releitura; detalhes em INVENTORY_PROTECTION.md.

Validação dos ajustes: 5 casos unittest de inventário (inclui probes ARM normal/zoom), 2 casos de loot (inclui probes normal/zoom) e 1 de preferências C++ aprovados; 89/89 Node. Probes verificam F7 sem abrir painel/cursor preservado, F6, cores de pixel e clipping em75/100/125, invalidar FC ao equipar, e restauração de Game. Captura BMP gerada apenas em pasta temporária de QA. Testes de persistência validam ACK, falha, concorrência e snapshots externos; teste real Window/Worker via Portal passou. Campanha física e publicação continuam fora desta etapa.
