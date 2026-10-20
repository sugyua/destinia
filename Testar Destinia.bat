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
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%diagnostico\test_suite.ps1" -Root "%ROOT%." %*
set "RESULT=%ERRORLEVEL%"
echo.
echo Codigo do diagnostico: %RESULT%
pause
exit /b %RESULT%
