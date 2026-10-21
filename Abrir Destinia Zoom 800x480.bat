@echo off
setlocal DisableDelayedExpansion

set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host_zoom"
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
  echo Nao encontrei o executavel do teste de zoom:
  echo   "%EXE%"
  echo.
  echo Execute Compilar Destinia.bat antes de abrir o jogo.
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
"%EXE%" --version 2>nul | findstr.exe /x /c:"Destinia 0.5 Beta" >nul
if errorlevel 1 (
  echo Executavel antigo ou sem acesso ao runtime esperado da 0.5.
  echo Execute Compilar Destinia.bat e confira as dependencias antes de abrir.
  pause
  exit /b 1
)
cd /d "%HOST_DIR%"
if errorlevel 1 exit /b 1

echo Abrindo Destinia em tela cheia com proporcao original. F11 alterna para janela.
echo A imagem mantem a proporcao original ao redimensionar a janela.
echo Pasta de save: configuracao global do Destinia.
echo Use Configurar pasta de saves.bat para escolher o local do progresso.
echo O host de zoom e independente; o save e compartilhado com as demais versoes.
echo O diretorio game original sera apenas lido.
echo.
"%EXE%" --fullscreen
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" echo O teste terminou com codigo de erro %RESULT%.
pause
exit /b %RESULT%
