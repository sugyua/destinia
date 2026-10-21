# Destinia 0.5 Beta — logs, backups e save compartilhado

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves.**

Release de fontes aprovada pelo autor em 21/10/2026. A validação manual no Windows continua pendente; a 0.4 e as anteriores permanecem disponíveis em Releases.

## Alterações desde a 0.4

- Log TXT automático por sessão em `relatorios`, com eventos observáveis, erros, desempenho resumido e rotação limitada.
- Backups automáticos verificados antes de carregar o save, com dez snapshots e proteção contra execução simultânea dos hosts novos na mesma pasta.
- Configuração global de save para normal, zoom e diferentes pastas de instalação, com `Configurar pasta de saves.bat`. Padrão anterior preservado para compartilhar com a 0.2–0.4; sem migração ou sobrescrita automática de partidas.
- Operações fread/fwrite de tamanho zero corrigidas, limites ELF validados e falhas de recursos SDL tratadas.
- Falhas emuladas e exceções passam a produzir código de saída de erro; diagnóstico continua verificando também logs e observações.
- Inicializadores e diagnóstico respeitam a configuração; testes seguem usando cópias isoladas.

## Uso e limites

Extraia os fontes completos, preserve os dados próprios em `game` e execute `Compilar Destinia.bat`. Abra um dos inicializadores para jogar; o BAT de testes usa uma cópia do progresso. Veja [instalação](../INSTALL.md) e [saves e logs](../docs/SAVES_E_LOGS.md).

O ZIP não contém EXEs Windows prontos, dados originais, saves ou dependências externas. Os requisitos continuam na seção Dependências do README. Não há expansão de nível/itens, correção da loja ou mudança de câmera/UI nesta versão.

Os testes locais automatizados não validam uma campanha completa nem substituem gameplay/UI/áudio no Windows. Consulte [verificações e aprovação](../docs/RELEASE_0.5.md) antes de publicar.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
