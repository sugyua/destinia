[CmdletBinding()]
param(
    [string]$NormalPath='',
    [string]$ZoomPath=''
)
$ErrorActionPreference='Stop'
# Windows PowerShell 5.1 has no PSScriptRoot while binding default parameters.
if(-not $NormalPath) {$NormalPath=Join-Path $PSScriptRoot '../host/build/destinia_host.exe'}
if(-not $ZoomPath) {$ZoomPath=Join-Path $PSScriptRoot '../host_zoom/build/destinia_host.exe'}
$temporary=Join-Path ([IO.Path]::GetTempPath()) ('destinia config nativo '+[guid]::NewGuid().ToString('N'))
$previousLocal=$env:LOCALAPPDATA
try {
    $root=Join-Path $temporary 'instalacao'
    [void][IO.Directory]::CreateDirectory((Join-Path $root 'scripts'))
    foreach($name in @('configure_game.ps1','game_preferences.ps1')) {Copy-Item -LiteralPath (Join-Path $PSScriptRoot "../scripts/$name") -Destination (Join-Path $root 'scripts')}
    foreach($entry in @(@('host',$NormalPath),@('host_zoom',$ZoomPath))) {
        $build=Join-Path $root ($entry[0]+'/build');[void][IO.Directory]::CreateDirectory($build)
        Copy-Item -LiteralPath $entry[1] -Destination (Join-Path $build 'destinia_host.exe')
    }
    $env:LOCALAPPDATA=Join-Path $temporary 'dados locais'
    $data=Join-Path $env:LOCALAPPDATA 'Destinia';[void][IO.Directory]::CreateDirectory($data)
    $config=Join-Path $data 'config.ini'
    $save=Join-Path $temporary 'partida';[void][IO.Directory]::CreateDirectory($save)
    $marker=Join-Path $save 'personagem.bin';[IO.File]::WriteAllText($marker,'progresso intocado')
    [IO.File]::WriteAllText($config,('save_dir='+$save+"`nchave_futura=preservada`n"))
    $inputFile=Join-Path $temporary 'entrada.txt'
    [IO.File]::WriteAllText($inputFile,"2`n35`n70`n1`n3`n1`n1`n4`nmeu_controle`nataque`ntecla`nF8`n6`n2`n0`n0`n",(New-Object Text.UTF8Encoding -ArgumentList $false))
    $shell=Join-Path $PSHOME 'powershell.exe';if(-not (Test-Path -LiteralPath $shell)) {$shell=Join-Path $PSHOME 'pwsh'}
    $script=Join-Path $root 'scripts/configure_game.ps1'
    $output=Join-Path $temporary 'saida.txt';$errors=Join-Path $temporary 'erros.txt'
    $process=Start-Process -FilePath $shell -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$script+'"')) -RedirectStandardInput $inputFile -RedirectStandardOutput $output -RedirectStandardError $errors -Wait -PassThru
    if($process.ExitCode -ne 0) {throw ([IO.File]::ReadAllText($output)+[IO.File]::ReadAllText($errors))}
    $text=[IO.File]::ReadAllText($config)
    foreach($line in @(('save_dir='+$save),'chave_futura=preservada','audio.musica=35','interface.host_zoom.controles_virtuais=0','audio.efeitos=70','audio.silenciar_sem_foco=1','pausa.sem_foco=1','pausa.sem_controle=1','controle_perfil=meu_controle','perfil.meu_controle.tecla.ataque=F8')) {
        if(-not $text.Contains($line)) {throw ('A interface nao guardou '+$line+"`n"+[IO.File]::ReadAllText($output))}
    }
    if([IO.File]::ReadAllText($marker) -cne 'progresso intocado') {throw 'O configurador alterou progresso'}
    Write-Host 'OK: menu real, volumes, pausas e perfil validados pelos dois EXEs; save preservado.'
} finally {
    $env:LOCALAPPDATA=$previousLocal
    if(Test-Path -LiteralPath $temporary) {Remove-Item -LiteralPath $temporary -Recurse -Force}
}
