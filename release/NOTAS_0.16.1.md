# Destinia 0.16.1 Beta

## Novidades

- A versão atual aparece no canto inferior direito do menu principal nos hosts normal e Zoom.
- O número vem do arquivo `VERSION` durante o build e acompanha a próxima versão configurada.
- A auto-poção de PV usa o limite de 50% e ignora PV cheio ou PV máximo ausente.

## Verificações

- Teste de renderização: posição inferior direita em 400×240, com margem e clipping corretos.
- Coerência entre `VERSION`, README, changelog, notas e manifesto.
- Suíte de recursos Web e compilação Release dos hosts normal e Zoom.

## Limites

- A marca da versão é exibida nos hosts PC. A versão Web continua identificada como 0.13.0.
- A confirmação visual no menu principal requer teste manual no jogo.
- O pacote não inclui saves nem os dados originais do jogo.
- Defeito preexistente ainda pendente na auto-poção: o vínculo com o estoque não é confiável. O caminho PC pode consumir outro consumível ou recuperar PV sem poção; a Web não confere estoque. Esta versão corrige o limite de PV e a proteção de PV cheio, não esse contrato de consumo. Não usar os testes de threshold como prova de validação de estoque.

Revisao 0.16.1 (30/10/2026): mana valida permanece independente de PV maximo ausente; pacote de fontes inclui os quatro modulos Web importados que faltavam no manifesto. Regressoes nativa/Web e coerencia verificadas. Validacao local: Web158 PASS/1 SKIP; coerencia5 e pacotes9 PASS; builds/ARM normal+Zoom em saves temporarios PASS.
