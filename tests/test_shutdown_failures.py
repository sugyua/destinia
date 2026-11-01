import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2/Unicorn')
class ShutdownFailure(unittest.TestCase):
 def test_exception_does_not_flush_dirty_codex(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-shutdown-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for zoom in (False,True):
    exe=base/('zoom.exe' if zoom else 'normal.exe')
    cmd=['g++','-std=c++17']+(['-DDESTINIA_TEST_ZOOM'] if zoom else [])+[str(ROOT/'tests/shutdown_failure_native.cpp'),'-o',str(exe)]+flags
    built=subprocess.run(cmd,text=True,capture_output=True);self.assertEqual(built.returncode,0,built.stderr)
    run=subprocess.run([str(exe),str(base/str(zoom))],cwd=base,text=True,capture_output=True,timeout=30)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr)
