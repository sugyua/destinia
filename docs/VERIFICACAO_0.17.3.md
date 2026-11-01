# Verificação da candidata 0.17.3

- Escopo: apresentação do menu principal no host Zoom e transformação inversa do mouse. Câmera e gameplay não foram alterados. Outros estados e painéis sobrepostos conservam o quadro completo.
- Reproduzido no motor ARM com save temporário: estado 1 apresenta menu de 400×240 no centro do framebuffer de 800×480; Opções usa estado 4.
- Testes: geometria e mapeamento do cursor; integração ARM em SDL dummy confirmou clique em Opções e alternância do diagnóstico F12. Suíte Web: 197 aprovados, 2 ignorados, 0 falhas; release coherence 6 e pacote de fontes 7 aprovados antes da última alteração documental.
- Regressão ARM do clique prolongado: o gesto é encerrado na escala original antes de trocar a área exibida, tanto ao abrir o painel F12 quanto ao expirar o aviso H. O teste também exercita o guia do controle e a abertura de Opções.
- Compilação, CI e pacote final: conferir resultados finais da candidata antes da publicação. Sem validação física de gameplay, controles USB ou tela pelo usuário nesta rodada.
- Web funcional não alterada; VPN permanece na revisão 0.17.2 e Site privado permanece na revisão conhecida 0.13.0. Não alegar novo deployment.
- Dados de teste isolados. Instalação principal será atualizada somente após encerramento do jogo e verificação de backup e saves.
