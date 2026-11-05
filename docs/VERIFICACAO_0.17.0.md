# Destinia 0.17.0 — validação

## Resultado local verificado

- Suíte Web final: 186 aprovados,1 ignorado,nenhuma falha (187 testes,79,1s). Testes com ELF original cobrem moldura, fonte, cursor, recursos dos companheiros, abertura durante a introdução e preservação do estado.
- Testes nativos normal/Zoom: molduras comparadas às rotinas ARM originais, clipping, cache, CPU/pilha/Game/UI/MMain/gráficos restaurados inclusive após falha injetada. Companheiros distintos mantêm a animação idle original; avanço a cada6quadros e paleta preservada.
- Testes dirigidos reproduziram as falhas antes das correções: molduras próprias, falta de texto/mapeamento de toque, acesso ao menu antes de carregar os recursos e recoloração do NPC. As regressões passaram depois.
- Builds Release0.17.0 e smoke ARM de dez quadros por host em saves temporários aprovados. Monitoramento MSI Afterburner sob68°C: CPUmáxima61,625°C/GPUprincipal43°C no lote de compilação; rodada nativa completa com145amostras,CPU55°C/GPU43°C. Conferir o último log de build antes de empacotar o SHA final.
- Coerência5, fontes7 e Windows2 testes aprovados. Manifesto inclui os novos adaptadores e testes; bibliotecas/imagens originais permanecem fora dos ZIPs públicos.
- Revisão independente, incluindo as correções finais de carregamento/cache e animação, sem bloqueadores confirmados.
- Prévia Maestri/Chromium com viewport390×844 e844×390. Tela em paisagem passou a caber integralmente no viewport; catálogo abriu sem interromper a execução após a proteção da introdução. Isso não valida Safari físico.

## Recursos e compatibilidade

Janelas grandes usam o contorno cinza chanfrado do menu original; cartões/avisos menores usam a moldura original de descrição. A fonte e o cursor são originais. O Mago dos Bosses preserva a paleta dos recursos de origem.

Companheiros expandidos usam aparências cosméticas `pet3`, `pet14`, `pet21` e retrato `boss8` (Diablo). Nomes,IDs,bônus e schema dos saves foram preservados. Essas associações cosméticas não identificam os perfis expandidos como espécies canônicas.

Capturas locais: D:/IA/config/destinia-updates/visual-0170 e pet-native-reference. Nenhuma imagem ou biblioteca original foi adicionada ao repositório por esta revisão.

## Limites de entrega

CI, pacotes finais, Release0.17.0 e instalação devem ser confirmados após sua execução. O deployment desta revisão no Site privado não foi realizado: Sites get/list/credential e descoberta de plugins retornaram ferramentas indisponíveis ao modelo. A prévia é restrita a loopback neste PC; servidor na LAN foi rejeitado pela revisão automática com blocked by policy. Não há link novo acessível ao iPhone. Campanha e Safari/iPhone físicos não foram testados.

O controle de estoque da autopoção continua com os defeitos preexistentes documentados na0.16.1; esta atualização visual não os corrige.

Revisão remota final: callbacks nativos retornam fallback após falhas de captura; cargas de mascotes inválidas são limitadas por cache com recuperação ao reinicializar; arte opcional inválida não bloqueia o primeiro quadro Web; cursor Web mantém âncora original; ZIP Windows inclui este relatório. Regressões dirigidas reproduzidas e aprovadas. Suíte187testes e builds/smokes finais aprovados sob monitoramento:151amostras,CPU61,25°C/GPU44°C,limite68°C (thermal-20261030-120243).
