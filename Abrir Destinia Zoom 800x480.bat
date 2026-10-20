@echo off
setlocal

set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host_zoom"
set "EXE=%HOST_DIR%\build\destinia_host.exe"
set "MSYS2_BIN=C:\msys64\ucrt64\bin"
if not defined LOCALAPPDATA (
  echo Nao foi possivel localizar a pasta local de dados do usuario.
  pause
  exit /b 1
)
set "SHARED_SAVE=%LOCALAPPDATA%\Destinia\save"

if not exist "%EXE%" (
  echo Nao encontrei o executavel do teste de zoom:
  echo   "%EXE%"
  echo.
  echo Compile a copia host_zoom pelo MSYS2 UCRT64.
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

echo Abrindo Destinia em tela cheia com proporcao original. F11 alterna para janela.
echo A imagem mantem a proporcao original ao redimensionar a janela.
echo Save compartilhado: "%SHARED_SAVE%"
echo O host de zoom e independente; o save e compartilhado com as demais versoes.
echo O diretorio game original sera apenas lido.
echo.
"%EXE%" --save "%SHARED_SAVE%" --fullscreen
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" echo O teste terminou com codigo de erro %RESULT%.
pause
exit /b %RESULT%
