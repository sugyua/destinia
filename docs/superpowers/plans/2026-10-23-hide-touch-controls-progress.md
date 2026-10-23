# SDD ledger — plan: docs/superpowers/plans/2026-10-23-hide-touch-controls.md

Pre-flight: tarefas 1/2 compartilham interface.<host>.controles_virtuais; padrão 1, validação 0/1. Sem conflito.
Ruling: executar diretamente nesta sessão, seguindo preferência anterior do autor por eficiência; nenhum subagente implementador por tarefa.
Ruling: os dois hosts usam as mesmas funções ARM e passaram no teste de render/hitboxes; ampliar a opção aos dois, com configuração independente.
Base: 88eeb27; worktree /workspace/destinia-ui08, original com Configurar Destinia.bat pendente preservado.
Exploração: _getTouchState real confirmou GameCanvas, estado 0 => toque 2; estado -1 => 3; estado 16/UI0 =>4; estado16/UI20000001 =>5. Apenas 2/3 são ocultados.
RED preferências: compilação falhou por virtualControls ausente.
RED integração: configuração de ocultação ainda mantinha key5; falha de assert key==0 em ambos os hosts.
GREEN inicial: preferência e integração passaram após implementação, com controles visíveis/ocultos nos dois hosts.
Verificação ampliada: menu adicional estado5, chamadas físicas GameCanvas, fingerprint incompatível; passou em 14,224s.
Baseline: preferências e câmera original passaram antes da modificação.
Task 1: complete — suíte Python com DESTINIA_NATIVE_TESTS=1: 38 testes, 37 aprovados e 1 skip em 77,981s; o skip de renderização foi depois executado separadamente e aprovado (total 38 verificações aprovadas).
Task 2: implementação e documentação locais concluídas — opção6, reset e testes PowerShell preparados; execução PowerShell pendente porque shell não está instalado neste ambiente.
Build: g++ C++17 -O2 -DHAS_SDL_MIXER compilou ambos os hosts, exit0; --version mantém 0.7 Beta enquanto a0.8 está em preparação.
Revisão independente: sem achados altos/médios; observou que além do desenho são omitidas animações dos botões ocultos. Conferido que comandos físicos não usam esses frames. Comentário/especificação corrigidos para descrever isso, sem mudança funcional.
Diff check passou; arquivo pendente do checkout original preservado.
Pendente: configurador PowerShell real, visual/campanha Windows, publicação e atualização web. Escala independente da UI é a próxima etapa, não implementada aqui.
Nenhum commit/push/release do produto nesta etapa; somente coordenação privada atualizada.
