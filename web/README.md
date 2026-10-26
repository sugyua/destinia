# Destinia 0.9.3 Beta — navegador

Teste privado: https://destinia-web-experimental.leafyheron257.chatgpt.site/

Barra corrigida: espaçamento original de23px, área extra para o décimo ícone e mão nativa por cima do menu. Códex acessível pela aba também dentro do inventário/equipamento; prévia antes de confirmar e toque na prévia ativam a consulta, sem atalho F8.

Códex integrado como décima aba na barra original, preservando as nove existentes. Usa moldura, fonte e ícones da biblioteca original; a aba adicional usa o ícone nativo de pergaminho destacado. Grade8×4, retratos frontais, descobertas, contador de mortes e detalhes por categoria (drops, locais, variantes). Toque no último ícone; selecione e toque INFO ou use AÇÃO. SAIR/Voltar retorna à grade/menu; as outras abas continuam acessíveis. Consulta congela a partida.

A web usa somente **100%**: seletor removido e preferência antiga75/125 ignorada, sem apagar outras configurações. PC mantém suas preferências independentes. Teclado, touch, gamepad padrão, ocultação automática e tela cheia preservados. Compatibilidade USB/Safari requer iPhone físico; emulação não comprova isso.

Saves/Códex web permanecem temporários nesta sessão; não substituem o save PC. Fechar/recarregar/reiniciar perde progresso. Testes usam dados controlados; não há personagem ou monstros falsos no produto.

## Fontes e dependências

HTML/JS estáticos em dist, sem compilação. Python3 para tools/pack_game.py; Node para npm test; Chromium/Playwright somente para QA. Não é necessário instalar nada para visitar o site.

Os dados originais não acompanham o espelho GitHub: forneça o APK extraído compatível e execute `python tools/pack_game.py --game /caminho/game --out dist/game`. Biblioteca revisada SHA-256 f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9. Respeite as licenças/direitos dos recursos. Emulador Unicorn.js2.1.4 e licença em dist/vendor, origem e alterações documentadas ali.

Sirva dist por HTTP(S), por exemplo `python -m http.server 8000 --directory dist`. `DESTINIA_GAME_ROOT=/caminho/game npm test` valida com dados originais locais; navegadores exigem HTTPS para Gamepad API em dispositivos remotos.

## Frente 4 — QoL candidata 0.9.9, não publicada

A seção **Itens no chão e coleta** configura destaque/categorias/rank e autoloot separado de ouro/materiais, todos desligados por padrão. Preferências em localStorage `destinia.lootQol.v1`; se armazenamento falhar, ajuste permanece apenas na sessão e a interface informa. Nenhuma preferência é gravada em save*.dat ou enviada à sincronização Drive. A assistência usa a rotina ARM original e o alcance nativo, respeita capacidade, foco/pausa/menus e não recolhe equipamentos/poções/quest. [Contrato e testes](../docs/LOOT_QOL.md). Fontes atualizadas nesta branch; nenhum deployment desta candidata foi realizado. URL privada existente preservada.
