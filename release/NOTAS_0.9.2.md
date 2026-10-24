> **Beta:** o jogo pode apresentar bugs; faça backup dos saves. A validação automática não substitui testes físicos.

# Destinia 0.9.2 Beta — Executáveis na raiz

## Novidade
- O ZIP Windows x64 inclui **`Destinia.exe`** e **`Destinia Zoom.exe`** diretamente na raiz da pasta extraída.
- Duplo clique nos novos inicializadores abre o host normal ou o host zoom, sem executar `.bat` e sem abrir terminal. Eles localizam os hosts e as DLLs do MSYS2 UCRT64, verificam `VERSION` e mostram aviso se faltar instalação.
- Os executáveis originais `host/build/destinia_host.exe` e `host_zoom/build/destinia_host.exe` continuam no pacote. Os BATs antigos foram preservados para suporte/diagnóstico.

## Download e requisitos
Baixe **`destinia-0.9.2-beta-windows-x64.zip`**, não o ZIP de fontes, e extraia tudo junto. O pacote contém **quatro arquivos EXE** (dois na raiz, dois nas subpastas), documentação e arquivos de configuração. Dependências SDL2/SDL2_mixer/Unicorn/zlib via MSYS2 UCRT64 e arquivos `game` originais não são incluídos. Consulte `INSTALL.md` e os checksums SHA-256.

## Compatibilidade e limitações
- Saves, gameplay, Códex, controles e perfis permanecem como na 0.9.1; esta atualização é apenas do lançamento/distribuição Windows.
- Web **inalterada**: revisão `5d05a13` da 0.9.1 em https://destinia-web-experimental.leafyheron257.chatgpt.site/ (acesso privado, progresso temporário).
- Bugs de prévia da aba Códex e mão apontadora seguem em análise no PR #3; **não são considerados corrigidos nesta release**.
- A CI compila e verifica os arquivos Windows, mas não comprova a campanha inteira nem o comportamento em cada PC.

As releases anteriores permanecem disponíveis.
