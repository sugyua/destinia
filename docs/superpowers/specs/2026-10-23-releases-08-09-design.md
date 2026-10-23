# Destinia 0.8 e 0.9 — desenho proposto para implementação

Estado: aprovado explicitamente pelo autor em23/10/2026; implementação em andamento, sem novas releases completas ainda. O autor solicitou concluir/publicar 0.8 e depois 0.9 em GitHub e web. A publicação está autorizada; a proposta abaixo explicita as escolhas de comportamento ainda ausentes do roadmap.

## Objetivo e sequência

Concluir as melhorias funcionais de cada versão, preservando a campanha e os saves. Primeiro implementar/testar/publicar a0.8 e sua web; congelar essa revisão/tag e só então implementar/testar/publicar a0.9. Cada uma tem changelog em português com aviso Beta, instalação/dependências e pacotes Windows/source sem bibliotecas externas, dados pessoais ou jogo original. Não chamar mudança isolada de versão completa.

## Base confirmada

- GitHub main47658d8; funcionalidade de controles opcionais50aa602, CI Windows39933223943 aprovada, incluindo configurador PowerShell. Novo teste específico de ocultação executado localmente, não selecionado pelo workflow antigo.
- Web143110b,39 testes Node aprovados e Chromium móvel aprovado; deployment privado da prévia já publicado. Saves e Códex web são temporários; só preferência de controles é persistida.
- Jogo executa ELF ARM original em Unicorn. UI e cenário compartilham framebuffer; não existe uma camada pronta de UI redimensionável.
- Backups na inicialização existem. fopen/fwrite/fclose atuais ainda escrevem diretamente nos saves ativos. Isso não constitui transação de salvamento.
- Host_zoom ainda pode conceder cash em resposta ao pedido de compra e deixar callback pendente. O modo de cancelamento existente é experimental; não comprova a correção offline.
- Existem _sortItemInBag, _compareItem, _checkItemNearby e GameUI_useNItem. Inspeção estática de _compareItem mostra comparação para ordenação por categoria/ID e dados de item; não comprova o comparador visual de atributos pedido no item13.
- _checkItemNearby usa uma janela original de aproximadamente19pixels por eixo em torno do herói. Endereço/semântica ainda exigem confirmação ARM com casos de borda antes de reutilizar para coleta.

## Abordagens e decisão proposta

Seguir a abordagem incremental já escolhida para controles: extensão mínima do host e das rotinas originais, com componentes compartilhados/testes e port correspondente para web. Interceptações específicas somente após mapear/verificar contratos ARM. A alternativa de reescrever todos os menus/inventário e lógica de itens seria uma mudança estrutural maior, sem necessidade comprovada; não é a proposta. Alterar o framebuffer inteiro para encolher a UI muda o zoom e não atende ao requisito.

Nenhum endereço de memória deve ser escolhido pelo nome de uma função ou por analogia com outro APK. A especificação define comportamento, não autoriza inventar contratos nativos que faltam.

## 0.8 — comportamento proposto

### Interface (08,09,19,24)

- Controles opcionais existentes preservados. Preferências separadas por host, com restauração de padrões e validação.
- Escala independente da UI com75%,100%e125%, padrão100%do tamanho atual. Preservar cenário, câmera, proporção e tela cheia2400x1440 centralizada no monitor3440x1440. Percentuais são proposta do implementador, não escolha anterior do autor.
- Aplicar escala ao HUD/controles e às janelas que forem mapeadas, com hitboxes transformadas pelo mesmo contrato. Entrega completa do item08 exige inventário, menus, diálogo e Códex coerentes, não somente alterar tamanho da textura SDL.
- Manter XP no tamanho atual por padrão, atendendo ao pedido anterior do autor; permitir ajuste próprio caso seja necessário. Textos devem continuar legíveis em janela e tela cheia. Usar pixels/fontes compatíveis com a aparência original.
- Mostrar teclas/botões efetivamente remapeados nas habilidades, atualizando conforme dispositivo usado; não exibir atalhos inexistentes.
- Português: inventariar recursos em inglês ainda carregados; ampliar tradução sem truncar arquivos; comprovar acentos/glyphs e quebras de linha. Não declarar tradução completa por traduzir só o configurador.
- Na web, oferecer os controles equivalentes sem impedir toque no iPhone; controles externos preservados. Opções web separadas das configurações Windows.

### Gravação e compatibilidade (03,36)

- Gravar em temporários e validar erros/fechamento antes de substituir arquivos ativos; falha/interrupção deve manter a versão anterior recuperável. Confirmar fronteira da operação nativa e conjunto de arquivos antes de prometer transação multiarquivo. Backup inicial sozinho não entrega este item.
- Modo clássico continua usando a pasta atual compartilhada e configurações existentes. Expandido usa pasta separada, identificada, com backups/trava próprios; nunca migrar ou sobrescrever o save clássico silenciosamente.
- Oferecer cópia explícita de um save clássico para o expandido somente após validar compatibilidade e informar que as partidas passam a divergir. Versões futuras compartilham a pasta do modo selecionado, não criam save por versão.
- No navegador, perfis separados apenas na memória da sessão; preservar o aviso de progresso temporário. Não introduzir persistência do jogo sem escopo próprio.

