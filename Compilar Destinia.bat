@echo off
setlocal DisableDelayedExpansion
set "ROOT=%~dp0"
set "MSYS2_ROOT=%DESTINIA_MSYS2_ROOT%"
if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
if not exist "%MSYS2_ROOT%\usr\bin\bash.exe" (
  echo Nao encontrei o MSYS2 em "%MSYS2_ROOT%".
  echo Consulte Dependencias no README. Para outro local, defina DESTINIA_MSYS2_ROOT.
  pause
  exit /b 1
)
if not exist "%ROOT%scripts\compile_windows.sh" (
  echo Falta scripts\compile_windows.sh. Extraia todos os arquivos do pacote.
  pause
  exit /b 1
)
if not "%~1"=="" goto argumento
echo DESTINIA - COMPILAR
echo [1] Zoom host_zoom
echo [2] Host normal
echo [3] Ambos
choice /c 123 /n /m "Escolha 1, 2 ou 3: "
if errorlevel 4 exit /b 2
if errorlevel 3 goto ambos
if errorlevel 2 goto host
if errorlevel 1 goto zoom
exit /b 2
:zoom
set "TARGET=host_zoom"
goto compilar
:host
set "TARGET=host"
goto compilar
:ambos
set "TARGET=ambos"
goto compilar
:argumento
if "%~1"=="host_zoom" goto zoom
if "%~1"=="host" goto host
if "%~1"=="ambos" goto ambos
echo Opcao invalida. Use host_zoom, host ou ambos.
pause
exit /b 2
:compilar
set "MSYSTEM=UCRT64"
echo Feche o jogo antes de compilar. Dependencias devem estar instaladas conforme INSTALL.md.
"%MSYS2_ROOT%\usr\bin\bash.exe" --noprofile --norc -s -- "%ROOT%." "%TARGET%" < "%ROOT%scripts\compile_windows.sh"
set "RESULT=%ERRORLEVEL%"
echo.
if "%RESULT%"=="0" echo Pronto. Agora abra o launcher ou Testar Destinia.bat.
if not "%RESULT%"=="0" echo A compilacao falhou. Codigo: %RESULT%. Copie a mensagem de erro acima.
pause
exit /b %RESULT%
