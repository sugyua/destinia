import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires SDL and Unicorn')
class TitleDeleteTests(unittest.TestCase):
 def test_abort_and_commit_do_not_leak_titles(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-title-delete-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/title_delete_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='zoom' else [])+flags
    b=subprocess.run(cmd,text=True,capture_output=True);self.assertEqual(b.returncode,0,b.stderr)
    r=subprocess.run([str(exe),str(base/variant)],text=True,capture_output=True,timeout=90);self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('TITLE_DELETE_ARM_OK',r.stdout)
if __name__=='__main__':unittest.main()
