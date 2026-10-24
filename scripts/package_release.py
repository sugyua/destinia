#!/usr/bin/env python3
"""Empacota somente os fontes explicitamente revisados; nunca inclui game ou saves."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import zipfile


def package(root, output, manifest):
    root, output = Path(root).resolve(), Path(output).resolve()
    version = manifest["version"]
    if not re.fullmatch(r"[0-9]+\.[0-9]+(?:\.[0-9]+)?", version):
        raise ValueError("Versao invalida")
    if (root / "VERSION").read_text(encoding="utf-8").strip() != version:
        raise ValueError("VERSION e manifesto divergem")
    base = manifest["base_commit"]
    if not re.fullmatch(r"[0-9a-f]{40}", base):
        raise ValueError("Commit base invalido")
    inputs = {}
    for name in manifest["files"]:
        path = PurePosixPath(name)
        if (not name or "\\" in name or ":" in name or path.is_absolute()
                or path.as_posix() != name or ".." in path.parts
                or any(part.startswith(".") and part != ".gitignore" for part in path.parts)
                or any(part in {"game", "save", "saves", "build", "relatorios", "Claude"}
                       or part.startswith(("save_", "build_")) for part in path.parts)
                or any(part == "dist" and not (index == 1 and path.parts[0] == "web")
                       for index, part in enumerate(path.parts))
                or path.suffix.lower() in {".exe", ".dll", ".so", ".dex", ".log", ".dat"}
                or ".bak" in path.name or name in inputs or name == "SOURCE_BASE.txt"):
            raise ValueError("Arquivo nao permitido: " + name)
        source = root / name
        if not source.is_file() or not source.resolve().is_relative_to(root):
            raise ValueError("Arquivo ausente ou fora da origem: " + name)
        if any(parent.is_symlink() for parent in (source, *source.parents) if parent != root):
            raise ValueError("Link simbolico nao permitido: " + name)
        inputs[name] = source.read_bytes()
    if not inputs:
        raise ValueError("Manifesto vazio")
    prefix = "destinia-" + version + "-beta"
    archive = output / (prefix + "-source.zip")
    checksum = archive.with_suffix(".zip.sha256")
    if archive.exists() or checksum.exists():
        raise FileExistsError("Artefato ja existe; use outro diretorio de saida")
    inputs["SOURCE_BASE.txt"] = (
        "Repositorio: https://github.com/sugyua/destinia\n"
        "Commit base: " + base + "\n"
        "Versao dos fontes: " + version + " Beta.\n"
        "Inclui alteracoes posteriores ao commit base; nao e um checkout integral desse commit.\n"
    ).encode("utf-8")
    output.mkdir(parents=True, exist_ok=True)
    try:
        with zipfile.ZipFile(archive, "x", compression=zipfile.ZIP_DEFLATED) as zipped:
            for name, data in sorted(inputs.items()):
                info = zipfile.ZipInfo(prefix + "/" + name, (2026, 10, 20, 0, 0, 0))
                info.create_system = 3
                info.external_attr = 0o100644 << 16
                info.compress_type = zipfile.ZIP_DEFLATED
                zipped.writestr(info, data)
        digest = hashlib.sha256(archive.read_bytes()).hexdigest()
        with checksum.open("x", encoding="ascii") as stream:
            stream.write(digest + "  " + archive.name + "\n")
    except Exception:
        archive.unlink(missing_ok=True)
        raise
    return archive


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    try:
        manifest = json.loads((args.root / "release/manifest.json").read_text(encoding="utf-8"))
        archive = package(args.root, args.out, manifest)
    except (ValueError, OSError, KeyError) as error:
        parser.exit(1, "Erro ao empacotar: " + str(error) + "\n")
    print(archive)


if __name__ == "__main__":
    main()
