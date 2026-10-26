@echo off
setlocal DisableDelayedExpansion

set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host"
set "EXE=%HOST_DIR%\build\destinia_host.exe"
set "MSYS2_ROOT=%DESTINIA_MSYS2_ROOT%"
if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
set "MSYS2_BIN=%MSYS2_ROOT%\ucrt64\bin"
if not defined LOCALAPPDATA (
  echo Nao foi possivel localizar a pasta local de dados do usuario.
  pause
  exit /b 1
)

if not exist "%EXE%" (
  echo Nao encontrei o executavel:
  echo   "%EXE%"
  echo.
  echo Execute Compilar Destinia.bat antes de abrir o jogo.
  pause
  exit /b 1
)

if exist "%HOST_DIR%\build\SDL2.dll" (
  set "PATH=%HOST_DIR%\build;%PATH%"
) else if exist "%ROOT%SDL2.dll" (
  set "PATH=%ROOT%;%PATH%"
) else if exist "%MSYS2_BIN%\SDL2.dll" (
  set "PATH=%MSYS2_BIN%;%PATH%"
) else (
  echo Nao encontrei as DLLs do runtime em "%HOST_DIR%\build" nem em:
  echo   "%MSYS2_BIN%"
  echo Verifique a instalacao do MSYS2 ou utilize o pacote portatil completo.
  pause
  exit /b 1
)
if not exist "%ROOT%VERSION" (
  echo Falta VERSION. Extraia todos os arquivos do pacote na mesma pasta.
  pause
  exit /b 1
)
set "EXPECTED_VERSION="
set /p "EXPECTED_VERSION="<"%ROOT%VERSION"
powershell.exe -NoProfile -Command "if ($env:EXPECTED_VERSION -cmatch '^[0-9]+\.[0-9]+(\.[0-9]+)?$') { exit 0 } else { exit 1 }"
if errorlevel 1 (
  echo VERSION invalido ou Windows PowerShell indisponivel. Confira o pacote completo.
  pause
  exit /b 1
)
"%EXE%" --version 2>nul | findstr.exe /x /c:"Destinia %EXPECTED_VERSION% Beta" >nul
if errorlevel 1 (
  echo Executavel incompativel com VERSION ou sem acesso ao runtime esperado.
  echo Execute Compilar Destinia.bat e confira as dependencias antes de abrir.
  pause
  exit /b 1
)
cd /d "%HOST_DIR%"
if errorlevel 1 exit /b 1

echo Abrindo Destinia...
echo Pasta de save: configuracao global do Destinia.
echo Use Configurar pasta de saves.bat para escolher o local do progresso.
echo.
"%EXE%"
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" echo O jogo terminou com codigo de erro %RESULT%.
pause
exit /b %RESULT%
