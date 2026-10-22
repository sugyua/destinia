import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class CodexDetailsUiTests(unittest.TestCase):
 def test_separate_drop_pages_colors_hidden_data_and_framebuffer_bounds(self):
  with tempfile.TemporaryDirectory() as directory:
   exe=pathlib.Path(directory)/'details-ui'
   result=subprocess.run(['g++','-std=c++17','-UNDEBUG',str(ROOT/'tests/codex_details_ui_cases.cpp'),'-lz','-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(result.returncode,0,result.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(run.returncode,0,run.stderr)
