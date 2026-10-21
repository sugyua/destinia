@echo off
setlocal DisableDelayedExpansion
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\configure_game.ps1"
set "RESULT=%ERRORLEVEL%"
pause
exit /b %RESULT%
