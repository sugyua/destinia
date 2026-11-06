#!/usr/bin/env python3
"""Audita a origem do motor Android usado no port iOS.

Nao altera arquivos do jogo e nao afirma que mods do host Windows
foram integrados ao emulador iOS.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
EXPECTED_PACKAGE = "game.destiniaeng"
EXPECTED_ANDROID_VERSION = "1.0.6"
EXPECTED_ENGINE_SHA256 = (
    "f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9"
)


def u16(data: bytes, at: int) -> int:
    return struct.unpack_from("<H", data, at)[0]


def u32(data: bytes, at: int) -> int:
    return struct.unpack_from("<I", data, at)[0]


def manifest_fields(data: bytes) -> dict[str, str | int]:
    if u16(data, 0) != 3:
        raise ValueError("AndroidManifest.xml nao e AXML binario")
    strings: list[str] = []
    at = u16(data, 2)
    while at < len(data):
        kind, header, size = struct.unpack_from("<HHI", data, at)
        if size < header or size == 0:
            raise ValueError("Chunk AXML invalido")
        if kind == 1:
            count = u32(data, at + 8)
            flags = u32(data, at + 16)
            strings_start = u32(data, at + 20)
            for i in range(count):
                p = at + strings_start + u32(data, at + header + 4 * i)
                if flags & 0x100:
                    def length(pos: int) -> tuple[int, int]:
                        n = data[pos]
                        return (((n & 127) << 8) | data[pos + 1], 2) if n & 128 else (n, 1)
                    _, consumed = length(p)
                    p += consumed
                    byte_count, consumed = length(p)
                    p += consumed
                    value = data[p:p + byte_count].decode("utf-8")
                else:
                    chars = u16(data, p)
                    p += 2
                    if chars & 0x8000:
                        chars = ((chars & 0x7fff) << 16) | u16(data, p)
                        p += 2
                    value = data[p:p + chars * 2].decode("utf-16le")
                strings.append(value)
        elif kind == 0x102 and strings:
            name_id = u32(data, at + 20)
            if strings[name_id] == "manifest":
                attr_offset = u16(data, at + 24)
                attr_size = u16(data, at + 26)
                attr_count = u16(data, at + 28)
                result: dict[str, str | int] = {}
                for i in range(attr_count):
                    p = at + 16 + attr_offset + i * attr_size
                    name = strings[u32(data, p + 4)]
                    raw = u32(data, p + 8)
                    typ = data[p + 15]
                    value = u32(data, p + 16)
                    result[name] = (
                        strings[raw] if raw != 0xffffffff else
                        strings[value] if typ == 3 else value
                    )
                return result
        at += size
    raise ValueError("Elemento <manifest> nao encontrado")


def audit() -> dict[str, object]:
    game = ROOT / "game"
    elf = game / "lib" / "armeabi" / "libdestinia_jni.so"
    android = manifest_fields((game / "AndroidManifest.xml").read_bytes())
    digest = hashlib.sha256(elf.read_bytes()).hexdigest()
    assets = [f for f in (game / "assets").rglob("*") if f.is_file()]
    repo_version = (ROOT / "VERSION").read_text(encoding="utf-8").strip()

    if android.get("package") != EXPECTED_PACKAGE:
        raise ValueError(f"Pacote Android inesperado: {android.get('package')}")
    if android.get("versionName") != EXPECTED_ANDROID_VERSION:
        raise ValueError(f"APK Android alterado: {android.get('versionName')}")
    if digest != EXPECTED_ENGINE_SHA256:
        raise ValueError(f"Biblioteca Android alterada: {digest}")
    if not (game / "assets" / "0_en.ddt").is_file() or len(assets) < 800:
        raise ValueError("Conjunto de assets original incompleto")

    return {
        "android_package": android["package"],
        "android_game_version": android["versionName"],
        "android_version_code": android.get("versionCode"),
        "engine_library": "libdestinia_jni.so",
        "engine_sha256": digest,
        "game_asset_file_count": len(assets),
        "windows_project_version": repo_version,
        "ios_port_version": os.environ.get("IOS_PORT_VERSION", "not-specified"),
        "windows_extensions_ported": False,
        "parity_status": "original_android_engine_only",
        "commit": os.environ.get("GITHUB_SHA", "local"),
    }


def main() -> int:
    try:
        result = audit()
        output = json.dumps(result, ensure_ascii=False, indent=2) + "\n"
        if len(sys.argv) > 1:
            target = Path(sys.argv[1])
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(output, encoding="utf-8")
        print(output)
        return 0
    except (OSError, UnicodeError, ValueError, struct.error) as error:
        print(f"FALHOU AUDITORIA DESTINIA: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
