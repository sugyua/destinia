# Destinia 0.17.2 Beta — confirmação de gravação Web

- Após falha no carregamento inicial, envios de saves ficam desativados até um carregamento confirmado; o progresso permanece em memória e a cópia remota não recebe a sessão iniciada sem confirmação.
- Gravação online só é confirmada após HTTP de sucesso e `ok: true`; erros e confirmações negativas mantêm aviso separado do FPS. Progresso em memória é preservado.
- Carregamento de saves decodifica toda a resposta antes de atualizar os arquivos em memória. Sidecars protegidos são preservados.
- Mensagens confirmam apenas a resposta do servidor; não prometem Google Drive, durabilidade externa ou sincronização PC/Web. Servidor estáticoVPN continua sem API de saves.
- Teste de sincronização usa apenas bytes sintéticos e servidor efêmero em loopback. O comando antigo também ficou seguro para execução.
- Hosts PC preservam a correção de inicialização0.17.1; nesta revisão mudam identificação de versão e documentação, sem mudança funcional nativa.
- Campanha, Safari físico e USB não executados nesta entrega. Site privadoCodex sem novo deployment por ferramenta indisponível. Dependências e dados originais permanecem fora dos ZIPs.
