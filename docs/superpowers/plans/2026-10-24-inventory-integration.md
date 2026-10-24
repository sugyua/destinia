# Integração visual do inventário — item14

Execução direta autorizada pelo autor; sem agentes. Base/spec aprovados: releases-08-09-design e contratos em docs/INVENTARIO_0.9.1.md. Preservar saves, BAT local, modo clássico e releases0.9.

1. Testes RED/GREEN: acrescentar faixa de preço à projeção; painel puro com filtros tipo/rank/nível/preço, ordem/direção, lista vazia e seleção por pool. Desenho/hit test em75/100/125.
2. Adaptador ARM em ambos hosts: coletar somente equipamentos ativos do contexto nativo (equipamento, mochila ou venda/bolsa atual), guardar20bytes de identidade e índice original. Abrir por botão interno/F7 livre/X. Não reordenar Game nem mudar índices para projetar. Usar navegação nativa5/7 para retornar ao item, verificar estado/pool/prévia; nunca executar equipar/vender automaticamente.
3. Painel retorna ao menu original com item correto destacado; confirmações e ações continuam nativas. Fechar/cancelar não altera seleção nativa. Reabrir reconstrói registros, ignora quantidade zero e revalida pool/fingerprint; contextos/ranks desconhecidos não viram alvos.
4. Portar a mesma apresentação/contrato à web, sem painel de filtros no site. Teclado/controle/toque, foco e desconexão liberam comandos. Congelar somente JNI enquanto painel aberto, como Códex; não gravar preferências novas.
5. Testes direcionados modelo/UI/ARM nos dois hosts e Node/browser; incluir arquivos no manifesto/CI. Validação integrada Windows antes de publicar. Registrar marco e marcador6389152394; não declarar0.9.1 completa.

Limites: equipamentos apenas; consumíveis/missões permanecem na interface nativa. Expor escopo no README. Sem favoritos, coleta, poções/reset ou mudanças de combate neste lote. Perfil térmico enviado aponta emulação64–68ms/render~1ms/espera0; otimização permanece tarefa separada.
