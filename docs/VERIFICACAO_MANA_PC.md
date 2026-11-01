# Mana excedente no PC

Uma gravação sintética com 769 de MP para máximo de 189 manteve esses valores ao ser recarregada pela engine original. Os dois hosts agora observam o personagem e limitam MP estável acima do máximo atual após 60 quadros elegíveis, sem executar uma gravação automática. A observação é reiniciada ao mudar o valor, o máximo, o nível, o estado ou quando o personagem não está vivo ou inicializado; máximo inválido não é aplicado.

O teste de política cobre transições e valores normais. O teste ARM usa arquivos temporários e confirma 769/189 para 189/189 e persistência pelo caminho nativo de salvar e carregar, nos dois hosts. Não foram usados saves pessoais. A duração de observação conta quadros elegíveis e não representa um tempo fixo em segundos.

A validação física do fluxo completo, inclusive mudanças de equipamentos e buffs temporários e integração de quadros, ainda é necessária antes de publicar ou instalar. A rotina não ressarce poções nem altera automaticamente arquivos de save; um save posterior pelo jogo pode persistir o valor ajustado.

A observação é reiniciada ao carregar ou inicializar personagem, pausar, abrir painéis ou sair da janela elegível de gameplay. O teste ARM reproduz uma recarga após 59 observações e confirma que a contagem recomeça. A detecção do HUD e o fluxo real de quadros ainda exigem teste próprio.
