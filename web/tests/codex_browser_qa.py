"""Entrada anterior; agora valida o Códex integrado à barra original."""
import runpy,pathlib
runpy.run_path(str(pathlib.Path(__file__).with_name('browser_native_codex_qa.py')),run_name='__main__')
