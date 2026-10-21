import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile


SCRIPT = Path(__file__).resolve().parents[1] / "scripts/package_release.py"


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.assertTrue(SCRIPT.is_file(), "Empacotador ainda nao implementado")
        spec = importlib.util.spec_from_file_location("package_release", SCRIPT)
        self.module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.module)
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / "source"
        self.root.mkdir()
        # Fixture binaria: manter bytes LF identicos tambem no Windows.
        (self.root / "VERSION").write_bytes(b"0.2\n")
        (self.root / "README.md").write_text("Beta\n", encoding="utf-8")
        self.out = Path(self.tmp.name) / "output"

    def manifest(self, files):
        return {"version": "0.2", "base_commit": "d9a9352bbbe9902b7b9bc3ef0dac2353b3f26611", "files": files}

    def test_allowlist_omits_unlisted_saves_credentials_and_game(self):
        for name in (".env", "save/msave.dat", "game/classes.dex"):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"not for distribution")
        archive = self.module.package(self.root, self.out, self.manifest(["VERSION", "README.md"]))
        with zipfile.ZipFile(archive) as zipped:
            self.assertEqual(set(zipped.namelist()), {"destinia-0.2-beta/VERSION", "destinia-0.2-beta/README.md", "destinia-0.2-beta/SOURCE_BASE.txt"})
            self.assertEqual(zipped.read("destinia-0.2-beta/VERSION"), b"0.2\n")
        checksum = archive.with_suffix(".zip.sha256").read_text().split()[0]
        self.assertEqual(checksum, hashlib.sha256(archive.read_bytes()).hexdigest())

    def test_rejects_forbidden_and_escaping_paths(self):
        for name in ("../outside", "/etc/passwd", "game/classes.dex", "save/msave.dat", ".env", "host/main.cpp.bak", "host/build/app.exe", "C:/Windows/file", "a\\b"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.module.package(self.root, self.out, self.manifest([name]))

    def test_rejects_symlink_to_outside(self):
        outside = self.root.parent / "private.txt"
        outside.write_text("private")
        (self.root / "linked.txt").symlink_to(outside)
        with self.assertRaises(ValueError):
            self.module.package(self.root, self.out, self.manifest(["linked.txt"]))

    def test_missing_input_produces_no_archive(self):
        with self.assertRaises(ValueError):
            self.module.package(self.root, self.out, self.manifest(["missing.txt"]))
        self.assertFalse(self.out.exists())

    def test_rejects_version_mismatch(self):
        manifest = self.manifest(["VERSION"])
        manifest["version"] = "0.3"
        with self.assertRaises(ValueError):
            self.module.package(self.root, self.out, manifest)

    def test_reproducible_archive_and_no_overwrite(self):
        manifest = self.manifest(["VERSION", "README.md"])
        archive = self.module.package(self.root, self.out, manifest)
        other = self.module.package(self.root, self.out.parent / "other", manifest)
        self.assertEqual(archive.read_bytes(), other.read_bytes())
        with self.assertRaises(FileExistsError):
            self.module.package(self.root, self.out, manifest)


if __name__ == "__main__":
    unittest.main()
