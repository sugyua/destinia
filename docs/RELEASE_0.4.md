# Release 0.4 Beta — plano e verificações

Escopo solicitado: compilação dos dois hosts por BAT e um launcher para cada host; após complemento do usuário, incluir zoom out, centralização com fundo cinza e UI menor. O log TXT detalhado fica para a 0.5.

## Implementação

- host_zoom usa 800×480; host normal permanece 400×240. Tela cheia e F11 preservados.
- Centralização em memória somente nos eixos em que o mapa é menor que o viewport. Um hook antes das leituras ARM cobre também escritores inline; mapas maiores preservam a câmera nativa.
- Hook de borda preenche margens fora do mapa em cinza RGB565 e preserva pixels internos. Filtro de tiles externos preservado. Barras externas SDL também cinzas.
- A UI original usa a escala nativa no framebuffer maior, ficando proporcionalmente menor. O texto de XP usa 14 linhas de pixels no zoom, equivalente às 7 linhas da 0.3 após considerar a ampliação de apresentação (3× e 6× no monitor 1440p). Não é uma reconstrução da UI nem um controle de escala configurável.
- BAT compila ambos por padrão, com opção de argumentos para um host. Launchers normal e zoom usam a mesma raiz MSYS2 configurável e orientam executar o compilador se faltar EXE.
- Diagnóstico atualizado para 800×480 no zoom, identidade do novo binário e pergunta manual sobre UI/XP. Saves continuam isolados nos testes; launchers normais compartilham o save existente, como antes.

## Verificações

Compilação CMake e testes em Linux com SDL2 e Unicorn. Sem SDL2_mixer nesta preparação; áudio não retestado. O fixture ARM usa mapas sintéticos em memória: 30×30, 60×60 e 20×10 tiles, leitura real ARM da câmera após escrita inline simulada, preenchimento cinza, preservação de área interna e texto XP. Não equivale a teste completo da passagem real do usuário.

Primeira execução do teste de cinza falhou porque o fixture trocava dimensões sem recalcular a câmera do novo mapa, mantendo a coordenada vertical do mapa anterior. O fixture passou a executar a rotina original de câmera, como nos demais casos; a implementação de preenchimento não foi alterada por essa falha.

Resultado final: ambos os hosts compilados; 12 testes Python aprovados, sete grupos PowerShell aprovados e teste de fluxo Camera/recusa de EXE antigo aprovado. Fixture ARM/SDL aprovado para centralizacao, fundo cinza, preservacao dos pixels do mapa, F11, geometria ultrawide simulada e altura do texto XP. BATs completos, gameplay, hitboxes da UI, PT e F9 ainda precisam de confirmação no Windows. F11 e geometria ultrawide são exercitados via SDL dummy com dimensões de saída simuladas. Compras/rede permanecem pendentes.

## Distribuição e aprovação

ZIP curado de fontes e SHA-256, sem APK extraído, saves, logs pessoais, EXE, DLLs ou instaladores/dependências. Recompilação necessária. A release 0.3 permanece disponível. Rascunho e pacote revisados; publicação da tag/release 0.4 aprovada pelo usuário em 21/10/2026. O jogo permanece Beta, com confirmação visual no Windows pendente.
