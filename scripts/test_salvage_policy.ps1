$ErrorActionPreference = 'Stop'
$env:PATH = 'C:/msys64/ucrt64/bin;C:/msys64/usr/bin;' + $env:PATH
$project = Split-Path $PSScriptRoot -Parent
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('destinia-salvage-policy-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Push-Location $project
try {
  $exe = Join-Path $scratch 'salvage_policy_cases.exe'
  $compile = & g++ -std=c++17 tests/salvage_policy_cases.cpp -o $exe 2>&1
  if ($compile) { $compile | Write-Output }
  if ($LASTEXITCODE) { throw 'Compilacao da politica de Auto-Salvage falhou' }
  & $exe
  if ($LASTEXITCODE) { throw 'Politica de Auto-Salvage falhou' }
  & g++ -std=c++17 tests/inventory_panel_cases.cpp -o (Join-Path $scratch 'inventory_panel_cases.exe')
  if ($LASTEXITCODE) { throw 'Compilacao do painel de inventario falhou' }
  & (Join-Path $scratch 'inventory_panel_cases.exe')
  if ($LASTEXITCODE) { throw 'Painel de inventario falhou' }
} finally {
  Pop-Location
  if (Test-Path -LiteralPath $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force }
}
