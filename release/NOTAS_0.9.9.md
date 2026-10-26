> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

# Destinia 0.9.9 Beta — QoL: Filtro de Drops, Autoloot, Indicador de Upgrade FC e Atalhos

- **Filtro Visual de Itens no Chão (Opt-in)**: Destaque visual colorido por categoria e rank para itens caídos no chão (equipamentos com cor do rank, materiais em ciano, ouro em amarelo e itens de missão em laranja). Ocultar um drop afeta unicamente o desenho, sem apagar o item nem bloquear a coleta manual.
- **Autoloot Seguro de Ouro e Materiais**: Coleta assistida de ouro (ativa por padrão) e materiais de fabricação confirmados (categorias 14, 15, 16, 17 e 22). Reusa a rotina ARM nativa de coleta no alcance de 19 pixels, sem colisão com NPCs ou ataques e sem duplicar itens. Equipamentos, poções e itens de missão são preservados no chão para segurança do inventário.
- **Indicador Visual de Upgrade (Bolinha Verde de FC)**: Indicador visual verde desenhado no canto superior direito do ícone de equipamentos no inventário cujo FC (Força de Combate) seja superior ao item atualmente equipado naquele slot.
- **Correção dos Atalhos do Inventário**: Pressionar `F7` ou a tecla `P` agora alterna diretamente a proteção do item selecionado (`PROT/DESPROT`), eliminando a abertura indesejada do painel de filtros. O painel FILTROS agora é acessado pela tecla `F6` ou clique direto no botão.
- **Pacote Portátil Standalone**: Executáveis recompilados acompanhados de todas as 16 DLLs de runtime MSYS2/UCRT64 (SDL2, SDL2_mixer, Unicorn, etc.) embutidas diretamente na raiz e pastas de execução, funcionando de imediato em qualquer Windows x64 sem necessidade de instalação externa.
- **Saves Nativos Intocados**: Todos os ajustes persistem via sidecar (`config.ini` no PC e `localStorage` na Web). Arquivos `save*.dat` originais 100% intocados e compatíveis com a sincronização do Google Drive.

## Downloads e dependências

Baixe o pacote `destinia-0.9.9-beta-windows-x64.zip` e extraia completo. Abra `Destinia.exe` ou `Destinia Zoom.exe` na raiz. Pacote portátil standalone com executáveis x64, launchers e todas as DLLs necessárias. Forneça apenas a pasta `game` com os dados originais. DLLs externas não são necessárias.

Web: https://destinia-web-experimental.leafyheron257.chatgpt.site/ — acesso privado, layout fixo baseado no Destinia Zoom (800x480), progresso temporário. Confira 0.9.9 no cabeçalho.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
