$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$msys = if ($env:DESTINIA_MSYS2_ROOT) { $env:DESTINIA_MSYS2_ROOT } else { 'C:/msys64' }
$env:PATH = "$msys/ucrt64/bin;$msys/usr/bin;" + $env:PATH
$env:SDL_VIDEODRIVER = 'dummy'
$env:SDL_AUDIODRIVER = 'dummy'
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('destinia-salvage-zoom-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Push-Location $project
try {
  $exe = Join-Path $scratch 'inventory_view_native.exe'
  $options = @('-std=c++17','-O0',"-I$msys/ucrt64/include/SDL2",'tests/inventory_view_native.cpp','-o',$exe,'-lSDL2','-lunicorn','-lz','-lwinmm','-limm32','-lole32','-loleaut32','-lversion','-lsetupapi','-lgdi32','-DDESTINIA_TEST_ZOOM')
  & g++ @options
  if ($LASTEXITCODE) { throw 'Compile failed' }
  & $exe --game game --save (Join-Path $scratch 'save') --settings (Join-Path $scratch 'config.ini') --reports (Join-Path $scratch 'reports') --headless 1 --mute --no-save-preferences --out (Join-Path $scratch 'frames')
  if ($LASTEXITCODE) { throw 'ARM inventory/salvage flow failed' }
} finally {
  Pop-Location
  $resolvedScratch = [IO.Path]::GetFullPath($scratch)
  $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
  if ($resolvedScratch.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and (Test-Path -LiteralPath $resolvedScratch)) {
    Remove-Item -LiteralPath $resolvedScratch -Recurse -Force
  }
}
