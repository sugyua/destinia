param([switch]$SkipNative)
$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$msys = if ($env:DESTINIA_MSYS2_ROOT) { $env:DESTINIA_MSYS2_ROOT } else { 'C:/msys64' }
$env:PATH = "$msys/ucrt64/bin;$msys/usr/bin;" + $env:PATH
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('destinia-0130-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Push-Location $project
try {
  foreach ($test in @('refinement_policy_0130','refinement_0130_cases','survival_0130_cases','survival_state_cases')) {
    $executable = Join-Path $scratch ($test + '.exe')
    & g++ -std=c++17 -Wall -Wextra "tests/$test.cpp" -o $executable
    if ($LASTEXITCODE) { throw "Compilacao falhou: $test" }
    & $executable
    if ($LASTEXITCODE) { throw "Teste falhou: $test" }
  }
  $storeExe = Join-Path $scratch 'survival_store_cases.exe'
  & g++ -std=c++17 -Wall -Wextra tests/survival_store_cases.cpp -o $storeExe
  if ($LASTEXITCODE) { throw 'Compilacao falhou: survival_store_cases' }
  & $storeExe (Join-Path $scratch 'store')
  if ($LASTEXITCODE) { throw 'Teste falhou: survival_store_cases' }
  & node --test web/tests/features-0130.test.mjs web/tests/update-0130-contract.test.mjs web/tests/refinement-policy-0130.test.mjs web/tests/features-storage-0130.test.mjs
  if ($LASTEXITCODE) { throw 'Testes Node falharam' }
  if (!$SkipNative) {
    $env:SDL_VIDEODRIVER = 'dummy'
    $env:SDL_AUDIODRIVER = 'dummy'
    foreach ($variant in @('host','host_zoom')) {
      $case = Join-Path $scratch $variant
      New-Item -ItemType Directory -Path $case | Out-Null
      $executable = Join-Path $case 'update0130.exe'
      $options = @('-std=c++17','-O0',"-I$msys/ucrt64/include/SDL2",'tests/update_0130_native.cpp','-o',$executable,'-lSDL2','-lunicorn','-lz','-lwinmm','-limm32','-lole32','-loleaut32','-lversion','-lsetupapi','-lgdi32')
      if ($variant -eq 'host_zoom') { $options += '-DDESTINIA_TEST_ZOOM' }
      & g++ @options
      if ($LASTEXITCODE) { throw "Compilacao ARM falhou: $variant" }
      & $executable --game game --save "$case/save" --settings "$case/config.ini" --reports "$case/reports" --headless 1 --mute --no-save-preferences --out "$case/frames"
      if ($LASTEXITCODE) { throw "Teste ARM falhou: $variant" }
    }
  }
  Write-Output "UPDATE_0130_CHECKS_OK: $scratch"
} catch {
  Write-Error $_
  exit 1
} finally { Pop-Location }
