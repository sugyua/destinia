# Novos pets e novos títulos — investigação inicial

> **Destinia permanece em versão Beta e pode apresentar diversos bugs.** Este documento registra planejamento e inspeção somente de leitura; os novos conteúdos ainda não foram implementados.

Solicitação do autor em **21/10/2026**: incluir novos pets e novos títulos nas atualizações futuras e avaliar quando poderão ser implementados. Escopo: port Windows; o protótipo web experimental permanece separado. Esta alteração não publica uma nova versão do jogo.

## Base examinada e método

- Fontes do port 0.6, base de planejamento `8868ee62abdf619e147a017ce7f7da1382eb73fe`.
- Biblioteca original `game/lib/armeabi/libdestinia_jni.so`, SHA-256 `F12F1D20AB42646EF81DF27516851ACCCB9FFFE7BE07A054DB83890D992DF7E9`.
- Leitura da tabela dinâmica ELF por `readelf --dyn-syms --wide`, enumeração dos assets, leitura estrutural das tabelas de texto e descompressão em memória dos assets ZCP de pets. Nenhum arquivo original, save ou código do jogo foi modificado.
- Tabelas de texto examinadas como contagem de 16 bits seguida por entradas com comprimento de 16 bits, conferindo os limites e o consumo exato do arquivo. Nomes terminados em `_en` não garantem que todos os bytes dos demais arquivos sejam UTF-8; a inspeção não converteu nem regravou textos.

## O que foi confirmado

### Títulos

- `nick_en.tdt` contém **58 entradas de texto**, incluindo `No Title`, `Novice`, `Aspiring Warrior`, `Hero` e `God of Quests`. A contagem de textos não comprova 58 títulos adquiríveis nem o limite de IDs da lógica.
- `gametxt_en.tdt` e `gametxt_f_en.tdt` contêm `Title acquired:<C9>%s<C>` e `Please select a title.`.
- A biblioteca exporta `addGetNickMsg`, `_drawNickSel`, `NICK_CON_ACQUIRE` e `HERO_NICK_ATTR`. As duas últimas são objetos de 200 bytes cada, segundo a tabela ELF. Isso indica estruturas nativas de condições/atributos, cujo layout, quantidade de registros e limites ainda precisam ser mapeados.
- `title.wmb`, isoladamente, não comprova uma tabela de títulos de personagem; o nome também pode se referir à tela inicial. A evidência relevante vem de `nick_en.tdt`, mensagens e símbolos de apelidos/títulos.

**Implicação:** já existe um sistema para ampliar, mas adicionar nomes ou traduzir títulos existentes não cria títulos novos. É preciso entender requisitos, identificação, efeitos, seleção e persistência; não assumir que os arrays nativos crescem automaticamente.

### Pets

- Existem **22 arquivos `pet*.agd` e 21 arquivos `pet*.wmb`** numerados. Todos os 43 têm cabeçalho `ZCP\0`; o tamanho comprimido declarado corresponde aos bytes após o cabeçalho de 12 bytes, e a descompressão zlib em memória produz o tamanho descomprimido declarado.
- A contagem de arquivos não comprova o número de pets adquiríveis nem uma correspondência de um para um entre gráficos, animações e IDs; por exemplo, há `pet22.agd` sem um `pet22.wmb` correspondente nessa enumeração.
- `pet_skill_en.tdt` contém **40 entradas de texto**, incluindo nomes como `Stun`, `Intimidate`, `Crush` e `Absorb`. Isso não comprova 40 habilidades diferentes: nomes e descrições podem compartilhar a tabela.
- A biblioteca exporta `GameUI_addNewPet`, `GameCanvas_loadPet`, `GameCanvas_freePet`, `movePet`, `procPet`, `_drawPetDesc`, `_drawPetMenu` e `_drawPetAniPopup`, além dos objetos `PET_EVOL_RATIO` (16 bytes) e `PET_SKILL_DATA` (40 bytes).
- Textos originais mencionam descarte, mochila cheia e limite de evolução. Nomes de símbolos e mensagens não substituem a análise das regras nem testes de comportamento.

**Implicação:** a base de mascotes já existe. Novos pets exigem também verificar gráficos/animações, atributos, habilidades, evolução, obtenção, comportamento e save. Descomprimir os arquivos não significa já compreender seus formatos internos nem possuir um editor funcional.

## Metas propostas no roteiro

| Recurso | Meta inicial | Condições para implementar e entregar |
| --- | --- | --- |
| **40. Novos títulos** | **0.10** | Base de saves/perfis da 0.8; mapear IDs, condições, bônus e UI; provar um título novo adquirido e selecionado, com save/reabertura corretos. |
| **39. Novos pets** | **0.11** | Base de saves/perfis da 0.8 e expansão/balanceamento da 0.10; mapear IDs, dados, gráficos, animações e comportamento; provar obtenção e persistência de um mascote novo. |

Os títulos são o primeiro candidato porque há texto, condições e atributos identificados, sem a necessidade adicional de criar animações de um mascote. Ainda assim, ambos têm dificuldade alta e podem depender de alterações na lógica ARM. São metas condicionais, sem calendário garantido, e podem ser reordenadas conforme as provas de viabilidade. O mapeamento pode começar antes; a meta 0.7 continua focada em interface e acessibilidade.

Nomes, quantidades, aparências, requisitos e bônus serão definidos na etapa de implementação. O pedido atual aprova registrar as duas ideias, sem escolher silenciosamente esses detalhes.

## Critérios antes de considerar concluído

- [ ] Mapear IDs, limites, estruturas e campos de save dos dois sistemas, sem deduzir capacidade a partir da contagem de arquivos/textos.
- [ ] Preservar o modo clássico; preparar expansão em perfil separado e cópias descartáveis, com compatibilidade e versão dos dados identificadas.
- [ ] Provar um título realmente novo: desbloquear pelo objetivo, selecionar, conferir exibição e efeitos, salvar e reabrir; preservar os títulos antigos.
- [ ] Provar um pet realmente novo: obter, carregar, movimentar, conferir animação/habilidades/evolução quando aplicável, trocar/descartar, salvar e reabrir; preservar os pets antigos.
- [ ] Validar IDs inválidos, dados ausentes, texto em português, regiões clicáveis e tratamento de falhas sem perda de save.
- [ ] Balancear aquisição e eventuais bônus/atributos. Registrar testes automatizados e confirmação manual separadamente antes de propor a release.

A inspeção atual comprova a existência da base original. Ainda não comprova novos IDs funcionais, novos conteúdos persistentes, novos gráficos ou compatibilidade de expansão com o save.
