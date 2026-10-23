# Controles virtuais opcionais Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Ocultar controles virtuais e seus toques sem modificar o cenário.
**Architecture:** Preferência persistente; interceptações ARM pontuais, condicionadas ao estado nativo e à biblioteca validada. Manter efeitos de estado e epílogos originais.
**Tech Stack:** C++17, SDL2, Unicorn, PowerShell, testes Python/C++.
**Spec:** docs/superpowers/specs/2026-10-23-hide-touch-controls-design.md

## Global Constraints
Padrão visível. Não mudar zoom, saves ou ritmo. Não tocar Configurar Destinia.bat. Não publicar release.

## Review Focus
- Estados de menus e entrada de nomes não podem perder toques.
- Controles ocultos não podem deixar hitboxes ativas.
- Atualização de estado do desenho deve continuar ocorrendo.
- Teclado e controle devem continuar operantes.
- Biblioteca incompatível não pode receber hooks desconhecidos.

### Task 1: Confirmar contratos ARM e implementar visibilidade
Files: common/user_settings.h; host_zoom/main.cpp; tests/preferences_cases.cpp; tests/touch_controls_native.cpp; tests/test_touch_controls.py.
Interfaces: Preferences::virtualControls bool por host; instalação de dois hooks pontuais no desenho e no resultado da seleção de toque.
- [x] Confirmar estados nativos e contratos de registradores em execução controlada; registrar resultado.
- [x] Escrever testes de preferência e integração ARM; observar falha antes de implementação.
- [x] Implementar preferência e hooks restritos ao gameplay, mantendo epílogos e efeitos de estado.
- [x] Executar testes novos e regressões de câmera, entrada e Códex afetadas.

### Task 2: Configurador e documentação
Files: scripts/configure_game.ps1; scripts/game_preferences.ps1; docs/PREFERENCIAS.md; CHANGELOG.md.
Interfaces: interface.host_zoom.controles_virtuais e eventual interface.host.controles_virtuais, somente hosts validados.
- [x] Expor escolha no configurador e restaurar padrão sem remover chaves desconhecidas ou save_dir.
- [x] Documentar escopo real, reinício necessário, testes e limites. Marcar item 09 somente se completo.
- [x] Revisar diff e testes, registrar resultado na central privada sem publicar release.

Resultado: implementação e testes locais concluídos. PowerShell/Windows e campanha manual pendentes, sem publicação. Detalhes no arquivo de progresso ao lado.
