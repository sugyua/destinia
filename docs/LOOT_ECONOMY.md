# Sistemas de Loot e Economia — Meta 0.10 (Frente 2)

Este documento descreve os quatro pilares implementados na **Frente 2 da Meta 0.10** do Destinia para PC e Web: **Drops Exclusivos de Boss**, **Sets de Equipamento Expandidos (2 a 6 Peças)**, **Origens de Drops (Concentradas vs Distribuídas)** e **Economia de Cash (Tempo Offline e Missões Épicas)**.

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
- As contagens de derrotas de chefes e o histórico de drops são contabilizados atomicamente no sidecar `economy-v1.txt` (ou `localStorage` na Web via cabeçalho `DESTINIA_ECONOMY_2`).
- Os arquivos de save originais `save*.dat` permanecem **100% intocados**.
- Os drops exclusivos e suas descrições completas são integrados e exibidos na tela de detalhes do **Códex de Monstros**.

---

## 2. Sets de Equipamento Expandidos (2, 3, 4, 5 e 6 Peças)

O sistema de conjuntos foi expandido para suportar bônus progressivos cumulativos para **2, 3, 4, 5 e 6 peças equipadas**, abrangendo todo o inventário do herói (Armas, Elmo, Armadura, Luvas, Botas e Anel):

### 1. Set do Dragão (*Dragon Set*) — **Drop Concentrado**
- **Origem:** Boss 108 (*Diablo*) e Boss 109 (*Lava Guardian*). Todas as 6 peças caem exclusivamente destes dois chefes.
- **2 Peças (Garras Dracônicas):** +30 Ataque, +20 Força.
- **3 Peças (Escamas de Dragão):** +35 Defesa, +150 HP.
- **4 Peças (Coração Dracônico):** +300 HP, +15% Dano Crítico.
- **5 Peças (Olhar do Dragão):** +10% Chance Crítica, +35 Força.
- **6 Peças (Fúria Dracônica Suprema):** +100 Ataque, +50 Força, +25% Dano Crítico, +500 HP Máximo.

### 2. Set Sombrio (*Shadow Set*) — **Drop Distribuído**
- **Origem:** Distribuído entre Defcan (101), Defcan Desperto (103), Salas (102), Sileth (104) e Monstros Elites do mapa.
- **2 Peças (Passos Silenciosos):** +25 Evasão, +20 Destreza.
- **3 Peças (Véu Noturno):** +30 Defesa, +15 Evasão.
- **4 Peças (Golpe das Sombras):** +40 Destreza, +12% Chance Crítica, +10% Vampirismo.
- **5 Peças (Dança da Morte):** +15% Evasão, +20% Dano Crítico.
- **6 Peças (Mestre das Sombras Supremo):** +80 Evasão, +60 Destreza, +20% Vampirismo, +350 HP.

### 3. Set Imperial (*Imperial Set*) — **Drop Distribuído**
- **Origem:** Distribuído entre Kaiser (106), Baramos (107) e Masmorras de Alto Nível.
- **2 Peças (Decreto Real):** +35 Defesa, +20 Vitalidade.
- **3 Peças (Guarda do Imperador):** +40 Defesa, +200 HP.
- **4 Peças (Soberania de Kaiser):** +80 Defesa, +40 Vitalidade, +400 HP Máximo.
- **5 Peças (Aura Conquistadora):** +30 Ataque, +10% Redução de Dano Físico.
- **6 Peças (Imperador Supremo):** +120 Defesa, +60 Vitalidade, +700 HP, +20% Redução Geral de Dano.

### 4. Set Arcano (*Arcane Set*) — **Drop Distribuído**
- **Origem:** Distribuído entre Salas (102), Defcan Desperto (103), Tamur (105) e Criaturas Místicas.
- **2 Peças (Mente Iluminada):** +35 Inteligência, +150 MP Máximo.
- **3 Peças (Ressonância Mágica):** +20 Defesa, +150 HP, +100 MP.
- **4 Peças (Canalizador Etéreo):** +70 Inteligência, +300 MP, -20% Custo de MP.
- **5 Peças (Foco Prismático):** +12% Chance Crítica Mágica, +40 Inteligência.
- **6 Peças (Arquimago Supremo):** +80 Inteligência, +500 MP, -35% Custo de MP em Habilidades.

