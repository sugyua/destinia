[CmdletBinding()]
param([string]$Directory = '', [switch]$UseDefault)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'shared_saves.ps1')
try {
    if (-not $env:LOCALAPPDATA) { throw 'Nao encontrei LOCALAPPDATA para salvar a configuracao.' }
    $default = Join-Path $env:LOCALAPPDATA 'Destinia/save'
    if ($UseDefault) { $Directory = $default }
    if (-not $Directory) {
        Write-Host 'DESTINIA - PASTA COMPARTILHADA DE SAVES'
        try { Write-Host ('Pasta atual: ' + (Get-DestiniaSaveDirectory)) } catch { Write-Host $_.Exception.Message }
        Write-Host ('Padrao: ' + $default)
        Write-Host 'Feche todas as versoes do jogo antes de trocar a configuracao.'
        Write-Host 'Escolha a pasta que ja contem seu progresso ou Enter para usar o padrao.'
        Write-Host 'Nenhum save sera copiado, apagado ou sobrescrito por este configurador.'
        $Directory = (Read-Host 'Caminho completo da pasta').Trim().Trim('"')
        if (-not $Directory) { $Directory = $default }
    }
    Set-DestiniaSaveDirectory $Directory
    Write-Host ('Configuracao salva em: ' + (Join-Path $env:LOCALAPPDATA 'Destinia/config.ini'))
    Write-Host ('Pasta selecionada: ' + [IO.Path]::GetFullPath($Directory))
    if ($env:DESTINIA_SAVE_DIR) { Write-Host 'ATENCAO: DESTINIA_SAVE_DIR esta definida e tem prioridade sobre config.ini.' }
    Write-Host 'Os hosts 0.5 e futuros usarao essa pasta ao abrir pelos inicializadores normais.'
    Write-Host 'Versoes 0.2 a 0.4 usam a pasta padrao; mantenha o padrao se quiser compartilhar tambem com elas.'
    exit 0
} catch {
    Write-Host ('FALHA: ' + $_.Exception.Message)
    exit 1
}