### Loja offline (04)

- Pedidos de compra sem serviço real retornam ao jogo com mensagem clara. Não cobrar dinheiro real, acrescentar cash, alterar saldo anterior nem deixar spinner/rede aguardando.
- Comprovar o callback nativo correto e testar produtos conhecidos/desconhecidos, repetição, cancelamento e salvar/reabrir. Não ativar o cancelamento experimental como se já estivesse verificado.

## 0.9 — comportamento proposto

Dependência: base0.8 de gravação/perfis aprovada pelos testes. Cada assistência vem desligada; preservar jogabilidade manual. Testar sempre em cópias temporárias.

- **13 Comparador:** mostrar diferenças reais entre atributos de item selecionado/equipado, com sinais e cores acessíveis; tratar slot incompatível e efeitos desconhecidos explicitamente. Reaproveitar o que a UI original já oferece.
- **14 Organização/filtros:** ordenar por tipo, rank, nível ou preço e filtrar sem apagar/duplicar itens, perder seleção ou confundir o item alvo de equipar/vender. Preferir ordenação da apresentação quando possível.
- **15 Favoritos:** persistir proteção por personagem/modo; impedir venda e desmanche de favoritos, inclusive ações em lote. Não bloquear consumo de poções ou remoção exigida por missão. Identidade estável de itens deve ser comprovada antes de persistir; posição da mochila não é identidade.
- **16 Filtro no chão:** destacar/ocultar visualmente por categoria/rank, padrão mostrar tudo. Não remover drop, alterar chance de cair ou impedir coleta manual de um item apenas filtrado.
- **17 Coleta:** opcional, com ouro e materiais habilitados separadamente nas preferências, dentro do alcance original confirmado, sem atravessar paredes, teleporte de itens ou coleta de equipamentos. Respeitar capacidade e regras de missão; não duplicar ao repetir o evento.
- **21 Poções:** opcionais e separadas para HP/MP, limiares iniciais propostos35%/20%, configuráveis. Respeitar inventário, requisitos e recarga nativos; não gastar cash/consumíveis premium, nem agir em menus, pausa ou personagem morto. Não consumir repetidamente no mesmo quadro.
- **22 Nomes/ranks:** comando configurável para segurar/alternar exibição, nomes com cores de raridade, sem roubar teclas reservadas/remapeadas. Web precisa botão de toque equivalente.
- **29 Redistribuição:** comprovar primeiro o reset original, seus custos/limites. Disponibilizar offline sem compra real, informar custo antes de confirmar e garantir conservação de pontos. Usar custo original quando existir; se necessário criar economia nova, voltar ao autor com evidências e proposta de valor, sem inventar um custo no código.

Os valores propostos de UI e assistência são decisões novas para revisão. Novos pets/classes, nível máximo, ranks e coop não entram nestas versões.

## Arquitetura, fontes e riscos a resolver

Native: common/user_settings.h, runtime_support e componente de escrita segura; contratos de compra, tradução, render/input e inventário dos dois hosts. Scripts de configuração, inicialização, empacotamento e testes devem consumir a mesma versão/manifesto. Evitar duplicação de lógica nova entre os hosts sem reestruturação geral.

Web: engine.mjs, worker.js, app.mjs, componentes de apresentação/configuração e testes. Manter ritmo original de aproximadamente16FPS, otimizações verificadas e acesso privado. Não copiar recursos originais para GitHub público como parte do port web.

Pendências técnicas: separação dos draws de HUD/menus sem efeitos colaterais; inversão de hitboxes; caracteres acentuados no font original; limites da transação de save; resultado do callback de compra; layout/identidade de itens; rotas de venda/desmanche; alcance/coleta/cooldown; comportamento do reset original. Esses pontos exigem investigação dirigida e testes reais antes de escrita definitiva.

## Critérios de entrega por versão

- Cada requisito demonstrado por teste comportamental; distinguir estruturas ARM controladas de campanha real. Não marcar todo o roadmap como entregue se só parte foi implementada.
- Windows: compilar ambos com áudio; incluir testes novos na CI; testar configurador, entradas, câmera, texto, menus, proteção de saves e empacotamento. Validar janela/DPI/tela cheia; registrar manual pendente quando não executado.
- Web: regressões Node e Chromium com toque, retrato/paisagem, áreas clicáveis, pausa, reinício e preferências. Registrar Safari físico como pendente quando não disponível; não alegar paridade por apenas trocar o número na página.
- Confirmar conservação de inventário/ouro/cash/pontos e isolamento de personagens/modos. Testes de interrupção e falhas de disco em temporários.
- Release0.8: fontes/EXEs/checksums e changelog0.7→0.8, sem dependências externas/dados pessoais. Atualizar site e verificar deployment antes de avançar.
- Release0.9: nova revisão e artefatos próprios; changelog0.8→0.9; manter0.8 baixável e atualizar site para0.9 somente depois dos testes.
- Atualizar README e central privada com fatos, limites, evidências e próximos passos. Autorização de publicação já recebida; não pedir novamente só por tag/deploy.
