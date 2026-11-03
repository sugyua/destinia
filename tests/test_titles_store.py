"""Title schema, sidecar IO, unlock and legacy boss-menu regression cases."""
import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]

class TitleStoreTests(unittest.TestCase):
 def test_title_cases(self):
  with tempfile.TemporaryDirectory(prefix='destinia-title-cases-',ignore_cleanup_errors=True) as d:
   for name in ('title_store_safe_cases','title_store_io_cases','title_unlock_cases','title_boss_save_cases'):
    with self.subTest(case=name):
     exe=pathlib.Path(d)/(name+'.exe')
     subprocess.run(['g++','-std=c++17',str(ROOT/'tests'/(name+'.cpp')),'-o',str(exe)],check=True)
     subprocess.run([str(exe)],check=True,timeout=60)
