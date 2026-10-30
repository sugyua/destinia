# Destinia 0.17 — fidelidade à interface original

Pedido e abordagem: o proprietário pediu todas as interfaces/sprites adicionados com a aparência original e confirmou executar tudo. Reutilizar recursos originais em tempo de execução; não redesenhar uma imitação. A confirmação também dispensa novas rodadas de perguntas sobre essa escolha.

## Desenho

- Adaptador compartilhado PC/Web para moldura `_drawDescBox`, fonte `Font_drawStringChar` e descritores de sprites verificados. Superfície ARM temporária, restauração incondicional de framebuffer, gráficos, fonte e estado relevante; nenhuma chamada dentro de hooks ARM ativos.
- Captura/cache por tamanho, texto, cor e idioma. Composição e hitboxes usam a mesma transformação; 400×240, 800×480 e escalas75/100/125. Sem filtros que borrem pixels.
- Missões primeiro: moldura real, título/objetivo/destino reais, minimizar por toque/tecla e paridade PC/Web. Não bloquear botões originais.
- Inventário de cobertura: catálogo, baú, contratos, Códex, mago/arena/refino, inventário/proteção/lote, atributos/FC, dificuldade, avisos/HUD/diagnóstico/guia/selo, mascotes e controles externos Web. Preservar conteúdo, atalhos, regras e saves.
- Sprites devem vir de descritores/recursos com identificação comprovada. Sem associar espécie a petN por palpite. Validar as animações originais antes de trocar o mascote genérico.
- Recursos originais permanecem fora do repositório e dos pacotes públicos. A hospedagem Web existente é privada e deve continuar privada.

## Critérios de entrega

Testes reais contra o raster original, restauração de estado também em falhas, captura visual antes/depois local, textos PT-BR legíveis, toque correspondente ao desenho e regressões existentes. Builds/smokes normais e Zoom com guard68°C e saves temporários; documentação, versão, CI, pacotes/checksums, Release/download e instalação com saves preservados. Publicar Web somente com confirmação real do serviço; não tratar URL antiga como revisão nova nem emulação como teste físico no iPhone.

## Alternativa considerada

Redesenhar manualmente imitando as cores/molduras teria menos dependência do renderer, mas não atenderia à fidelidade pedida. Escolha autorizada: reutilização original. Detalhes de cache/arquivos e recortes ficam a cargo da implementação, mediante testes.
