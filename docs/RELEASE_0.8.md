> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

# Destinia 0.8 Beta — interface, saves e compatibilidade

## Mudanças desde a 0.7

- Escala independente da interface em **75%, 100% ou 125%**, sem alterar o zoom do cenário; hitboxes transformadas junto da UI e contador XP preservado.
- Controles virtuais opcionais por host, com suas áreas de toque desativadas quando ocultos. Teclado, controle e menus continuam disponíveis.
- Atalhos das habilidades exibem as teclas configuradas ou os botões/stick do controle utilizado no Windows.
- Ampliação **parcial** dos textos em português nos dois hosts e na web; fontes dos painéis do port suportam acentos. Textos sem espaço seguro mantêm o original.
- Saves ARM gravados por lote em temporários, com journal e recuperação após interrupção; mantém backups e pasta compartilhada entre atualizações.
- Loja offline cancela pedidos com aviso, libera o estado de rede e preserva o cash existente, sem conceder crédito.
- Perfis **clássico/expandido** com pastas, locks e backups separados. Importação do clássico exige destino novo e ação explícita; não altera a origem.
- Códex da main reúne a grade de retratos frontais, contador corrigido, informações e drops por categoria/raridade das correções posteriores à release original 0.7.
- Protótipo web acompanha escala, controles, idioma e modos; partidas permanecem temporárias na memória.

## Download e instalação

Baixe **destinia-0.8-beta-windows-x64.zip** para os dois EXEs e inicializadores, ou **destinia-0.8-beta-source.zip** para compilar. Confira o arquivo `.sha256` correspondente. Extraia em pasta nova; copie sua pasta `game` compatível e preserve a configuração de saves. Abra **Abrir Destinia.bat** ou **Abrir Destinia Zoom 800x480.bat**. Para compilar use **Compilar Destinia.bat** no pacote de fontes, com o jogo fechado.

Ferramentas, DLLs externas e dados do APK não acompanham os pacotes. Consulte **Dependências** no README e INSTALL.md. As releases antigas continuam disponíveis.

## Configuração

Em **Configurar Destinia.bat**: 6 controles virtuais; 7 modo clássico/expandido; 8 escala da UI; 9 idioma. O padrão mantém o modo clássico, interface100% e controles visíveis. Para importar uma cópia do clássico use a opção de importação do configurador ou execute o EXE somente com `--import-classic`, com destino expandido ainda inexistente. Não execute os dois hosts simultaneamente na mesma pasta de save.

## Verificações e limites

Regressões automatizadas cobrem saves/recuperação, loja com callbacks reais ARM, isolamento/importação, preferências, Códex, pixels/alpha/clipping/toque nas escalas75/100/125 e fontes/atalhos. A publicação Windows exige compilação com áudio, testes nativos e scripts PowerShell pelo workflow desta release. Na web, testes Node e Chromium com toque verificam inicialização, menus, pausa, rotação e preferências.

Isso não comprova uma campanha completa nem testes físicos no Windows/iPhone. A tradução não é integral; a fonte do jogo Android conserva suas limitações. Recuperação de save não garante atomicidade entre múltiplas chamadas ARM nem proteção absoluta contra falha física/falta de energia. Expandido ainda usa conteúdo e limites originais; novos níveis, itens e classes não estão incluídos. Web continua sem áudio e sem persistência do progresso; não há garantia de16FPS em todos os aparelhos.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
