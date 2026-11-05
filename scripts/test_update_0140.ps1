$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$msys = if ($env:DESTINIA_MSYS2_ROOT) { $env:DESTINIA_MSYS2_ROOT } else { 'C:/msys64' }
$env:PATH = "$msys/ucrt64/bin;$msys/usr/bin;" + $env:PATH
$env:PKG_CONFIG_PATH = "$msys/ucrt64/lib/pkgconfig"
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('destinia-0140-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Push-Location $project
try {
  $testExe = Join-Path $scratch 'diagnostics_panel_cases.exe'
  & g++ -std=c++17 -Wall -Wextra tests/diagnostics_panel_cases.cpp -o $testExe
  if ($LASTEXITCODE) { throw 'Compilação dos testes do painel falhou' }
  & $testExe
  if ($LASTEXITCODE) { throw 'Testes do painel falharam' }
  foreach ($variant in @('host','host_zoom')) {
    $build = Join-Path $scratch $variant
    & cmake -S $variant -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE) { throw "Configuração CMake falhou: $variant" }
    & cmake --build $build --parallel 2
    if ($LASTEXITCODE) { throw "Compilação falhou: $variant" }
  }
  foreach ($source in @('host/main.cpp','host_zoom/main.cpp')) {
    if (-not (Select-String -Path $source -Pattern 'SDLK_F12' -Quiet) -or -not (Select-String -Path $source -Pattern 'diagnosticsPanel.paint' -Quiet)) {
      throw "Integração de painéis incompleta: $source"
    }
  }
  Write-Output "UPDATE_0140_CHECKS_OK: $scratch"
} catch {
  Write-Error $_
  exit 1
} finally {
  Pop-Location
  $resolvedScratch = [IO.Path]::GetFullPath($scratch)
  $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
  if ($resolvedScratch.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and (Test-Path -LiteralPath $resolvedScratch)) {
    Remove-Item -LiteralPath $resolvedScratch -Recurse -Force
  }
}
