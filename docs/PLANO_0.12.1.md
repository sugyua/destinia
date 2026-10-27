# Correções da Issue #5 — 0.12.1 Beta

Pedido aprovado: coerência de versão/pacote e atalhos Web. Correções locais sobre a arquitetura atual; sem reescrever hosts ou ampliar gameplay. Alternativa de somente trocar rótulos não resolve atalhos; refatoração ampla não é necessária.

1. Regressão do pacote sem notas atuais; selecionar notas por VERSION e falhar se ausentes.
2. Mapa de teclado compartilhado pelos rótulos e listener, preservando comandos, Tab, keyup e pausa; regressões Node e navegador.
3. Alinhar README, changelog, VERSION, manifesto, banner e estado do roadmap; manter histórico identificado.
4. Compilar dois hosts, testar boot em diretórios descartáveis e conferir pacotes curados/checksums. Excluir DLLs/dados originais/saves/BAT pessoal.
5. Publicar somente artefatos validados, conferir downloads e atualizar central; deployment Web condicionado ao acesso real sem mudar audiência.

Isolamento: branch fix/0121-issue5, base c0c1566; main e arquivos locais preservados. Revisão própria identificada como tal. Fuzz, CI Web, hooks/saves e demais itens ficam fora deste lote.
