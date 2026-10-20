@echo off
setlocal

set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host"
set "EXE=%HOST_DIR%\build\destinia_host.exe"
set "MSYS2_BIN=C:\msys64\ucrt64\bin"
if not defined LOCALAPPDATA (
  echo Nao foi possivel localizar a pasta local de dados do usuario.
  pause
  exit /b 1
)
set "SHARED_SAVE=%LOCALAPPDATA%\Destinia\save"

if not exist "%EXE%" (
  echo Nao encontrei o executavel:
  echo   "%EXE%"
  echo.
  echo Compile o projeto pelo MSYS2 UCRT64 antes de abrir o jogo.
  pause
  exit /b 1
)

if not exist "%MSYS2_BIN%\SDL2.dll" (
  echo Nao encontrei as DLLs do MSYS2 UCRT64 em:
  echo   "%MSYS2_BIN%"
  echo Verifique a instalacao do MSYS2.
  pause
  exit /b 1
)

set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%HOST_DIR%"

echo Abrindo Destinia...
echo Save compartilhado: "%SHARED_SAVE%"
echo.
"%EXE%" --save "%SHARED_SAVE%"
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" echo O jogo terminou com codigo de erro %RESULT%.
pause
exit /b %RESULT%
