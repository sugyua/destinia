import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer zlib e dados originais')
class CodexPortraitTests(unittest.TestCase):
 def test_original_complete_poses_variants_bosses_and_invalid_resources(self):
  self.assertTrue((ROOT/'common/codex_portraits.h').exists(),'Miniaturas originais do Codex ainda nao implementadas')
  with tempfile.TemporaryDirectory() as directory:
   exe=pathlib.Path(directory)/'portraits.exe'
   built=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/codex_portrait_cases.cpp'),'-lz','-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(built.returncode,0,built.stderr)
   run=subprocess.run([str(exe),str(ROOT/'game/assets')],capture_output=True,text=True)
   self.assertEqual(run.returncode,0,run.stderr);self.assertIn('CODEX_PORTRAITS_73_OK',run.stdout)
