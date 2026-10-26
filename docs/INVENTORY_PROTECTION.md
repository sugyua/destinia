# Proteção de equipamentos — candidata 0.9.8, Frente 1

## Contrato implementado

- Candidata0.9.9: F7/P alternam a proteção diretamente no item do cursor nativo, sem abrir painel; F6/clique/X abrem FILTROS. Dentro do painel, PROT/DESPROT, F7/P e ação extra alternam a marca do selecionado visível. `[P]` e nome dourado indicam proteção. VOLTAR/Esc fecha; escolher equipamento protegido para equipar continua permitido.
- Em `GameUI.state = 0x91040004`, bloqueia a escolha pelo painel e as confirmações `6/17` na entrada ARM `MainMenu_mKeyPressed`. Inclui comandos gerados internamente pelo toque. Não remove/reordena slots ou pools nativos. Item permanece visível; o badge informa PROTEGIDO ao bloquear.
- A guarda não chama a engine recursivamente: lê o alvo atual e desvia PC para LR antes da confirmação. Nenhuma escrita em Game nem em `save*.dat` foi introduzida pela funcionalidade.

## Persistência

PC: arquivo fixo `inventory-protect-v1.txt` dentro de `saveDir` já configurado. Os dois hosts compartilham marcas quando usam a mesma pasta; os modos clássico/expandido mantêm as pastas existentes. Formato versionado: cabeçalho `DESTINIA_INVENTORY_PROTECT_1`, seguido por linhas `personagem pool fingerprint_hex_40`. Limites: personagem 0–9, pool 0–199, fingerprint de exatamente 20 bytes e arquivo até 100.000 bytes. Rejeita registros inválidos, duplicados, formato desconhecido, truncamento, links/reparse e arquivos não regulares. Salva em temporário exclusivo, faz flush/sincronização e substituição atômica. A alteração em memória só é confirmada depois da gravação. Falha conserva o estado anterior; arquivo inválido é preservado e bloqueia confirmação no contexto de venda. Revalida também mudanças externas do sidecar antes de confirmar. O lock já existente da pasta de saves continua sendo usado pelo host.

Web: o mesmo formato textual é persistido em localStorage, chave `destinia:inventory-protect-v1:<modo>`. Reabertura da engine recupera marcas; personagem e modo separados. Atualização inválida, quota ou armazenamento inacessível não confirma um toggle e preserva o conteúdo anterior. Antes de vender, recarrega e valida os metadados; falta de armazenamento ou sidecar inválido bloqueia a confirmação. Não passa o sidecar pelo shim ARM de arquivos, `saved`, nem pela sincronização Google Drive. Limpar dados do site remove as marcas; esta entrega não acrescenta sincronização dessas marcas entre PC e web.

## Validação executada

- C++17/g++ UCRT64: projeção, painel e persistência; isolamento de personagem, reabertura, truncamento/duplicação/formato inválido, falha injetada de substituição atômica, preservação de sidecar inválido e sentinel `save0.dat` temporário intacto. Hitboxes em 400×240/800×480 e escalas 75/100/125.
- Sondas ARM reais compiladas em Windows para host e host_zoom: `INVENTORY_SELECTION_ARM_OK`, saída 0 nos dois. Validam identidade de gêmeos em pools distintos, toggle sem alterar Game, confirmações nativas bloqueadas com Game/GameUI idênticos, corrupção externa do sidecar preservada, recuperação e venda original após desproteger. Execução headless, SDL dummy e pastas de saves temporárias.
- Node: suíte completa de 76 testes aprovada novamente após corrigir os dois achados da revisão, sem falhas; teste adicional de hitbox aprovado na execução dirigida de 6 casos. Caso ARM web dirigido aprovado com reabertura em outra engine, falha de quota, armazenamento indisponível, sidecar externo corrompido e preservação de Game/saves. Testes unitários adicionais de hitbox e metadados.
- Revisão independente somente leitura: dois achados P2 corrigidos (PC: corrupção durante execução; web: persistência inacessível). Revisor confirmou correções, sem novos bugs concretos; não reexecutou hosts.

Comando básico PC: adicionar `C:\msys64\ucrt64\bin` ao PATH e executar `py -3 -m unittest discover -s tests -p test_inventory_view.py -v`. O caso ARM opcional desse wrapper foi pulado por flag; as duas sondas foram compiladas/executadas separadamente com g++, SDL2/Unicorn/zlib, removendo `-mwindows` dos flags de pkg-config para capturar saída/código reais.

Web: definir `DESTINIA_GAME_ROOT` para a pasta local `game` e executar `node --test web/tests/*.test.mjs`. Os dados originais são lidos localmente; não são distribuídos no commit.

## Limites e integração

Proteção cobre especificamente `0x91040004`; não promete cobertura de outras rotinas de descarte/desmanche. Fingerprint alterado invalida a marca. Destruição/recriação perfeitamente idêntica no mesmo pool entre observações é indistinguível, podendo herdar a proteção; o comportamento conservador impede venda acidental. Versões antigas ignoram sidecar/guarda. Copiar/restaurar só o save original não transporta automaticamente as marcas.

Gameplay físico Windows/iPhone/controle USB e campanha não foram validados. Não houve CI no SHA desta branch, empacotamento, release nem deployment web por esta frente. VERSION/manifesto globais e publicação da 0.9.8 ficam com o Maestro/Frente 3 após integrar todas as frentes. Web existente: https://destinia-web-experimental.leafyheron257.chatgpt.site/ (mesma audiência; não atualizada por esta branch).


### Janela/worker da web — confirmação de gravação

O worker recebe o snapshot do sidecar local da janela e solicita gravação por mensagem. A janela valida formato, modo e conteúdo anterior antes de gravar no localStorage; só o ACK de sucesso confirma a marca ativa. Durante a gravação, venda/seleção destrutiva ficam bloqueadas. Quota, alteração concorrente ou falha não confirmam o toggle. Timeout/transporte ambíguo exige recarregar e mantém venda bloqueada. Nada passa pelo shim de save ARM ou pelo Drive. Testes cobrem o protocolo e a engine real, mais um worker real via Portal com gravação/ACK/releitura, restaurando os metadados de teste ao final.
