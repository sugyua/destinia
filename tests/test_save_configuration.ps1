$ErrorActionPreference = 'Stop'
$helper = Join-Path $PSScriptRoot '../scripts/shared_saves.ps1'
if (-not (Test-Path -LiteralPath $helper)) { throw 'Configuracao compartilhada ainda nao implementada.' }
. $helper
$temp = Join-Path ([IO.Path]::GetTempPath()) ('destinia-save-config-' + [guid]::NewGuid().ToString('N'))
$oldOverride = $env:DESTINIA_SAVE_DIR
try {
    $env:DESTINIA_SAVE_DIR = ''
    $local = Join-Path $temp 'dados locais'
    $default = Join-Path $local 'Destinia/save'
    if ((Get-DestiniaSaveDirectory $local) -ne $default) { throw 'Pasta padrao inesperada.' }
    $chosen = Join-Path $temp 'partida anterior com espacos'
    [void][IO.Directory]::CreateDirectory($chosen)
    [IO.File]::WriteAllText((Join-Path $chosen 'msave.dat'), 'progresso original')
    Set-DestiniaSaveDirectory $chosen $local
    if ((Get-DestiniaSaveDirectory $local) -ne $chosen) { throw 'Configuracao nao persistida.' }
    if ([IO.File]::ReadAllText((Join-Path $chosen 'msave.dat')) -ne 'progresso original') { throw 'Configurador alterou o save.' }
    $other = Join-Path $temp 'outra pasta'
    $env:DESTINIA_SAVE_DIR = $other
    if ((Get-DestiniaSaveDirectory $local) -ne $other) { throw 'Variavel de ambiente nao tem precedencia.' }
    $env:DESTINIA_SAVE_DIR = ''
    Set-DestiniaSaveDirectory $default $local
    if (-not (Test-Path -LiteralPath (Join-Path $local 'Destinia/config.ini.previous'))) { throw 'Configuracao anterior nao foi preservada.' }
    if ([IO.File]::ReadAllText((Join-Path $chosen 'msave.dat')) -ne 'progresso original') { throw 'Troca de pasta apagou o progresso anterior.' }
    $failed = $false
    try { Set-DestiniaSaveDirectory 'pasta relativa' $local } catch { $failed = $true }
    if (-not $failed) { throw 'Pasta relativa aceita.' }
    [IO.File]::WriteAllText((Join-Path $local 'Destinia/config.ini'), 'save_dir=')
    $failed = $false
    try { Get-DestiniaSaveDirectory $local } catch { $failed = $true }
    if (-not $failed) { throw 'Configuracao invalida trocou silenciosamente o save.' }
    Write-Host 'OK: pasta padrao, configuracao global, espacos, precedencia, configuracao anterior e progresso preservados.'
} finally {
    $env:DESTINIA_SAVE_DIR = $oldOverride
    Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue
}
