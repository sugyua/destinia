# Triagem GitHub — 30/10/2026

Base inspecionada: main `92532f0b668194d5762f11cb4b81f3c69b645af1` (0.17.0). Análise de código e metadados remotos; testes históricos e CI não equivalem a gameplay físico.

## Pull requests abertos

- [PR7](https://github.com/sugyua/destinia/pull/7), HEAD4862d0f: altera gatilho PV 50%, máximo ausente e teste Web. A PR9 integrada em e4fb6b0 contém essas regras e a regressão adicional de PM independente de PV máximo. CI antiga PR7: Node/Windows PASS, revisão ignorada. Não mesclar novamente; recomendar encerramento como substituído por PR9. Não há correção exclusiva identificada no diff; documentação antiga não deve sobrescrever a atual.
- [PR8](https://github.com/sugyua/destinia/pull/8), HEAD9152ef4: adiciona selo/menu, VERSION/CMake, regras auto-poção e documentação0.16.1. Entrega substituída pela PR9, depois0.17.0. Comentários sobre PM sem PV máximo, CMAKE_CONFIGURE_DEPENDS e central foram tratados na entrega posterior. O selo Zoom proporcionalmente pequeno foi comentário informativo; não representa correção exclusiva implementada na PR8 (usa o mesmo desenho). Node PASS, Windows FAIL e CHANGES_REQUESTED antigos. Download do log Windows falhou por timeout; causa exata não afirmada. Não mesclar esse candidato antigo.
- PR9 e PR10 estão integradas; não reabrir/republicar releases0.16.1/0.17.0. Nenhum PR foi encerrado durante a análise.

## Issue #5 — dez frentes

| Frente | Estado verificado e restante |
| --- | --- |
| P1 atalhos F4/F5/F8 | Entregue0.12.1, conforme comentário do autor e keyboard.mjs/testes; F5 protegido e Tab preservado. Não reimplementar. |
| P1 coerência/pacote | Versão/manifesto/testes e notas dinâmicas estão implementados. README principal e web/README ainda mantinham descrições antigas de estado da publicação; atualizar sem apagar histórico. |
| P1 CI Web | verify-web.yml roda npm test em pushmain/PR com web/**, Node24, contentsread. CI final0.17 Node41077553904PASS. QA browser/Safari físico não é job automatizado confirmado. |
| P1 saves/recuperação | Journal/rollback, trava, backup e testes de journal desconhecido/truncado já existem. Pontes Web têm ACK e erro de armazenamento/concorrência; progressoARMWeb continua temporário e sidecars têm contratos próprios. Documentação SAVES_E_LOGS preserva descrição histórica0.5; reconciliar contrato atual num lote próprio. Falta demonstrar matriz completa de disco/permissão/reparse/interrupções. |
| P1 inicialização Unicorn | uc_open já checado genericamente; cinco mapas e dois hooks core não eram checados em ambos hosts. Save era criado/recuperado antes disso. Lote0.17.1 trata somente essa unidade com diagnóstico e boot sem acesso ao save em falha. Fingerprint de biblioteca já existe em virtual_controls.h; não anunciar nova validação de fingerprint. Hooks opcionais/Zoom restantes ficam para revisão específica. |
| P2 profiling | Ambos têm --profile, mas janelas e campos diferem. Não há p95/p99 comuns observados; nenhuma medição comparável nova executada nesta triagem. Não otimizar sem baseline. |
| P2 build/distribuição | Pacotes curados excluem dependências externas e dados; origem/licença/patch UnicornWeb documentados em THIRD_PARTY e vendor. CI de verificação contentsread; workflows históricos de publicação ainda contentswrite e Actions por tags. Revisão de histórico/permissões e pinningSHA permanece pendente. Não alterado automaticamente. |
| P2 divergência hosts | Componentes comuns já existem; câmera/viewport/Zoom intencionais preservados. Extração ampla continua sem justificativa neste lote. |
| P2 acessibilidadeWeb | Ajuda abre/fecha e restaura foco; ainda sem rolemodal/Escape/ciclo de foco observados. Loja tem role/aria-modal. Falta teste de teclado/leitor de tela e zoom200%; viewportQA 0.17 não comprova acessibilidade completa. |
| P3 fuzz/propriedades | Existem mutações ELF e regressões determinísticas, mas não foi identificado harness dedicado de fuzz com corpus/seed. Manter prioridade posterior. |

A Issue #5 deve permanecer aberta: contém trabalho não concluído. Próxima unidade de estabilidade: bootUnicorn; depois contratos de save/acessibilidade em lotes próprios. A correção do estoque da auto-poção é pendência separada conhecida: PC pode consumir item não identificado/curar sem estoque e Web cura sem debitar. Não confundir gatilho de 50% com consumo correto nem adivinhar IDs de consumíveis.

## Web

Sites vinculado permanece0.13 conforme último estado conhecido; ferramenta indisponível, sem nova chamada nesta rodada. Alternativa autorizada:0.17.0 em servidorPC privado Tailscale, endereço100.108.92.19:8000. Não há publicação pública nem mudança de audiência. Safari físico depende de confirmação do usuário.