### 5. Set do Guardião (*Guardian Set*) — **Drop Concentrado**
- **Origem:** Boss 100 (*Dungeon Guardian*). Todas as 6 peças caem 100% exclusivamente da masmorra deste chefe.
- **2 Peças (Muralha de Pedra):** +35 Defesa, +200 HP.
- **3 Peças (Escudo de Granito):** +30 Defesa, +20 Vitalidade.
- **4 Peças (Corpo de Rocha):** +450 HP, +20% Resistência a Atordoamento.
- **5 Peças (Força da Terra):** +50 Defesa, +25 Regeneração de HP.
- **6 Peças (Colosso Inabalável):** +90 Defesa, +600 HP, +30% Resistência Geral, +30 Regeneração de HP.

---

## 3. Origens dos Drops (Concentrados vs Distribuídos)

Para enriquecer as rotas de farm e o endgame:
- **Sets Concentrados:** Destinados ao jogador que deseja focar em um chefe específico para completar todo o equipamento (ex: Set do Dragão em Diablo/Lava Guardian e Set do Guardião no Dungeon Guardian).
- **Sets Distribuídos:** Peças divididas entre chefes regionais e monstros elites espalhados pelo mundo, estimulando a exploração e a caça por todo o mapa.

---

## 4. Economia de Cash Calibrada e Missões Épicas

### Calibração com a Loja Original (`item_en.gdt`)
Na loja nativa do Destinia, itens premium (`f12 == 0`, como Celestial Backpacks, Baús Raros, Pedras de Forja e Pergaminhos de Reset) custam exatamente **20 Cash**. A economia foi calibrada para permitir a conquista justa destes utilitários pelo gameplay:

### 1. Ganho de Cash Offline (Teto Estrito de 24h)
- **Taxa:** 1 Cash a cada **1.800 segundos (30 minutos)** offline.
- **Teto Estrito de 24 Horas:** Máximo absoluto de **48 Cash por ciclo** (equivalente a comprar ~2 itens da loja premium por dia de ausência).
- **Proteção Anti-Fraude:** Proteção contra manipulação de relógio do sistema.

### 2. Missões Épicas de Cash (Marcos de Conquista)
Recompensas pontuais por conquistas lendárias na conta:

| ID da Missão | Título | Objetivo | Recompensa | Poder de Compra na Loja |
| :--- | :--- | :--- | :---: | :--- |
| `boss_slayer_10` | **Matador de Chefes I** | Derrotar 10 chefes de masmorras | **25 Cash** | 1 item premium (ex: Mochila) |
| `boss_slayer_25` | **Matador de Chefes II** | Derrotar 25 chefes em sua jornada | **50 Cash** | 2 itens premium |
| `boss_slayer_50` | **Conquistador de Masmorras** | Derrotar 50 chefes lendários | **75 Cash** | 3 itens premium |
| `boss_slayer_100` | **Lenda dos Chefes** | Derrotar 100 chefes supremos | **100 Cash** | 5 itens premium de reset/baús |
| `full_set_equipped` | **Conjunto Perfeito** | Equipar as 6 peças de um set lendário | **50 Cash** | 2 itens premium |

O total de Cash resgatável através de Missões Épicas é de **300 Cash**, além do fluxo diário offline.

---

## 5. Garantia de Saves, Integridade e Retrocompatibilidade

- **Saves Binários Intocados:** Nenhum arquivo `save*.dat` é tocado.
- **Codec DESTINIA_ECONOMY_2:** Armazena timestamps, cash offline concedido, cash de missões, drops e kills por chefe de forma atômica e segura.
- **Retrocompatibilidade:** O leitor aceita tanto o formato legado `DESTINIA_ECONOMY_1` quanto o atualizado `DESTINIA_ECONOMY_2`.
