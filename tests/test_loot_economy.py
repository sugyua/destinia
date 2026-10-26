"""Unit and integration tests for Meta 0.10 Frente 2: Loot and Economy."""
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

# Ensure compiler is found on Windows MSYS2 / UCRT64 setups
UCRT_BIN = pathlib.Path(r"C:\msys64\ucrt64\bin")
if UCRT_BIN.exists() and str(UCRT_BIN) not in os.environ.get("PATH", ""):
    os.environ["PATH"] = str(UCRT_BIN) + os.pathsep + os.environ.get("PATH", "")

class LootEconomyTests(unittest.TestCase):
    def test_boss_drops_sets_and_offline_cash_cpp(self):
        compiler = "g++"
        if not subprocess.run(["where.exe", "g++"], capture_output=True).returncode == 0:
            if (UCRT_BIN / "g++.exe").exists():
                compiler = str(UCRT_BIN / "g++.exe")
        with tempfile.TemporaryDirectory(prefix="destinia_economy_") as folder:
            exe = pathlib.Path(folder) / ("economy_cases.exe" if os.name == "nt" else "economy_cases")
            cmd = [compiler, "-std=c++17", str(ROOT / "tests/loot_economy_cases.cpp"), "-o", str(exe)]
            built = subprocess.run(cmd, capture_output=True, text=True)
            self.assertEqual(built.returncode, 0, built.stderr)
            run = subprocess.run([str(exe), folder], capture_output=True, text=True, timeout=30)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            self.assertIn("ALL_LOOT_ECONOMY_TESTS_OK", run.stdout)

if __name__ == "__main__":
    unittest.main()
