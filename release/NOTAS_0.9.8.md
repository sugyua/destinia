> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

# Destinia 0.9.8 Beta — Favoritar/Proteger Equipamentos e Separação de Chefes no Códex

- **Proteção e Favoritar Equipamentos (Sidecar)**: Proteção persistente de itens por personagem/pool/fingerprint gravada em `inventory-protect-v1.txt` (PC) e `localStorage` (Web). Itens protegidos recebem o indicador visual `[P]` e o botão alternador `PROT/DESPROT` (tecla F7/atalho extra). Venda rápida ou acidental bloqueada nativamente no estado `0x91040004` com guarda ARM de confirmação. Os arquivos `save*.dat` originais permanecem 100% intocados e compatíveis com a sincronização do Google Drive.
- **Separação de Monstros Comuns vs Chefes/Bosses no Códex**: Classificação nativa da engine (`species >= 100` para Chefes e `species < 100` para Monstros Comuns). Três abas de categoria implementadas no cabeçalho do Códex: `[TODOS]`, `[COMUNS]` e `[CHEFES]`. Grade de 8x4 e paginação recalculadas por categoria, com destaque visual diferenciado para chefes e navegação completa por toque, teclado e controle.
- **Unificação da Engine Web com o Destinia Zoom**: Versão Web migrada para a base e layout do Destinia Zoom (`host_zoom`, 800x480 fixo), eliminando seletores de zoom dinâmico e unificando renderização e proporção visual entre PC e Web.
- **Testes Automatizados**: Suíte completa de testes no Node.js (testes de runtime, inventário, Códex e novos casos dedicados) e casos de teste nativos em C++ aprovados. Validação Windows física pendente.

## Downloads e dependências

Baixe o ZIP Windows x64 e extraia completo. Abra `Destinia.exe` ou `Destinia Zoom.exe` na raiz. Inclui quatro EXEs (dois inicializadores e dois hosts), fontes em ZIP separado e checksums SHA-256. Instale dependências separadamente conforme README/INSTALL e forneça seus dados originais `game`. DLLs, instaladores, APK/ativos originais e saves não acompanham os arquivos.

Web: https://destinia-web-experimental.leafyheron257.chatgpt.site/ — acesso privado, layout fixo baseado no Destinia Zoom (800x480), progresso temporário. Confira 0.9.8 no cabeçalho; recarregar perde o progresso temporário.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
