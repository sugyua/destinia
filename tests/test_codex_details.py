import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer zlib e dados originais')
class CodexDetailsTests(unittest.TestCase):
 def test_original_metadata_drop_boundaries_and_malformed_resources(self):
  self.assertTrue((ROOT/'common/codex_details.h').exists(),'Metadados nativos do Codex ainda nao implementados')
  with tempfile.TemporaryDirectory() as directory:
   exe=pathlib.Path(directory)/'details.exe'
   built=subprocess.run(['g++','-std=c++17','-UNDEBUG',str(ROOT/'tests/codex_details_cases.cpp'),'-lz','-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(built.returncode,0,built.stderr)
   run=subprocess.run([str(exe),str(ROOT/'game/assets'),directory],capture_output=True,text=True)
   self.assertEqual(run.returncode,0,run.stderr);self.assertIn('CODEX_DETAILS_73_SPECIES_172_MAPS_OK',run.stdout)
