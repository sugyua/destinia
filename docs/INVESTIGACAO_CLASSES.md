# Novas classes — viabilidade e primeiro teste

> **Destinia está em versão Beta e pode apresentar bugs. Faça backup dos saves.**

Investigação solicitada pelo autor em **21/10/2026**, para avaliar quando implementar as classes planejadas e antecipar pelo menos um teste. Esta etapa registra análise e uma sonda descartável da rotina ARM em memória; não implementa nem publica uma nova classe jogável.

## Base e método

- Fontes do port na base `57f9436261161acc7d3317476ee13709e934d2b8`.
- Biblioteca Android original `game/lib/armeabi/libdestinia_jni.so`, SHA-256 `F12F1D20AB42646EF81DF27516851ACCCB9FFFE7BE07A054DB83890D992DF7E9`.
- Inspeção de símbolos ELF, desmontagem Thumb das rotinas de habilidades, atributos, nomes, equipamento, gráficos e save; leitura estrutural dos TDT e enumeração dos assets.
- Sonda com a biblioteca original no emulador ARM já usado pelo protótipo web: três valores de classe-base em uma instância descartável, arquivos originais somente lidos e nenhum save escrito. Não foi executada uma classe-base de índice 3, nem foi alterada uma instalação ou sessão do jogador.

## O que foi confirmado

### Berserker tem base original de evolução

`gametxt_en.tdt` e sua variante de texto contêm esta sequência nas entradas 50–58:

| Classe-base | Nomes de evolução nos dados originais |
| --- | --- |
| Warrior | Blademaster, Berserker |
| Gunner | Sniper, Weapon Master |
| Thief | Assassin, Rogue |

A rotina `_drawMenuStatus` combina o índice de classe-base e um seletor de evolução para obter o nome. `GameUI_calcHeroStat` também consulta o seletor de evolução, e o fluxo original de save grava esse campo. Portanto, Berserker tem suporte original de evolução, além de um texto avulso. Ainda não foi validada nesta investigação uma campanha jogável com sua liberação, nem o fluxo completo da escolha de evolução.

Isso muda a prioridade: **o primeiro ensaio pode aproveitar o Berserker original**, usando um personagem e uma pasta de teste separados. O objetivo seria conferir seleção, nome, atributos, habilidades, combate e salvar/reabrir. Disponibilizar essa evolução antecipadamente não demonstra a inclusão de uma quarta classe-base independente nem conclui o item das 11 opções iniciais planejadas. A proposta de Berserker como opção inicial ou com novas habilidades continua exigindo projeto e validação próprios.

### A base de habilidades suporta três conjuntos fixos

- `SKILL_DATA_T`: **273 bytes**, com três blocos de **91 bytes**.
- `SKILL_DATA`: bloco ativo de **91 bytes**, usado por registros de **7 bytes**, correspondendo a **13 habilidades** por conjunto na base examinada.
- `GameUI_setSkillData`, em `0x2a270`, lê o índice em `Game+0x4886`, multiplica por 91 e copia o conjunto correspondente. A rotina não faz nessa cópia uma expansão automática ou uma checagem para uma quarta classe.
- `ATTR_UP_DATA_T`: 252 bytes; `GameUI_calcHeroStat` indexa seus blocos de 84 bytes pelo mesmo campo. `Game+0x4888` é consultado no tratamento da evolução e gravado por `GameCanvas_SaveGame`.

**Sonda executada:** para os índices 0, 1 e 2, a rotina ARM original copiou exatamente o bloco esperado de 91 bytes para `SKILL_DATA`. Os três casos passaram; a instância terminou com **zero arquivos de save escritos**. A sonda não cria uma classe nova e não valida combate, progressão, interface, compatibilidade de saves ou campanha.

Consequência: acrescentar um nome, criar `skill3_desc_en.tdt` ou gravar simplesmente o valor 3 nesse campo não acrescenta uma classe completa. A quarta escolha precisa de uma representação segura, sem indexar dados depois das tabelas originais. As opções de ampliar tabelas/rotinas ou representar arquétipos no host ainda precisam de comparação e de um protótipo próprio; nenhuma foi escolhida como implementação final.

### Há recursos úteis para reaproveitar

Foram encontrados conjuntos `hero0.agd`, `hero1.agd`, `hero2.agd`, `skill0/1/2_desc_en.tdt`, `attr0/1/2_desc_en.tdt` e os três arquivos de ícones de habilidades. As rotinas de gráficos escolhem caminhos de classe e usam nomes como `hero_bsword%d.wmb`, `hero_gun%d.wmb` e `hero_ssword%d.wmb`; isso não constitui um carregador genérico já preparado para onze classes.

