import os,pathlib,shutil,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SurvivalTransactionCrash(unittest.TestCase):
 def test_recovery_at_each_reward_replacement_and_after_commit(self):
  with tempfile.TemporaryDirectory(prefix='destinia-survival-crash-',ignore_cleanup_errors=True) as d:
   root=pathlib.Path(d);exe=root/('probe.exe' if os.name=='nt' else 'probe')
   subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests/survival_transaction_crash.cpp'),'-o',str(exe)],check=True)
   for case_id in list(range(1,7))+list(range(11,17)):
    point=case_id%10
    case=root/str(case_id)
    result=subprocess.run([str(exe),str(case),str(case_id)],capture_output=True,text=True)
    self.assertEqual(result.returncode,72 if point==6 else 71,(point,result.stdout,result.stderr))
    result=subprocess.run([str(exe),str(case),'recover-new' if point==6 else ('recover-old-missing-title' if case_id>10 else 'recover-old')],capture_output=True,text=True)
    self.assertEqual(result.returncode,0,(point,result.stdout,result.stderr))
if __name__=='__main__':unittest.main()
