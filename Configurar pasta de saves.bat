@echo off
setlocal DisableDelayedExpansion
set "SCRIPT=%~dp0scripts\configure_saves.ps1"
if not exist "%SCRIPT%" (
  echo Nao encontrei scripts\configure_saves.ps1. Extraia o pacote completo.
  pause
  exit /b 1
)
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%"
set "RESULT=%ERRORLEVEL%"
pause
exit /b %RESULT%
