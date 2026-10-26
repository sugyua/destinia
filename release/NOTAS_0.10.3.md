# Notas de Lançamento — Destinia 0.10.3 Beta

> **AVISO: Versão Beta experimental. Faça backup dos seus saves antes de atualizar.**

---

## Destaques da Versão 0.10.3 Beta

### 1. Sistema Canônico de Dificuldades (Partes 1 e 2)
Implementação completa dos 5 níveis canônicos de dificuldade de campanha com escala progressiva:
* **Normal (Padrão):** Multiplicadores base 1.0x (HP, Dano, XP e Ouro).
* **Difícil:** Inimigos com 1.5x HP, 1.5x Dano, 1.3x XP e 1.3x Ouro. Desbloqueia ao derrotar o Boss Final na dificuldade Normal.
* **Pesadelo:** Inimigos com 2.5x HP, 2.5x Dano, 1.8x XP e 1.8x Ouro. Desbloqueia ao derrotar o Boss Final no Difícil.
* **Inferno:** Inimigos com 4.0x HP, 4.0x Dano, 2.5x XP e 2.5x Ouro. Desbloqueia ao derrotar o Boss Final no Pesadelo.
* **Abissal (Desafio Máximo):** Inimigos com 6.0x HP, 6.0x Dano, 4.0x XP e 4.0x Ouro.
  * **Regra Especial de Drop:** Zero bônus em drops de itens de monstros (mantém a taxa padrão de campanha como prova suprema de maestria).

### 2. Interface e Seleção de Dificuldade
* **Menu Sistema:** Novo botão **"Mudar Dificuldade"** integrado ao menu de configurações do jogo. Permite alternar apenas entre as dificuldades desbloqueadas para aquele slot.
* **Tela de Seleção de Personagem:** Exibição do indicador da dificuldade atual (ex: `Paladino [Normal]`) ao lado da classe do personagem, renderizado de forma não intrusiva com sanitização de caracteres e apenas quando há save presente no slot.

### 3. Emulação ARM de Baixo Nível e Proteção contra Exploits
* **Hooks ARM Invasão Zero:** Hooks não destrutivos no runtime Unicorn para os dois executáveis (`destinia_host.exe` e `destinia_host_zoom.exe`).
* **Prevenção de Empilhamento:** HP máximo e atual escalados preservando a proporção de dano sofrido sem acumular valores repetidos em respawns.
* **Congelamento de Tier na Morte (`deadTier`):** Ao atingir HP $\le 0$, a dificuldade do abate é congelada; trocar a dificuldade no menu durante a animação de morte não aumenta recompensas de XP/ouro nem altera o abate do Boss Final.

### 4. Persistência Transacional e Preservação de Saves
* Os dados binários originais `save*.dat` permanecem 100% intocados e compatíveis com as versões anteriores.
* O progresso de desbloqueio é mantido de forma atômica no sidecar protegido `difficulty-unlocks-v1.txt`.

---

## Validação e Conformidade
* Testes nativos executados com sucesso em ambos os hosts (resolução padrão e zoom 800x480).
* Paridade determinística comprovada em 256 seeds RNG.
* Auditoria de Qualidade e Segurança de Código homologada pelo **Sentinel** (Score 10/10).
