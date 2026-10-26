# 0.10.3 — primeira parte: dificuldade

Escopo autorizado: bot?o no Sistema, cinco op??es, desbloqueio sequencial ao matar Diablo (108), indicador junto da classe apenas em saves existentes. Reutilizar fonte/moldura nativas; preservar saves e controles existentes. Esta etapa entrega commit testado, n?o a publica??o da vers?o completa.

1. Testar primeiro regras de desbloqueio, isolamento por slot, persist?ncia e falha de escrita.
2. Usar difficulty de progression-v1.txt; registrar teto liberado em difficulty-unlocks-v1.txt. Gravar os dois com SaveTransaction existente; preservar XP/NG+ e recusar dados inv?lidos. N?o inferir abates antigos do C?dex, que n?o registra dificuldade.
3. Acrescentar sexta op??o do Sistema sem substituir as cinco originais. Navegar com teclas/controle/toque, cinza nas bloqueadas. Indicador na chamada nativa que desenha a classe na sele??o, condicionado — presen?a do save.
4. Validar ambos hosts com ARM original e saves tempor?rios, regress?es e builds Windows. Atualizar documenta??o e central; commit apenas dos arquivos desta etapa. Gameplay f?sico e publica??o web/release ser?o relatados separadamente.

A primeira parte — de UI e persist?ncia: n?o altera balanceamento, multiplicadores de combate, campanha ou NG+.
