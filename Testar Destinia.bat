@echo off
setlocal DisableDelayedExpansion
set "ROOT=%~dp0"
where powershell.exe >nul 2>&1
if errorlevel 1 (
  echo Nao encontrei o Windows PowerShell. Consulte as dependencias no README.
  pause
  exit /b 1
)
if not exist "%ROOT%diagnostico\test_suite.ps1" (
  echo Falta diagnostico\test_suite.ps1. Extraia todos os arquivos do pacote de testes.
  pause
  exit /b 1
)
if not "%~1"=="" goto argumentos
echo DESTINIA - ESCOLHA O TESTE
echo [1] Camera host_zoom - novo teste focado
echo [2] Roteiro geral dos dois hosts
choice /c 12 /n /m "Escolha 1 ou 2: "
if errorlevel 3 exit /b 2
if errorlevel 2 goto geral
if errorlevel 1 goto camera
exit /b 2
:camera
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%diagnostico\test_suite.ps1" -Root "%ROOT%." -Mode Camera
goto resultado
:geral
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%diagnostico\test_suite.ps1" -Root "%ROOT%."
goto resultado
:argumentos
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%diagnostico\test_suite.ps1" -Root "%ROOT%." %*
:resultado
set "RESULT=%ERRORLEVEL%"
echo.
echo Codigo do diagnostico: %RESULT%
pause
exit /b %RESULT%
