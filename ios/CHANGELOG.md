# Histórico do port Destinia para iOS

> Esta cronologia acompanha o **host iOS** e não deve ser confundida com
> [as alterações de gameplay do Windows 0.17.4 Beta](../CHANGELOG.md).
> O motor efetivamente carregado continua sendo o APK Android 1.0.6.

## Auditoria de paridade — 05/11/2026 (branch ios/unicorn-poc)

- Identificada a origem do APK: `game.destiniaeng`, Android 1.0.6,
  `versionCode=106`; `libdestinia_jni.so` com SHA-256
  `f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9`.
- Confirmado que o repositório Windows mantém `VERSION=0.17.4`, mas
  o iOS ainda não executa os adaptadores das expansões Windows.
- Incluída auditoria automática em `ios/tools/audit_game_source.py`;
  manifesto de proveniência incluído nas novas IPAs.
- Criada matriz de funcionalidades e lacunas em
  [`PORTING_0174.md`](PORTING_0174.md), usando o CHANGELOG e as
  verificações existentes no GitHub.
- Nenhum gameplay mod do Windows foi declarado como portado por esta
  atualização. A integridade de saves deve ser preservada.

## iOS 0.3.x — interface e desempenho

- Renderização real RGB565 400×240 do motor ARM32 via `jniRun`.
- Tela em paisagem, passagem de eventos de toque via `jniTouch`.
- Faixa de diagnóstico removida durante gameplay.
- Unicorn-TCI em build Release e correção de pacing.
- Aceleradores nativos de desenho de retângulos/sprites sob
  comparação byte a byte dos framebuffers.
- Ajustes de reuso de buffers JNI para sustentar mais quadros.
- Desempenho no iPhone e campanha completa ainda em validação.

## Inicialização experimental iOS 0.1–0.2

- Unicorn-TCI ARM32 sem JIT no iOS ARM64.
- Leitura e mapeamento da biblioteca ELF32 ARM original.
- Resolução das 213 relocações; 26 imports ARM32 mapeados.
- Emulação de partes da libc, matemática, arquivos, JNI e
  execução de `initGame` original.

## Expansões de jogo Windows 0.2–0.17.4

Consulte o [CHANGELOG geral](../CHANGELOG.md) e a
[matriz de portabilidade](PORTING_0174.md). Estes itens não se tornam
funcionais no iPhone simplesmente pela presença das fontes `common/`.
É necessária integração e validação explícita por módulo.
