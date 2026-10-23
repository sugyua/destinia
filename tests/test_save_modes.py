import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SaveModeTests(unittest.TestCase):
 def test_isolation_explicit_copy_and_rejection(self):
  with tempfile.TemporaryDirectory(prefix='destinia modes ') as folder:
   exe=pathlib.Path(folder)/('cases.exe' if os.name=='nt' else 'cases')
   build=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/save_modes_cases.cpp'),'-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(build.returncode,0,build.stderr)
   run=subprocess.run([str(exe),folder],capture_output=True,text=True)
   self.assertEqual(run.returncode,0,run.stderr);self.assertIn('SAVE_MODES_OK',run.stdout)
