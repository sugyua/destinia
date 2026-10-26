# Notas de Lançamento — Destinia 0.10.1 Beta

> **AVISO: Versão Beta experimental. Faça backup dos seus saves antes de atualizar.**

## Destaques da Versão 0.10.1 Beta

1. **Correção de Inicialização Web em Redes Locais / HTTP (Sem Secure Context)**:
   - Resolução da falha de tela preta `undefined is not an object (evaluating 'crypto.subtle.digest')` que ocorria ao acessar o servidor web por IP de rede local (ex: `http://192.168.x.x:8000/`) em celulares ou navegadores sem HTTPS.
   - Implementado algoritmo SHA-256 nativo em JavaScript puro como fallback transparente dentro do Web Worker para validação de integridade dos binários sem depender da API `crypto.subtle`.
   - Atualizados os cabeçalhos de controle de cache (`Cache-Control: no-cache`), query strings (`?v=0.10.1`) e limpeza ativa de Service Workers obsoletos para garantir carregamento instantâneo da versão corrigida em iPhones e dispositivos Android via Wi-Fi.

2. **Novas Funcionalidades de Endgame da Linha 0.10 (Consolidadas)**:
   - Nível 999 em aritmética segura u64/BigInt com cap de 2.035.180.079.455 XP.
   - 14 Patentes de Ranks, 5 Dificuldades e multiplicadores lineares de NG+.
   - Sets de Equipamento com bônus progressivos cumulativos de 2, 3, 4, 5 e 6 peças (origens concentradas e distribuídas).
   - Mago dos Bosses (Boss Rush) em Lagrant com estilização baseada no `mon_6_0` e teleporte pago em ouro (500 a 50.000 ouro) para os 10 chefes exclusivos (`species 100..109`).
   - Título "Complecionista" ao registrar 100% dos monstros no Códex (+100% ATK, DEF, PV, PM).
   - Barra de 10 ícones centralizada na tela com hitboxes perfeitamente alinhadas nos dois modos gráficos.
   - Economia de Cash reestruturada: remoção do ganho passivo offline e liberação exclusiva via 5 Missões Épicas de marcos de chefes e set completo (10 a 25 Cash).
   - Saves nativos `save*.dat` 100% intocados.
