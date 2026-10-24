> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

# Destinia 0.9.1 Beta — Códex no menu original e filtros

## O que mudou desde a 0.9
- Códex integrado como décima aba da barra nos dois hosts Windows e na web. Usa moldura, fonte e ícones do jogo; nove abas, retratos frontais, descobertas, mortes e páginas de drops/locais/variantes preservados. INFO/ação abre detalhes; SAIR/Voltar retorna. Cache evita redesenho ARM ocioso.
- Filtros/ordenação de equipamentos por tipo, rank, nível e preço, no contexto/bolsa atual; seleção por identidade revalidada antes de voltar à interface nativa. Sem equipar/vender automaticamente. Filtros ainda usam seu próprio painel; a reformulação visual é do Códex.
- Controles: gamepad web/diagnóstico, ocultação automática do touch no jogo/site, tela cheia/fallback e desconexão/foco; no PC, L3+R3 alterna botões e hitboxes. Recursos entregues nas correções0.9 agora reunidos neste pacote.
- Web somente100%; seletor75/125 removido e preferência antiga de escala ignorada. Configurações PC, teclado/touch, saves e modos preservados.

## Download e instalação
Baixe **destinia-0.9.1-beta-windows-x64.zip** em Assets para os dois EXEs e BATs. Extraia em pasta separada, forneça os dados originais na pasta game e configure a pasta de saves existente. ZIP source é para desenvolvimento, não contém EXEs. Confira os arquivos SHA-256.

### Dependências
Windows64bits e bibliotecas externas SDL2, SDL2_mixer e Unicorn conforme README/INSTALL.md; instalação separada. Não são incluídos DLLs/instaladores/APK extraído/saves pessoais. Não substitua seu BAT pessoal com uma cópia sem guardar o original.

**[Testar no navegador/iPhone](https://destinia-web-experimental.leafyheron257.chatgpt.site/)** — mesmo acesso privado; sem instalar, sem áudio e sem progresso persistente. Recarregar perde o progresso web. USB/Safari depende do aparelho/controle/adaptador.

## Verificações e limites
Pacote Windows publicado somente após CI aprovada de ambos hosts/áudio, ARM, render, configurações PowerShell e empacotamento. Testes direcionados verificam moldura/fonte nativas, nove abas, seleção, Game/UI/contadores intactos; web validada em Chromium com toque e gamepad emulado. Essas provas não substituem testes físicos Windows/iPhone nem campanha completa.

Favoritos, proteções, comparador ampliado, coleta/poções automáticas, reset por pergaminho, novas armas/classes e otimização térmica continuam pendentes. Esta release publica o lote concluído; não marca todo o roadmap como entregue. Releases0.9 e anteriores preservadas.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
