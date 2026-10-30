# Destinia 0.17.2 — verificação

- TDD: nove casos falharam no baseline0.17.1. Após a correção, dez casos passam: HTTP500 com corpo ok, confirmação negativa, JSON/base64 inválido, carregamento atômico, sidecar protegido, callback falho e aviso persistente. O comando legado seguro executou os mesmos dez casos com sucesso.
- Nenhum teste leu ou escreveu saves pessoais. O servidor de teste usa porta efêmera em127.0.0.1 e bytes sintéticos em memória; não depende de GoogleDrive, da API real ou de biblioteca ARM para esses dez casos.
- Suíte Web:197 testes,196 aprovados,1 ignorado,nenhuma falha. Coerência6, fontes7 e Windows2 aprovados. BuildsRelease, --version0.17.2 e dez quadrosARM normal/Zoom em saves temporários aprovados. Guard térmico thermal-20261030-170621:159amostras,CPU62,625°C/GPU45°C,Exit0,limite68°C.
- A primeira suíte completa expôs uma fixture antiga que chamava o método em objeto parcial. O teste passou a usar uma engine real, preservando as asserções de proteção e cleanup; nova suíte completa aprovada.
- Chromium/Maestri em390×844 e844×390: primeiro frameARM alcançado; avisoAPI404 permaneceu visível enquantoFPS atualizava. Canvas e rodapé cabem no viewport; revisão final pausada. Não equivale a Safari físico ou teste de leitor de tela.
- VPN0.17.2:HTTP200/banner/status conferidos e56arquivos estáticos comparados porSHA256. Site privadoCodex sem novo deployment por ferramenta indisponível, audiência preservada.
- Revisões independentes sem bloqueadores. CI/release/download/instalaçãoPC ainda pendentes nesta candidata; resultado final será registrado na central após confirmação.
- Contrato preservado:/api/save com JSON/base64. A confirmação refere apenas a resposta do servidor, sem garantir GoogleDrive, durabilidade externa ou sincronizaçãoPC/Web. Servidor estáticoVPN continua sem API de saves. Ordenação de gravações concorrentes e timeouts de rede são limites anteriores fora deste lote.
