# Correção da escala do menu inicial no Zoom

Objetivo: ampliar a arte e as opções do menu inicial do host Zoom para preencher a área útil proporcional, mantendo a arte original e a câmera e gameplay atuais.

Abordagens: (1) ampliar a região central do framebuffer na apresentação, recomendada por limitar a mudança ao host; (2) alterar o desenho ARM, com maior alcance e risco. A primeira foi escolhida pelo usuário.

Aplicar a transformação apenas quando o estado nativo e a região visível do menu estiverem confirmados. Usar a mesma transformação inversa para mouse, incluindo clique e arrasto, respeitar barras laterais e posicionar a versão dentro da área visível. Não recortar submenus ou transições sem verificação. Preservar alterações locais pré-existentes e saves reais.

Verificar coordenadas e limites por testes automatizados, testar transições, teclado, mouse, tela cheia e diferentes proporções, além de inspeção visual com dados de teste isolados. Reportar limitações de gameplay manual. Inventário/status será tratado depois, em alteração separada.
