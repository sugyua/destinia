import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class CodexTests(unittest.TestCase):
 def test_discover_kill_dedup_respawn_slots_and_persistence(self):
  self.assertTrue((ROOT/'common/codex.h').exists(),'Códex ainda não implementado')
  with tempfile.TemporaryDirectory() as folder:
   exe=pathlib.Path(folder)/'codex.exe'
   subprocess.run(['g++','-std=c++17',str(ROOT/'tests/codex_cases.cpp'),'-o',str(exe)],check=True,capture_output=True)
   result=subprocess.run([str(exe),str(pathlib.Path(folder)/'data')],capture_output=True,text=True)
   self.assertEqual(result.returncode,0,result.stderr)
