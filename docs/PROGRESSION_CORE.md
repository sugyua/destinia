# Núcleo canônico de progressão — Frente 1 / META 0.10

Contrato para integração futura. **Não habilita NV 999 no jogo por si só.** Nenhum host, hook ARM, HUD, combate, campanha, save original ou ponte de worker foi modificado nesta frente.

## API compartilhada

`common/progression.h`, namespace `destinia::progression`; equivalente ESM em `web/dist/progression.mjs`. C++17, sem dependência de SDL/Unicorn/ativos para o núcleo. XP usa `uint64_t` no C++ e **somente `BigInt`** no JS; nível/status/multiplicadores usam `uint32_t`/Number inteiro validado. Falhas são exceções antes de retornar um resultado ou persistir dados.

| Consulta pura | Contrato |
|---|---|
| `xpForNextLevel(level)` | `(n + 2 + floor(n²/10))² + 20`, extensão exata da fórmula usada pelos hosts; retorna zero em 999 |
| `xpToReachLevel(level)` | Soma dos custos anteriores, começa em zero no NV1; NV999 = 2035180079455 |
| `query(totalXp)` | `{level, rank, totalXp, currentXp, requiredXp}`; rejeita XP acima do teto |
| `awardXp(totalXp, amount)` | `{progress, accepted, discarded}`; limita ganho ao teto sem somar valores que possam transbordar |
| `rankForLevel(level)` / `rankName(rank)` | Enum estável 0..13 e nome PT-BR; rejeita nível fora de 1..999 |
| `multipliers(difficulty, ngCycle, rules)` | HP, attack, XP e gold em pontos-base (10000 = 1x) |
| `scale(value, basisPoints)` | `floor(value*basisPoints/10000)`; rejeita resultado maior que u64, mesmo quando o produto intermediário é maior que u64 |
| `statAtLevel(level, curve)` | `base + linear*(level−1) + floor(quadratic*(level−1)²/divisor)`; coeficientes u32, divisor positivo, resultado u32 verificado |

Curvas de status são **parametrizadas**, não substituem a fórmula nativa. Não foram inventados pontos por classe nem aumentados atributos em memória. Os consumidores devem definir os coeficientes apropriados e aplicar `scale` uma única vez sobre o valor base, sem realimentar um valor já escalado a cada frame.

| Nível | Rank / ID |
|---|---|
| 1–20 | Aprendiz / 0 |
| 21–40 | Aventureiro / 1 |
| 41–60 | Guerreiro / 2 |
| 61–80 | Cavaleiro / 3 |
| 81–99 | Campeão / 4 |
| 100–199 | Mestre / 5 |
| 200–299 | Grão-Mestre / 6 |
| 300–399 | Lenda / 7 |
| 400–499 | Mítico / 8 |
| 500–599 | Semideus / 9 |
| 600–699 | Divino / 10 |
| 700–799 | Transcendente / 11 |
| 800–899 | Celestial / 12 |
| 900–999 | Supremo / 13 |

| Dificuldade / ID | HP e ATK | XP e ouro |
|---|---:|---:|
| Normal / 0 | 1,0x | 1,0x |
| Difícil / 1 | 1,5x | 1,3x |
| Pesadelo / 2 | 2,5x | 1,8x |
| Inferno / 3 | 4,0x | 2,5x |
| Abissal / 4 | 6,0x | 4,0x |

Padrão inicial de NG+ explicitado à orquestração: crescimento **linear por ciclo** de +25% HP/ATK e +10% XP/ouro; configurável em `Rules.ngGrowth` / `defaultRules().ngGrowth`, persistido no sidecar. O multiplicador final é `floor(dificuldade * (10000 + ciclo*crescimento) / 10000)`. Normal/NG+0 permanece exatamente 1x; Normal 1x é validado mesmo em regras customizadas. Não há reset, unlock ou avanço automático de campanha. Ciclos são u32; combinações cujo multiplicador exceda u32 são rejeitadas, não truncadas. O balanceamento de endgame exige validação posterior em gameplay.

## Fronteira ARM: consultas e conversões, sem escrita

Mapeamento confirmado no código existente e inspeção estática do ELF original local (não distribuído):

| Campo relativo a `Game` | Representação / cuidado |
|---|---|
| `+0x48FA` | nível u16; 999 cabe fisicamente, mas **não** é compatível com o limite lógico/tabelas originais |
| `+0x48FC` | XP corrente u32; não é o XP acumulado do sidecar |
| `+0x48BA + 2*i` | 13 stats lidos como i16 pelo comparador FC; limite seguro positivo 32767 |

`GameCanvas_gainExpProc` (Thumb `0x17adc`, 620 bytes) compara com `#0x63` em `0x17bc6`; `GameCanvas_getGainExp` (`0x123f0`) calcula com multiplicações nativas estreitas; `GameUI_calcHeroStat` (`0x24fb0`) usa `strh`. Estes endereços são evidência da biblioteca analisada, **não autorização para instalar hooks sem verificar versão/assinatura**.

`checkedArmLevel` recusa níveis acima de 99; `nativeLevelCompatible` informa compatibilidade lógica. `checkedArmU16` apenas verifica largura 0..65535; não autoriza usar NV999 no motor original. `checkedArmStat` recusa >32767; `checkedArmXp` recusa >4294967295. Nunca usar cast/máscara/clamp silencioso para injetar progressão estendida. Não há gravação em pools ou memória ARM neste módulo.

