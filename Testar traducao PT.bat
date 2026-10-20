@echo off
setlocal

set "ROOT=%~dp0"
set "HOST_DIR=%ROOT%host_zoom"
set "EXE=%HOST_DIR%\build\destinia_host.exe"
set "MSYS2_BIN=C:\msys64\ucrt64\bin"
set "SOURCE_SAVE=%HOST_DIR%\save_zoom800"
set "TEST_SAVE=%HOST_DIR%\save_translation_pt"

if not exist "%EXE%" (
  echo Nao encontrei o executavel do teste de traducao:
  echo   "%EXE%"
  echo Compile a pasta host_zoom pelo MSYS2 UCRT64.
  pause
  exit /b 1
)

if not exist "%MSYS2_BIN%\SDL2.dll" (
  echo Nao encontrei as DLLs do MSYS2 UCRT64 em:
  echo   "%MSYS2_BIN%"
  pause
  exit /b 1
)

if not exist "%SOURCE_SAVE%\msave.dat" (
  echo Nao encontrei o save principal:
  echo   "%SOURCE_SAVE%\msave.dat"
  echo Abra o jogo normalmente e tente novamente.
  pause
  exit /b 1
)

if not exist "%TEST_SAVE%" (
  echo Criando uma copia isolada do save para o teste de traducao.
  robocopy "%SOURCE_SAVE%" "%TEST_SAVE%" /E /COPY:DAT /R:1 /W:1 >nul
  if errorlevel 8 (
    echo Falha ao copiar o save para a pasta de teste.
    pause
    exit /b 1
  )
) else (
  echo Usando o save isolado ja existente em:
  echo   "%TEST_SAVE%"
)

set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%HOST_DIR%"

echo Teste de traducao em portugues: itens, equipamentos, habilidades, pausa, Fabricacao e Missoes.
echo A pasta game original sera somente lida; o save de teste e separado.
echo.
"%EXE%" --save save_translation_pt --lang pt
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" echo O teste terminou com codigo de erro %RESULT%.
pause
exit /b %RESULT%
