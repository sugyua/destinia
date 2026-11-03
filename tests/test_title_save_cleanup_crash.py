"""Interruption after commit must preserve the committed character and titles."""
import pathlib, subprocess, tempfile, unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class TitleSaveCleanupCrash(unittest.TestCase):
 def test_interrupted_cleanup_preserves_committed_batch(self):
  with tempfile.TemporaryDirectory(prefix="destinia-title-cleanup-") as d:
   base=pathlib.Path(d);exe=base/"crash.exe";save=base/"save"
   subprocess.run(["g++","-std=c++17",str(ROOT/"tests/title_save_cleanup_crash.cpp"),"-o",str(exe)],check=True)
   crashed=subprocess.run([str(exe),str(save),"crash"])
   self.assertEqual(crashed.returncode,74)
   recovered=subprocess.run([str(exe),str(save),"recover"])
   self.assertEqual(recovered.returncode,0,"recovery rolled back a committed batch or failed")
   promoted=base/"save-before-promotion"
   self.assertEqual(subprocess.run([str(exe),str(promoted),"crash-before-promotion"]).returncode,75)
   self.assertEqual(subprocess.run([str(exe),str(promoted),"recover"]).returncode,0)
   initialization=base/"save-initialization"
   self.assertEqual(subprocess.run([str(exe),str(initialization),"crash-init"]).returncode,76)
   self.assertEqual(subprocess.run([str(exe),str(initialization),"recover-old"]).returncode,0)
   orphans=list(base.glob("save-initialization.transaction.init-*"));self.assertEqual(len(orphans),1)
   self.assertEqual(subprocess.run([str(exe),str(initialization),"new-after-init"]).returncode,0)
   self.assertTrue(orphans[0].is_dir())
   other=base/"save-owner"
   crashed=subprocess.run([str(exe),str(other),"crash-owner"])
   self.assertEqual(crashed.returncode,74)
   # With the ownership record destroyed, recovery must refuse to delete an
   # unidentifiable directory while leaving the committed files untouched.
   recovered=subprocess.run([str(exe),str(other),"recover"])
   self.assertEqual(recovered.returncode,4)
   self.assertEqual((other/"save0.dat").read_bytes(),b"new")
   self.assertEqual((other/"titles-v1.txt").read_bytes(),b"new")
   self.assertTrue((base/"save-owner.transaction-committed").is_dir())
   collision=base/"save-collision";collision.mkdir()
   unknown=base/"save-collision.transaction-committed";unknown.mkdir()
   (unknown/"owner").write_bytes(b"personal file")
   self.assertEqual(subprocess.run([str(exe),str(collision),"recover"]).returncode,4)
   self.assertEqual((unknown/"owner").read_bytes(),b"personal file")
   self.assertEqual(subprocess.run([str(exe),str(base/"save-error"),"cleanup-error"]).returncode,0)

if __name__ == "__main__": unittest.main()
