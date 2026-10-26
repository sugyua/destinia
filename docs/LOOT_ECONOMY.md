# Sistemas de Loot e Economia — Meta 0.10 (Frente 2)

Este documento descreve os três pilares implementados na **Frente 2 da Meta 0.10** do Destinia para PC e Web: **Drops Exclusivos de Boss**, **Sets de Equipamento** e **Ganho de Cash Offline**.

---

## 1. Drops Exclusivos de Boss (`species >= 100`)

Cada um dos dez chefes originais do Destinia (`species` 100 a 109) possui um **drop exclusivo lendário (Rank 4)** com lore dedicada e atributos especiais:

| ID | Chefe | Item Exclusivo (PT / EN) | Atributos e Bônus Principais |
| :---: | :--- | :--- | :--- |
| **100** | **Dungeon Guardian** | Coração de Pedra Ancestral<br>*Ancient Stone Heart* | +150 HP Máximo, +20 Defesa, +15 Vitalidade |
| **101** | **Defcan** | Manto das Sombras de Defcan<br>*Defcan's Shadow Shroud* | +15 Evasão, +25 Destreza, +10 Defesa |
| **102** | **Salas** | Presa Venenosa de Salas<br>*Poison Fang of Salas* | +30 Ataque, +10 Destreza, +5% Chance Crítica |
| **103** | **Defcan Desperto** | Olho do Vazio de Defcan<br>*Defcan's Void Eye* | +150 MP Máximo, +40 Inteligência |
| **104** | **Sileth** | Lâmina Espectral de Sileth<br>*Spectral Blade of Sileth* | +35 Ataque, +15 Destreza, +10% Dano Crítico |
| **105** | **Berserk Tamur** | Núcleo de Fúria de Tamur<br>*Tamur's Rage Core* | +100 HP, +25 Ataque, +40 Força, +10% Vampirismo |
| **106** | **Kaiser** | Coroa Imperial de Kaiser<br>*Kaiser's Imperial Crown* | +200 HP, +50 MP, +20 ATK/DEF, +25 a Todos os Atributos |
| **107** | **Baramos** | Chifre Demoníaco de Baramos<br>*Baramos's Demon Horn* | +50 Ataque, +30 Defesa, +20 Força, +20 Vitalidade |
| **108** | **Diablo** | Alma Ardente de Diablo<br>*Diablo's Burning Soul* | +200 HP, +100 MP, +60 Ataque, +30 STR/INT, +5% Crítico, +10% Dano Crítico |
| **109** | **Lava Guardian** | Magma Primordial do Guardião<br>*Guardian's Primeval Magma* | +250 HP, +45 Defesa, +20 Vitalidade |

### Persistência e Códex
- As derrotas de chefes e o registro dos drops são contabilizados atomicamente no sidecar `economy-v1.txt` (ou `localStorage` na Web), mantendo os arquivos de save originais `save*.dat` 100% intocados.
- Os drops exclusivos e suas descrições completas são integrados e exibidos na tela de detalhes do **Códex de Monstros**.

---

## 2. Sets de Equipamento (Bônus Únicos de 2 e 4 Peças)

Foram implementados 5 conjuntos temáticos que concedem bônus progressivos cumulativos ao equipar 2 ou 4 peças do mesmo conjunto:

### 1. Set do Dragão (*Dragon Set*)
- **Palavras-chave:** Itens contendo `dragao` ou `dragon` no nome.
- **2 Peças (Fúria Dracônica Menor):** +25 Ataque, +15 Força.
- **4 Peças (Poder Dracônico Ancestral):** +60 Ataque, +30 Força, +10% Dano Crítico, +200 HP Máximo.

### 2. Set Sombrio (*Shadow Set*)
- **Palavras-chave:** Itens contendo `sombri`, `shadow` ou `sombra`.
- **2 Peças (Manto Noturno):** +20 Evasão, +15 Destreza.
- **4 Peças (Mestre das Sombras):** +40 Evasão, +30 Destreza, +12% Chance de Crítico, +10% Vampirismo.

### 3. Set Imperial (*Imperial Set*)
- **Palavras-chave:** Itens contendo `imperial` ou `kaiser`.
- **2 Peças (Vontade de Ferro):** +30 Defesa, +15 Vitalidade.
- **4 Peças (Soberania Absoluta):** +70 Defesa, +35 Vitalidade, +350 HP Máximo, +15% Redução de Dano.

### 4. Set Arcano (*Arcane Set*)
- **Palavras-chave:** Itens contendo `arcan`, `arcane`, `mago` ou `mage`.
- **2 Peças (Fluxo Etéreo):** +30 Inteligência, +120 MP Máximo.
- **4 Peças (Conhecimento Primordial):** +65 Inteligência, +250 MP Máximo, -20% Custo de MP em Habilidades.

### 5. Set do Guardião (*Guardian Set*)
- **Palavras-chave:** Itens contendo `guardiao` ou `guardian`.
- **2 Peças (Bastião Protetor):** +25 Defesa, +150 HP Máximo.
- **4 Peças (Fortaleza Viva):** +60 Defesa, +300 HP Máximo, +20% Resistência a Atordoamento.

---

## 3. Sistema de Cash Offline (Teto Estrito de 24h)

Para incentivar a progressão sem depender de microtransações, o sistema recompensa o tempo em que o jogo esteve fechado:

- **Mecanismo:** Abertura e encerramento gravam o timestamp UNIX (segundos desde a época) no sidecar `economy-v1.txt`.
- **Taxa de Ganho:** 1 Crédito de Cash a cada **1.800 segundos (30 minutos)** offline.
- **Teto Estrito:** O acúmulo é limitado a **no máximo 86.400 segundos (24 horas)**, gerando no máximo **48 Cash** por período inativo. Tempos maiores (ex: 48h, 7 dias, 1 mês) permanecem rigorosamente travados no teto de 48 Cash.
- **Proteção Anti-Cheat:** Relógio do sistema manipulado para trás não credita valores negativos nem inválidos.
- **Crédito no Motor Nativo:** O valor calculado é creditado diretamente via chamada nativa ARM à função exportada `setMyCash(novo_saldo)`.

---

## 4. Garantia de Saves e Assets

- **Saves `save*.dat` 100% Intocados:** Nenhum byte dos arquivos binários originais é modificado para a economia, mantendo a compatibilidade plena com o Google Drive e versões anteriores.
- **Assets 100% Originais:** Nenhuma textura ou sprite externa é exigida.
- **Compatibilidade Multiplataforma:** As implementações em C++ e JavaScript/ESM são espelhadas e validadas por suítes completas de testes automatizados.
