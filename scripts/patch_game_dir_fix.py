#!/usr/bin/env python3
from pathlib import Path

code_to_insert = """  if (!fs::exists(gameDir + "/lib/armeabi/libdestinia_jni.so")) {
    if (fs::exists("game/lib/armeabi/libdestinia_jni.so")) {
      gameDir = "game";
    } else if (fs::exists("./game/lib/armeabi/libdestinia_jni.so")) {
      gameDir = "./game";
    } else if (fs::exists("../game/lib/armeabi/libdestinia_jni.so")) {
      gameDir = "../game";
    } else if (argc > 0 && argv[0]) {
      fs::path exeDir = fs::absolute(argv[0]).parent_path();
      if (fs::exists(exeDir / "game" / "lib" / "armeabi" / "libdestinia_jni.so")) {
        gameDir = (exeDir / "game").string();
      } else if (fs::exists(exeDir / ".." / "game" / "lib" / "armeabi" / "libdestinia_jni.so")) {
        gameDir = (exeDir / ".." / "game").string();
      }
    }
  }
"""

for p in [Path('host/main.cpp'), Path('host_zoom/main.cpp')]:
    text = p.read_text(encoding='utf-8')
    target = '  if (soPath.empty()) soPath = gameDir + "/lib/armeabi/libdestinia_jni.so";'
    if 'if (!fs::exists(gameDir + "/lib/armeabi/libdestinia_jni.so"))' not in text:
        text = text.replace(target, code_to_insert + target)
        p.write_text(text, encoding='utf-8')
        print(f'{p} patched with gameDir auto-detection!')

# Update scripts/compile_windows.sh to copy executables to root
p_sh = Path('scripts/compile_windows.sh')
sh_text = p_sh.read_text(encoding='utf-8')
old_done = '  printf \'Compilacao concluida: %s/build/destinia_host.exe\\n\' "$target"\ndone'
new_done = """  printf 'Compilacao concluida: %s/build/destinia_host.exe\\n' "$target"
  if [[ "$target" == "host" ]]; then
    cp -f host/build/destinia_host.exe Destinia.exe 2>/dev/null || true
  elif [[ "$target" == "host_zoom" ]]; then
    cp -f host_zoom/build/destinia_host.exe "Destinia Zoom.exe" 2>/dev/null || true
  fi
done"""

if 'cp -f host/build/destinia_host.exe Destinia.exe' not in sh_text:
    sh_text = sh_text.replace(old_done, new_done)
    p_sh.write_text(sh_text, encoding='utf-8')
    print('scripts/compile_windows.sh patched!')
