# Códex: detalhes nos dois hosts — atualização da main 0.7 Beta

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs. Faça backup dos saves.**

## Como usar

Abra Menu → CODEX e selecione um monstro. AÇÃO/ataque abre suas informações dentro do jogo. A página 1 contém Drops, a 2 Locais de spawn e a 3 Variantes. ANTERIOR/PRÓXIMA, esquerda/direita, Page Up/Page Down ou AÇÃO percorrem categorias. SUBIR/DESCER e cima/baixo percorrem as subpáginas da categoria. VOLTAR retorna à mesma seleção da grade; F8 fecha a consulta.

O cabeçalho mantém retrato frontal completo, HP máximo e nível do último encontro da sessão, além do contador acumulado de mortes. Desconhecidos mantêm os dados ocultos. A consulta mantém a partida pausada.

## Drops e fontes dos dados

Cada item aparece em uma linha, com subpáginas próprias por categoria e raridade. Equipamentos e receitas mostram nomes base; prefixos variam na geração. Cores originais: Comum branco, Mágico verde, Raro azul, Épico roxo e Lendário laranja. Consumíveis, materiais e ovos preservam o branco original; o tier de um ovo não equivale à raridade de equipamento.

As possibilidades respeitam o nível do encontro e a variação original de +0 a +2 na geração de equipamentos. Épico pode surgir a partir do nível 38, Lendário a partir de 78. Elites e chefes usam equipamentos Raros ou superiores, sujeitos ao nível; receitas do pool normal são separadas e não incluem anéis. Itens de missão dependem de uma missão ativa compatível. Nada disso modifica as chances ou os drops reais.

O leitor consulta as tabelas originais de monstros/itens/equipamentos, os 172 mapas e os 59 blocos de scripts do APK extraído. Um monstro só recebe um mapa quando existe um posicionamento que o referencia ou um estágio verificado de chefe. Locais especiais não identificados continuam indicados como desconhecidos.

## Compatibilidade

HP, nível, variante e locais observados são dados temporários da sessão, separados por personagem. Após reiniciar, um novo encontro atualiza essas informações. Descobertas e mortes continuam persistidas no mesmo codex-v1.txt, sem conversão do auxiliar nem alteração dos saves originais.

Arquivos de metadados incompatíveis são recusados e a grade/contadores continuam disponíveis. Nenhuma dependência nova foi adicionada: utiliza a zlib já exigida pelas miniaturas.

## Distribuição e verificação

Esta atualização pertence à main; os ZIPs e tags antigos não são substituídos. Baixe os fontes atuais e execute Compilar Destinia.bat com o jogo fechado, preservando sua pasta game. O workflow verify-codex-menu compila os dois EXEs Windows e gera pacotes de teste somente após suas verificações.

Verificação local em Linux: ambos os hosts compilados com áudio; 37 testes Python aprovados, incluindo inicialização ARM, persistência do contador, metadados, paginação, controles, backups, áudio, câmera e framebuffer 800×480. Metadados e renderização também passaram em AddressSanitizer/UndefinedBehaviorSanitizer na revisão. As imagens do painel em 400×240 e 800×480 foram inspecionadas. A compilação e os roteiros PowerShell do Windows são verificados separadamente pelo workflow no GitHub Actions. Campanha completa, monitor 3440×1440 e controle físico no Windows ainda exigem verificação manual.

### Resultado Windows

[Execução 39858876580 aprovada](https://github.com/sugyua/destinia/actions/runs/39858876580), commit f50bcfbed4c38ce000d26cb07fea54809b5f23b6: dois hosts compilados com áudio, testes ARM/metadados/UI/estabilidade e sete roteiros PowerShell passaram. [Pacotes de teste Windows e fontes com SHA-256](https://github.com/sugyua/destinia/actions/runs/39858876580/artifacts/12461226530). O GitHub pode exigir login; abra o artefato e extraia o ZIP Windows interno. Dependências externas e dados originais continuam fora dos pacotes.
