import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class ManaGuardTests(unittest.TestCase):
 def test_stable_excess_and_transitions(self):
  with tempfile.TemporaryDirectory(prefix="destinia-mana-unit-") as d:
   exe=pathlib.Path(d)/"mana.exe"
   built=subprocess.run(["g++","-std=c++17",str(ROOT/"tests/mana_guard_cases.cpp"),"-o",str(exe)],capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
