@echo off
setlocal
rem ===========================================================================
rem  DIAGNOSTICAR CALOR - Destinia PC Port
rem  Abre o jogo com o profiler ligado, mede o uso real de CPU da CPU do
rem  processo em segundo plano e, quando voce fecha o jogo, gera um relatorio
rem  explicando o que estava acontecendo (para achar a causa do aquecimento).
rem
rem  COMO USAR:
rem    1. Dê dois cliques neste arquivo.
rem    2. Jogue normalmente por um tempo (quanto mais tempo, melhor o dados).
rem    3. Feche a janela do jogo.
rem    4. O relatorio abre sozinho no Bloco de Notas.
rem ===========================================================================
set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host"
set "EXE=%HOST_DIR%\build\destinia_host.exe"
set "DIAG=%ROOT%diagnostico"
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
  echo Compile o projeto pelo MSYS2 UCRT64 antes de usar este diagnostico.
  pause
  exit /b 1
)
if not exist "%MSYS2_BIN%\SDL2.dll" (
  echo Nao encontrei as DLLs do MSYS2 UCRT64 em:
  echo   "%MSYS2_BIN%"
  pause
  exit /b 1
)

rem --- pasta do relatorio com timestamp ---
for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd_HHmmss"') do set "TS=%%i"
set "REPORT_DIR=%ROOT%relatorios\calor_%TS%"
set "LOG=%REPORT_DIR%\perfil_bruto.log"
set "CPU=%REPORT_DIR%\cpu_usage.csv"
set "REPORT=%REPORT_DIR%\relatorio_calor.txt"
mkdir "%REPORT_DIR%" >nul 2>&1

set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%HOST_DIR%"

echo ==========================================================
echo   DIAGNOSTICO DE CALOR - DESTINIA
echo ==========================================================
echo Janela de saida: "%REPORT_DIR%"
echo.
echo Instrucoes:
echo   - Jogue por um tempo (explore, entre em combate, abra menus).
echo   - Quando terminar, FECHE a janela do jogo.
echo   - O relatorio abre automaticamente no Bloco de Notas.
echo.
echo Iniciando medicao de CPU em segundo plano...

rem --- inicia o amostrador de CPU em background (fecha sozinho apos o jogo) ---
start "" /b powershell -NoProfile -ExecutionPolicy Bypass -File "%DIAG%\sample_cpu.ps1" -ProcessName destinia_host -OutCsv "%CPU%" >nul 2>&1

rem --- abre o jogo com o profiler ligado; redireciona tudo para o log ---
echo Rodando o jogo com profiler (--profile). Feche a janela para encerrar.
"%EXE%" --save "%SHARED_SAVE%" --profile >> "%LOG%" 2>&1

rem --- espera o amostrador encerrar e gravar o CSV ---
timeout /t 4 /nobreak >nul

rem --- monta o relatorio ---
powershell -NoProfile -ExecutionPolicy Bypass -File "%DIAG%\build_report.ps1" -Log "%LOG%" -Cpu "%CPU%" -Out "%REPORT%"

echo.
if exist "%REPORT%" (
  echo Relatorio gerado:
  echo   "%REPORT%"
  start "" notepad "%REPORT%"
) else (
  echo Nao consegui gerar o relatorio. Veja o log bruto em:
  echo   "%LOG%"
)
pause
exit /b 0