`importNative(level,currentXp)` é uma conversão explícita de snapshot já fornecido pelo chamador, não abre saves. Aceita níveis 1..99, XP corrente menor que o custo do nível; em 99 exige zero (convenção do HUD original). Sua soma produz uma posição na curva, não uma alegação de XP histórico efetivamente ganho.

## Sidecar progression-v1

Nome fixo `progression-v1.txt`, apenas números decimais ASCII e cabeçalho `DESTINIA_PROGRESSION_V1`. Inclui identificador `legacy-extended-v1`, modo `classic`/`expanded`, regras completas e até dez personagens por slot 0..9, com XP acumulado, ciclo e dificuldade. `encode`/`decode` têm paridade de bytes entre C++ e JS. XP serializado em decimal sem passar por Number/JSON numérico. Parser limitado a 4096 bytes, rejeita versão/curva desconhecida, duplicatas, modo divergente, overflow, lixo e truncamento.

`common/progression_store.h` é a camada opcional de disco. O chamador fornece o **diretório do modo já resolvido** (`modeSaveDir` no host), nunca um nome de save. Arquivo/lock fixos; rejeita links/reparse points no diretório e nos arquivos operados, arquivo não regular e conteúdo anterior inválido. `save(directory, ledger, ops, expected)` usa lock exclusivo, verifica os bytes anteriores, grava temporário, flush/_commit e substituição atômica pelo `AtomicSaveOps` existente. Falha preserva o sidecar anterior e remove temporário. `expected=nullopt` permite somente criação; para atualizar, passar o texto exato de `readText(directory)` e trabalhar sobre o ledger decodificado desse snapshot. Não gerar `expected` a partir de uma reserialização de arquivo externo.

```cpp
auto previous = destinia::progression::readText(modeDirectory);
auto ledger = previous ? destinia::progression::decode(*previous, mode)
                       : destinia::progression::Ledger{};
ledger.mode = mode;
// Alterar apenas o ledger em memória; nunca campos ARM aqui.
destinia::progression::save(modeDirectory, ledger, destinia::AtomicSaveOps{}, previous);
```

Dois consumidores com snapshot antigo não podem substituir silenciosamente um ao outro. Corrupção/conflito/busy devem aparecer como erro recuperável na integração, sem apagar dados nem reconhecer progresso como persistido. Não há transação combinada com saves originais: a integração futura precisa decidir checkpoint e reconciliação explicitamente. Recriação/exclusão de personagem exige limpar explicitamente seu registro; **slot não é identidade permanente de personagem**. Não carregar progresso por slot automaticamente sem esse tratamento. Mover um modo/save para outro diretório não importa o sidecar implicitamente.

Na web `createSidecarStore({storage,mode,locks})` recebe Storage da janela e Web Locks (`navigator.locks`). Usa chave `destinia.progression-v1.<modo>`, `load()` retorna `{text,ledger}` ou null e `commit(ledger,expectedText=null)` retorna Promise com o novo texto após lock/CAS/gravação/releitura. Propaga quota/Storage indisponível e recusa escrita sem Web Locks; **não há fallback inseguro**. Exportar `encode(ledger)` permite download do mesmo sidecar textual. Worker não deve acessar localStorage diretamente: integração futura deve usar ponte/ACK da janela e só confirmar progresso após resolução da Promise. Alterações por ferramentas externas que ignorem o lock não são uma operação suportada.

## Verificação e limites da entrega

`PATH` deve incluir `C:\msys64\ucrt64\bin` no Windows:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
py -3 tests/test_progression.py
node --test web/tests/*.test.mjs
```

Runner compila C++17, executa casos nativos e habilita `PROGRESSION_CPP` para comparar todos os 999 níveis, ranks, curvas, dificuldades/ciclos, escala até u64 máximo e serialização. Testes incluem falha de substituição, snapshot obsoleto, lock, corruptos, truncamentos, quota/conflito web e save sintético com NUL preservado byte a byte. Não usam saves pessoais nem dados originais. Não comprovam campanha, balanceamento, HUD, NV999 em jogo, instalação de hooks ou persistência através do worker real, que estão fora desta frente.

Nenhuma tag/release ou publicação web da 0.10 foi feita aqui. `VERSION` permanece na release 0.9.9; manifesto apenas inclui as novas fontes. Integração/revisão/publicação pertencem à orquestração.

Checkpoint automatizado local de 26/10/2026: runner C++/Node de progressão aprovado (6 casos Node, sem skips, incluindo paridade); suíte Node completa 94 aprovados, 1 skip da paridade por variável `PROGRESSION_CPP` ausente nessa execução geral, coberta pelo runner dedicado; 7 testes do empacotador aprovados e pacote temporário de fontes com 232 entradas validado, sem saves ou sidecar de usuário. A primeira execução geral Node falhou por ausência da variável `DESTINIA_GAME_ROOT` nos testes legados; repetida com o caminho local correto e aprovada. Compilação C++ com `-Wall -Wextra` mostra somente avisos preexistentes de indentação em `common/safe_save.h:171–172`, arquivo não modificado.
