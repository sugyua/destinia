#!/usr/bin/env bash
# Chamado pelo BAT com o Bash do MSYS2. Nao instala dependencias nem acessa saves.
set -eu
export PATH="/ucrt64/bin:/usr/bin:$PATH"
case "${2-}" in
  host_zoom) targets=(host_zoom) ;;
  host) targets=(host) ;;
  ambos) targets=(host host_zoom) ;;
  *) printf '%s\n' 'Alvo invalido. Use host_zoom, host ou ambos.' >&2; exit 2 ;;
esac
for tool in cygpath cmake ninja g++ pkg-config; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    printf 'Ferramenta ausente: %s. Consulte Dependencias no README e INSTALL.md.\n' "$tool" >&2
    exit 1
  fi
done
project_root=$(cygpath -u -- "${1-}")
cd -- "$project_root"
for target in "${targets[@]}"; do
  if [[ ! -f "$target/CMakeLists.txt" ]]; then
    printf 'Fonte ausente: %s/CMakeLists.txt. Extraia o pacote na raiz do projeto.\n' "$target" >&2
    exit 1
  fi
  printf '\nConfigurando %s...\n' "$target"
  if cmake -S "$target" -B "$target/build" -G Ninja -DCMAKE_BUILD_TYPE=Release; then :; else
    result=$?; printf 'Falha na configuracao de %s. Confira o erro acima.\n' "$target" >&2; exit "$result"
  fi
  printf '\nCompilando %s...\n' "$target"
  if cmake --build "$target/build"; then :; else
    result=$?; printf 'Falha na compilacao de %s. Confira o erro acima.\n' "$target" >&2; exit "$result"
  fi
  printf 'Compilacao concluida: %s/build/destinia_host.exe\n' "$target"
done
