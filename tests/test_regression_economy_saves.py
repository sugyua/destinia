"""No real saves: durable regressions for bounties and sidecar protections."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class EconomySaveRegressions(unittest.TestCase):
    def test_no_exit_sweep_rewrites_or_fabricates_sidecars(self):
        for name in ("host/main.cpp", "host_zoom/main.cpp"):
            with self.subTest(host=name):
                source = (ROOT / name).read_text(encoding="utf-8-sig")
                # Data owners must persist only the sidecars they actually use.
                self.assertNotIn("saveAllSidecarsAtomic(", source)
                self.assertNotIn(".atomic.tmp", source)

    def test_keyboard_and_mouse_claims_are_fail_closed_and_durable(self):
        with tempfile.TemporaryDirectory(prefix="destinia-bounty-regression-") as temp:
            folder=Path(temp)
            binary=folder/"bounties_regression"
            compilation=subprocess.run(
                ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra",
                 str(ROOT/"tests/bounties_claim_regression.cpp"), "-o", str(binary)],
                capture_output=True,text=True,timeout=60)
            self.assertEqual(compilation.returncode,0,compilation.stderr)
            result=subprocess.run([str(binary),str(folder/"synthetic-saves")],
                                  capture_output=True,text=True,timeout=30)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn("BOUNTIES_REGRESSION_PASS",result.stdout)

if __name__ == "__main__":
    unittest.main()
