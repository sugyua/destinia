# Desenho proposto — 0.13.0 Beta

Status: inspeção inicial concluída; desenho aguardando aprovação. Nenhuma implementação nova, build ou publicação concluída neste checkpoint.

## Objetivo e precedência

Implementar o pedido de 26/10/2026: Survival Refinement & Endless Arena Gauntlet no Windows e na Web. O pedido atual prevalece sobre a nota histórica de refino de equipamentos. Preservar os protótipos locais existentes sem confundi-los com funcionalidades entregues.

## Situação verificada

- Branch atual: `feat/0130-vanguard-survival-refinement`; VERSION contém `0.12.0`.
- Hosts C++17 com SDL2, Unicorn e zlib; SDL2_mixer opcional; build CMake/Ninja. Compilador UCRT64 disponível em `C:/msys64/ucrt64/bin/g++.exe`.
- `common/boss_rush_ui.h` já possui cinco abas, incluindo Santuário e Títulos; ambos os hosts o utilizam.
- Protótipos locais não rastreados: `common/survival_arena.h`, `common/refinement.h`, módulos Web e testes. Há também alterações preexistentes em app/engine/worker Web. Não descartar nem sobrescrever esse trabalho sem análise.
- O protótipo de arena termina na onda 15 e concede título ao final. Seu ledger não contém os quatro recordes solicitados. Não há integração nativa identificada por busca de survival/refinement nos hosts.
- Baseline executada: `node --test web/tests/features-0130.test.mjs`: 6 testes, 5 aprovados e 1 falha no caso de identidade do refino (`10 !== 0`, linha 42). Não atribuída à arena nem corrigida sem investigação.
- DLLs locais não rastreadas são dependências de execução e não devem entrar no commit ou pacote.

## Abordagem

Estender os componentes existentes, com lógica de domínio compartilhada em `common/` e adaptadores nos dois hosts. Reutilizar o Mago, as cinco abas, o Santuário, o sistema de títulos e os caminhos de controles. Uma implementação duplicada por host aumentaria o risco de divergência; uma arena apenas simulada no overlay não atende ao pedido de combate real.

## Regras propostas pelo implementador

- Ondas sem final predefinido; limitar população simultânea e saturar multiplicadores/contadores para impedir overflow, sem encerrar a progressão na onda 15/20/50. A cada 10 ondas, chefe lendário prevalece sobre mini-chefe; aura de elite a cada 3 ondas deve ser representada no combate e na apresentação.
- Validar instâncias, run, slot e mapa; morte repetida não concede pontos; abortar ao morrer, sair do mapa ou trocar slot. Falha parcial de criação deve limpar apenas entidades pertencentes à tentativa.
- Tempo monotônico de jogo ativo; pausar junto com o jogo. Pontuação determinística compartilhada e documentada. `max_wave` representa a maior onda alcançada; o marco de título usa onda 20 alcançada, conforme pedido.
- `survival-v1.txt` separado por slot, validação estrita e gravação atômica através da infraestrutura existente. Não restaurar uma sessão viva após reload. Não gravar diretamente em `save*.dat` para manter recordes ou preferências.
- Último Suspiro opcional: dano letal resulta em 1 HP, imunidade por 2 segundos, cooldown de 120 segundos. Processar mitigação/damage cap antes da decisão de letalidade; impedir reentrada do hook e reaplicação de bônus. Definir o percentual de cap e duração/custo de elixir no plano de implementação após verificar as unidades reais do combate.
- Título Mestre da Sobrevivência: +120% ATK/DEF, +20% resistência, +15% evasão; persistir o desbloqueio em `titles-v1.txt` e exibir na quinta aba. Resistência/evasão exigem efeito real no cálculo de combate.
- Paridade Web por módulo de domínio e integração efetiva no fluxo existente; `localStorage` com namespace por slot. Não presumir que arquivos em dist estejam publicados.

## Execução e critérios de entrega

1. Preservar snapshot das alterações locais relevantes; mapear criação/morte de monstros, cálculo de dano, título e persistência nos hosts e na Web.
2. Escrever testes de comportamento C++/Node antes da implementação: ondas 3/5/10/20/50+, duplicação/contexto, escalonamento, tempo/pontuação, rejeição de estado inválido, atomicidade, Last Stand/cooldown/cap e bônus do título.
3. Implementar domínio, persistência e integrações nativas/Web, sem alterar a arquitetura do emulador além dos hooks verificados necessários.
4. Compilar os dois hosts e executar smoke headless de 60 quadros com dados/caminhos de teste isolados dos saves pessoais. Testes sintéticos e smoke não comprovam gameplay físico nem controle USB.
5. Atualizar README, CHANGELOG, VERSION/manifesto, `docs/VERIFICACAO_0.13.0.md` e `release/NOTAS_0.13.0.md` apenas com resultados efetivos.
6. Empacotar em `release/dist_0130/`, com executáveis, fontes curadas e SHA256; excluir DLLs externas, game, saves, dados pessoais e BAT pessoal. Revisar antes do commit explícito de arquivos pertinentes.
7. Registrar central Issue #4/HANDOFF e informar Antigravity. Publicação depende de CI Windows verde no SHA exato e demais verificações aplicáveis, conforme diretrizes. Deployment Web somente com ferramenta/acesso real e mantendo a audiência.

## Pendências de validação

Não foram executados testes C++, builds dos hosts, smoke de 60 quadros, gameplay Windows, teste USB físico, empacotamento, CI, deployment ou publicação da 0.13.0. Nada neste documento comprova essas etapas.