Nos textos originais de habilidades há:

- Guerreiro: `Regenerative Aura`, `Combative Aura`, `Immunity Aura`, `Fire Sword` e `Meteor Strike`.
- Atirador: `Summon Machine Gun`, descrita como uma invocação de dez segundos, e `Hyper Bazooka`.
- Ladrão: `Berserk Aura`, `Hiding`, ataques múltiplos e efeitos de controle.

Esses recursos justificam investigar **Paladino** e **Engenheiro** antes de criar todos os sistemas do zero. Textos de habilidades não comprovam, isoladamente, que seus efeitos possam ser extraídos para outra classe, que as invocações já tenham a IA necessária ou que exista uma arma nova pronta.

## Ordem recomendada e janela de implementação

As janelas abaixo são metas condicionais de investigação, sem data garantida. O planejamento das versões 0.8–0.12 é preservado.

| Etapa | O que antecipar | Condição para avançar |
| --- | --- | --- |
| **Próximo experimento, durante a preparação da 0.8** | Ensaio isolado do **Berserker original**, aproveitando a evolução existente. Pode começar antes de terminar toda a 0.8. Uma eventual prévia seria experimental. | Personagem descartável, pasta de save explícita separada, campos de evolução e controles verificados; não usar o progresso principal nem anunciar uma classe-base nova concluída. |
| **Investigação junto da 0.8; após sua base de saves/perfis** | Provar uma quarta opção de classe com identidade e persistência separadas. **Paladino** é a primeira proposta de classe adicional recomendada. | Resolver os índices fixos, seleção/criação, tabelas, identidade do personagem, equipamento e carregamento; preservar os três perfis originais e os registros do Códex. |
| **Meta condicional 0.10 para uma primeira classe adicional experimental** | Integrar o Paladino inicial ao modo expandido, depois das provas e da base de inventário da 0.9. Começar com um conjunto pequeno de habilidades para testar sua identidade. | Habilidades e atributos aprovados, comportamento próprio, regiões clicáveis corretas, combate e progressão verificados, salvar/reabrir sem perdas e regressão das classes originais. Se falhar a prova da quarta opção, replanejar antes de prometer a entrega. |
| **Depois da primeira classe adicional validada** | Investigar **Engenheiro**, incluindo a base de `Summon Machine Gun`, e **Mago**, incluindo magia, ataque principal e animações. | Validar o reaproveitamento real das rotinas; armas, efeitos e regras de cada classe ainda precisam ser definidos. Versões finais a definir. |
| **Expansões seguintes** | **Granadeiro, Monge, Druida e Necromante**, além das ampliações específicas do Berserker planejado. | Granadeiro exige granadas e regras de arremesso/explosão; Monge exige validar combate e animações; Druida depende do desenho de natureza/animais; Necromante combina foice, invocações, IA e habilidades. A ordem poderá mudar conforme as provas técnicas. |

As armas já definidas pelo autor permanecem: **foice para Necromante** e **granadas para Granadeiro**. Tipos, atributos, habilidades e obtenção não foram inventados nesta investigação. As demais classes também continuam no planejamento; o ensaio do Berserker não substitui as oito opções adicionais solicitadas.

## Critérios do primeiro teste jogável

1. Inicializador de teste com pasta de save exclusiva, identificada na tela/log; a opção `--save` já existente permite escolher uma pasta e `--no-save-preferences` evita gravar preferências de teste. Não misturar personagens experimentais com o save compartilhado principal.
2. Personagem criado para o ensaio, sem conversão silenciosa de um personagem existente.
3. Identidade correta e consistente na escolha, status, habilidades e save. No Berserker original, identificar explicitamente a reutilização de uma evolução existente.
4. Mover, atacar, usar habilidades, equipar, trocar de mapa e retornar ao menu; nenhum acesso a tabelas ou animações ausentes.
5. Fechar e carregar o teste sem perder identidade/progresso ou contaminar os saves e o Códex dos personagens existentes.
6. Registrar o resultado automatizado e pedir avaliação real no Windows. Se houver uma prévia web, identificar sua persistência somente em memória e verificar toque separadamente; não supor paridade nem aprovação no Safari a partir do emulador.

**Estado ao terminar esta análise:** investigação e sonda dos três conjuntos originais concluídas; nenhum Berserker de teste foi distribuído, nenhuma quarta classe foi executada, nenhum item 44/45 foi marcado como entregue e nenhuma nova release foi publicada.
