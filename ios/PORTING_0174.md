# Destinia iOS — paridade com o projeto Windows 0.17.4

**Escopo:** branch `ios/unicorn-poc`. Este arquivo registra o que foi
implementado em outras plataformas e o que falta ligar no executável iOS.
Não representa a integração concluída dessas funcionalidades.

## Versões corretas

| Camada | Versão | Origem |
| --- | --- | --- |
| Jogo Android original | **1.0.6** (`versionCode=106`) | `game/AndroidManifest.xml`, pacote `game.destiniaeng` |
| Biblioteca ARM32 | SHA-256 `f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9` | `game/lib/armeabi/libdestinia_jni.so`, 284.144 bytes |
| Port e expansões Windows | **0.17.4 Beta** | `VERSION`, `README.md`, `CHANGELOG.md` |
| Aplicativo iPhone | **0.3.3** na configuração analisada | `.github/workflows/ios-device-diagnostics.yml`; linha de versão independente |

A versão **0.17.4** **não** está embutida no motor Android: ela pertence às
funcionalidades de host (C++ Windows), Web e módulos compartilhados. O iOS
usa o mesmo ELF e os mesmos arquivos originais de `game/` que `main`,
mas, até que os adaptadores sejam portados, executa o gameplay **1.0.6**.
Alterar o número em `Info.plist` ou copiar `common/` não ativa mods.

O script `ios/tools/audit_game_source.py` verifica a identidade do motor
no build e produz um manifesto JSON declarando explicitamente a falta de
paridade com os mods Windows.

## Estado comprovado do iOS

- Unicorn-TCI ARM32 interpretado em iPhone ARM64, sem JIT.
- Carregador ELF com 213/213 relocações; 26 imports com handlers/stubs.
- `initGame`, `jniRun`, RGB565 400×240 e `jniTouch` originais.
- Assets do APK originais, saída em paisagem, entrada por toque.
- Otimizações gráficas experimentais isoladas e testes de comparação de
  framebuffer, **sem comprovar equivalência de toda a campanha**.
- Salvamento/arquivos Android redirecionados a área privada do app;
  **não equivale** ao sistema de saves/sidecars seguros do Windows.
- Áudio completo, performance sustentada e port dos mods ainda não
  validados em iPhone.

## Inventário de funcionalidades a portar

| Histórico Windows / Web | Componentes-fonte já presentes | Estado iOS |
| --- | --- | --- |
| Tradução PT-BR / textos, nomes de itens e skills | `common/portuguese_text.h`, `common/drop_names.h`, `common/skill_labels.h` | A integrar via hooks ARM (sem substituir arquivos originais) |
| Saves seguros, journal, backups e proteção de sidecars | `common/safe_save.h`, `docs/SAVES_E_LOGS.md` | A portar, com migração isolada e testes de falha |
| Códex de monstros, chefes e equipamento (0.7–0.11) | `common/codex*.h`, `common/weapons_catalog*.h` | A portar: hooks, UI, sidecars |
| Filtros de inventário, proteção, autoloot e QoL (0.9.x) | `common/inventory_*.h`, `common/loot_qol*.h` | A portar |
| Progressão até NV 999, sets e estatísticas (0.10) | `common/progression*.h`, `common/combat_power*.h` | A portar e validar com saves |
| Dificuldades, modo Abissal, títulos e pós-jogo (0.10.3–0.12) | `common/difficulty*.h`, `common/titles.h`, `common/abyss_tower.h` | A portar, incluindo a regra de drops |
| Mago dos Bosses, teleporte, pets, arena e refinamento | `common/boss_mage*.h`, `common/boss_rush_ui.h`, `common/pets_expansion.h`, `common/survival*.h`, `common/refinement*.h` | A portar por módulos |
| Baú compartilhado, contratos e catálogo (0.11) | `common/shared_stash*.h`, `common/bounties*.h`, `common/weapons_catalog*.h` | A portar com sidecars separados |
| Interfaces com moldura/fontes originais (0.17.0) | `common/original_ui*.h`, `common/quest_tracker_ui.h` | A portar; iOS hoje apresenta apenas UI nativa do motor 1.0.6 |
| Auto-poção, ouro do Mago, ajustes de zoom (0.17.3/0.17.4) | `common/auto_potion*.h`, `common/boss_mage_runtime.h`, `common/zoom_menu_view.h` | A portar; zoom Windows não se transfere diretamente |
| Mago (quarta classe, itens de catálogo) e correção de mãos duplicadas (0.17.4) | `CHANGELOG.md`, `host/main.cpp`, `common/` | A auditar/portar; **não confirmado no iOS** |
| Confirmação de salvamento Web (0.17.2) | `web/` | Específico da Web; no iOS exige equivalente nativo, não copiar JS |
| Multiplayer, novos servidores, PvP online | `docs/CONTRATO_MULTI_MODE_ARM.md` | Experimental/não integrado; **não marcar como entregue** |

**Observação sobre Obsidian:** `CHANGELOG.md` cita o problema de mãos
duplicadas como vindo do “vault Obsidian”, porém o próprio vault local
não foi fornecido nem está conectado a este ambiente. Este inventário
usa as fontes auditáveis do repositório; não afirma ser uma leitura
completa do vault.

## Ordem segura para integração

1. **Identidade/segurança:** auditar dados carregados; manter saves originais
   intactos; testar rollback e isolamento por personagem. Não reutilizar
   automaticamente sidecars sem validação e backups.
2. **Interface e idioma:** portar textos, skin original, hitboxes e controles
   touch, mantendo a geometria 400×240 e a lógica ARM32 real.
3. **Progressão persistente:** portar os adaptadores `common/` de títulos,
   dificuldades, catálogo/Códex, inventário e baú. Usar testes em saves
   sintéticos antes de saves reais.
4. **Conteúdo e mecânicas:** Boss Rush, pets, pós-jogo, Mago, autopoção e
   correções 0.17.4. Cada sistema precisa de hooks ARM, stores e UI.
5. **Qualidade:** desempenho sustentado, som, pausa/background, restauração,
   campanha real, regressões e aprovação de nova build iOS.

Não copiar `host/main.cpp` para o iOS: ele tem estado global, SDL/Windows,
caminhos de arquivo e dependências de macros específicas. Adaptar seus
hooks para `ArmImportRuntime`/Unicorn iOS em módulos verificáveis.

## Critério de aceitação de uma funcionalidade

- Compilar em ARM64 iOS e passar testes do Unicorn TCI no macOS.
- Comprovar que o hook executa sobre o **ELF original** e não em
  dados sintéticos apenas; comparar resultado com Windows quando aplicável.
- Mostrar a UI correspondente no framebuffer 400×240 e aceitar toque.
- Persistir e restaurar com isolamento de slot e tratamento de erro.
- Validar no iPhone. Só então atualizar esta tabela para **portado**.

## Fontes para comparação

- [Histórico de mudanças 0.2–0.17.4](../CHANGELOG.md)
- [README do port Windows 0.17.4](../README.md)
- [Decisões técnicas do motor original](../docs/DECISOES_TECNICAS.md)
- [Proteção de inventário e sidecar](../docs/INVENTORY_PROTECTION.md)
- [Verificação da 0.17.4](../docs/VERIFICACAO_0.17.4.md)
- [Contrato Multi Mode ARM](../docs/CONTRATO_MULTI_MODE_ARM.md)
- [Código do host Windows](../host/main.cpp)
- [Biblioteca e assets originais](../game/)
