from pathlib import Path
import re
import sys

p = Path("main.cpp")
original = p.read_bytes()
new = original

def replace_once(pattern, replacement, label):
    global new
    updated, count = re.subn(pattern, replacement, new, count=1)
    if count != 1:
        sys.exit(f"ERRO: não encontrei o trecho esperado: {label}. Nada foi gravado.")
    new = updated

replace_once(
    rb"static int autoFrame = -1, autoX = 0, autoY = 0;[^\r\n]*",
    b"static int autoFrame = -1, autoX = 0, autoY = 0;   // --autotap FRAME,X,Y\n"
    b"static int autoFrame2 = -1, autoX2 = 0, autoY2 = 0; // --autotap2 FRAME,X,Y",
    "variáveis do autotap"
)

replace_once(
    rb'    else if \(a == "--autotap"\) \{ if \(sscanf\(next\(\)\.c_str\(\), "%d,%d,%d", &autoFrame, &autoX, &autoY\) != 3\) \{ printf\("--autotap precisa de QUADRO,X,Y\\n"\); return 0; \} \}',
    b'    else if (a == "--autotap") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame, &autoX, &autoY) != 3) { printf("--autotap precisa de QUADRO,X,Y\\\\n"); return 0; } }\n'
    b'    else if (a == "--autotap2") { if (sscanf(next().c_str(), "%d,%d,%d", &autoFrame2, &autoX2, &autoY2) != 3) { printf("--autotap2 precisa de QUADRO,X,Y\\\\n"); return 0; } }',
    "argumento --autotap"
)

replace_once(
    rb"\[--autotap QUADRO,X,Y\]",
    b"[--autotap QUADRO,X,Y] [--autotap2 QUADRO,X,Y]",
    "mensagem de uso"
)

replace_once(
    rb"    if \(autoFrame >= 0 && frame == autoFrame\) touch\(autoX, autoY, 0\);\r?\n    if \(autoFrame >= 0 && frame == autoFrame \+ 3\) touch\(autoX, autoY, 1\);",
    b"    if (autoFrame >= 0 && frame == autoFrame) touch(autoX, autoY, 0);\n"
    b"    if (autoFrame >= 0 && frame == autoFrame + 3) touch(autoX, autoY, 1);\n"
    b"    if (autoFrame2 >= 0 && frame == autoFrame2) touch(autoX2, autoY2, 0);\n"
    b"    if (autoFrame2 >= 0 && frame == autoFrame2 + 3) touch(autoX2, autoY2, 1);",
    "injeção dos toques"
)

backup = Path("main.cpp.bak_autotap2")
if backup.exists():
    sys.exit("ERRO: o backup já existe. Nada foi gravado.")

backup.write_bytes(original)
p.write_bytes(new)
print("Alteração aplicada; backup criado em main.cpp.bak_autotap2")
