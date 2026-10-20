DESTINIA — contexto mínimo para Claude (19/10/2026)

Tarefa atual: diagnosticar a duplicação lateral do cenário no port Windows 800×480, preservando campo de visão real ampliado. Não corrigir ainda; primeiro comparar BMPs e traces já existentes. Responder em português, objetivamente e economizando tokens.

Projeto

• Raiz: C:\destinia. Port em C++17/CMake/Ninja, MSYS2 UCRT64, executa game\lib\armeabi\libdestinia_jni.so (ARM32/Thumb) via Unicorn 2.1.4; camada JNI/Android simulada, SDL2/SDL2_mixer, framebuffer RGB565.
• Versão jogável de referência: Testar traducao PT.bat → host_zoom\build\destinia_host.exe, --save save_translation_pt --lang pt; host_zoom\main.cpp é a base 800×480. host\main.cpp (400×240) é legado, preservar.
• Versão diagnóstica mais recente: host_zoom\build_viewport_trace_map_transition_20261019\destinia_host.exe, com --trace-viewport, F9 liga/desliga captura e cria BMP; logs via --trace-out.

Bug e evidências

• Foto de gameplay: à esquerda de uma divisão vertical aparece cenário visualmente duplicado; herói/monstros nunca aparecem nessa faixa. Movimentação, paredes e passagens para outros mapas seguem funcionando; após transição, o efeito reaparece.
• host_zoom passa 800×480 à GameThread_initGame e recebe framebuffer único; SDL envia textura RGB565 inteira com pitch 1600 bytes/linha. Duplicação pode ser gerada dentro da renderização ARM, não há cópia lateral SDL identificada. A causa interna específica NÃO está provada.
• Possíveis causas a testar: câmera/origem do tilemap, repetição de tiles, clipping divergente entre cenário e entidades ou limite de mapa. Não confundir transição normal com origem do defeito.
• Símbolos ARM úteis (endereços virtuais da .so, considerar base de carga/Thumb): Map_setMapView 0x11B45; Grp_setValidRect 0x9EC1; GameCanvas_mPaint 0x1959D; Map_loadData/Map_freeData/Map_loadTile; GameCanvas_mapCheck, mapMoveCheck2. Os hooks já monitoram essas funções; não interpretar R0–R3 sem validar assinaturas.
• F9 testado durante gameplay. Primeiro trace saturava com Grp_setValidRect (até 147.247 chamadas em ~135 quadros; logs amostrados). Em uma sessão, no quadro 262, foram registradas 1 chamada cada a Map_freeData, Map_loadData, Map_loadTile; isso aponta carregamento de mapa, não prova causa da duplicação. Map_setMapView teve zero chamadas nas janelas capturadas.
• Capturas BMP anteriores não permitiram confirmar a faixa duplicada com segurança; prioridade é capturar BMP com defeito visível antes da transição, e outro depois, junto dos traces.

Arquivos dos testes

• Trace: host_zoom\trace_viewport\map_transition_trace_test_20261019\viewport_trace_1792447334253.log
• BMP: host_zoom\trace_viewport\map_transition_trace_test_20261019\viewport_frame_193_1792447349250327_0.bmp
• Segundo conjunto: host_zoom\trace_viewport\map_transition_compare_20261019\ (trace + BMPs de quadros 174 e 290).
• Save isolado: host_zoom\save_map_transition_trace_test_20261019.
• PowerShell para iniciar diagnóstico (somente se necessário):
Set-Location C:\destinia\host_zoom
.\build_viewport_trace_map_transition_20261019\destinia_host.exe --game ..\game --save save_map_transition_trace_test_20261019 --lang pt --trace-viewport --trace-frames 1200 --trace-out trace_viewport\map_transition_trace_test_20261019

Restrições e próxima ação

1. Somente leitura agora: veja 1–2 BMPs/traces citados e host_zoom\main.cpp apenas nos trechos relevantes. Informe se a duplicação está visível; se não estiver, peça captura manual via F9 na posição exata, antes de atravessar a passagem.
2. Não editar, compilar ou instrumentar novamente sem autorização; se precisar, proponha uma ação mínima com justificativa. Não resolver com escala 400×240→800×480, pois perde FOV real.
3. Nunca tocar nos saves originais, host\main.cpp, .so, assets, launcher ou executável normal. Backup antes de eventual edição; build e saves isolados. Não enviar ao GitHub sem revisar conteúdo.
4. Windows no GitHub sugyua/Destinia_PCport (público, separado do futuro port iOS). Git não inicializado em C:\destinia no último diagnóstico. Não é foco agora.

Entregável imediato: resposta curta com (a) evidência observada nos BMPs/logs, (b) hipótese prioritária, (c) apenas o próximo teste seguro. Evitar reanalisar todo o repositório e não repetir histórico.