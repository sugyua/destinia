# Destinia para iOS — emulação ARM32

Este diretório contém o port nativo iOS, mantido **somente na branch**
[`ios/unicorn-poc`](https://github.com/sugyua/destinia/tree/ios/unicorn-poc).

## Que versão estou jogando?

- **Jogo Android original:** Gamevil/Everple **1.0.6**, package
  `game.destiniaeng`. A biblioteca `libdestinia_jni.so` e os assets
  originais são executados na CPU ARM32 emulada.
- **Projeto expandido Windows:** **0.17.4 Beta**, declarado em
  [`VERSION`](../VERSION), com mecânicas e interfaces implementadas nos
  adaptadores `host/`, `host_zoom/`, `web/` e `common/`.
- **Aplicativo iPhone:** versão `0.3.x` definida no workflow/Info.plist.
  Esse número é a evolução do emulador iOS; **não é a versão do conteúdo**.

**Atenção:** o iOS ainda não herdou automaticamente as mudanças da
0.17.4 para PC. Ele roda o motor Android original com compatibilidade
de sistema, entrada por toque, gráficos e otimizações próprias. O
histórico e as diferenças estão no
[Inventário de funcionalidades a portar](PORTING_0174.md).

## O que já executa

- Unicorn-TCI ARM32 sem JIT, sobre iOS ARM64.
- ELF original, 213 relocações e 26 símbolos vinculados.
- `initGame`, `jniRun`, tela 400×240 RGB565 e `jniTouch`.
- JNI/arquivos/strings/matemática necessários ao boot.
- Modo paisagem com toque; testes no aparelho.
- Integração gráfica nativa experimental, sob testes de equivalência
  de framebuffer.

## Verificação da origem

Na raiz do checkout:

```sh
python3 ios/tools/audit_game_source.py
```

A auditoria confirma o APK **1.0.6**, o SHA-256 da biblioteca ARM e
a versão `VERSION` do projeto Windows. Ela falha se o motor Android
for substituído silenciosamente. O resultado registra
`windows_extensions_ported: false` enquanto não houver provas de
paridade.

## Para continuar o port

- [Matriz de portabilidade 0.17.4 e prioridades](PORTING_0174.md)
- [Histórico 0.2–0.17.4](../CHANGELOG.md)
- [Decisões técnicas](../docs/DECISOES_TECNICAS.md)

Não atribuir ao port iOS funcionalidades do Windows apenas porque
elas existem em `common/`; devem ser integradas aos hooks ARM/JNI,
ter persistência própria e passar em testes no iPhone. Não alterar
saves reais sem backup e validação de compatibilidade.
