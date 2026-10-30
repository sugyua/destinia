# Destinia 0.17.1 — verificação

- Baseline falhou no teste de uc_open: ambos hosts criavam pasta de save antes da falha. Logthermal-20261030-161048,35 amostras CPU 55,625/GPU 46.
- Após correção:15 testes runtime PASS, incluindo oito falhas por host, engine fechado exatamente uma vez quando aberto, sem initGame, sem pasta nova e sem mutação de save/journal existentes. Logthermal-20261030-161142,84 amostras CPU 61,5/GPU 45,Exit0.
- Builds Release normal/Zoom, --version0.17.1 e10quadrosARM por host em saves temporários PASS. Coerência5, fontes7, Windows2PASS. Web187testes:186PASS/1SKIP/0FAIL. Logthermal-20261030-161420,156amostras CPU63.25/GPU45,Exit0.
- Revisão independente do diff sem bloqueadores. CI/release/download/instalação terão estado final registrado na central após confirmação; não inferir publicação destes resultados locais.
- VPN no PC0.17.1:55arquivos copiados e SHA256comparado, HTTP200/banner0.17.1 confirmado. SiteCodex permanece sem nova publicação por ferramenta indisponível; audiência preservada.
- Testes controlados não equivalem a gameplay físico ou Safari real. Fingerprint da biblioteca já existia; este lote não adiciona um formato de biblioteca. Hooks de câmera/limiteZoom e hooks opcionais permanecem fora do escopo.
