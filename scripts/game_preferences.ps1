# PowerShell 5.1+. Only config.ini is changed; progress files are never opened.
function Read-DestiniaPreferences([string]$Config) {
    $values=New-Object 'System.Collections.Specialized.OrderedDictionary' ([StringComparer]::Ordinal)
    if(Test-Path -LiteralPath $Config) {
        foreach($raw in [IO.File]::ReadAllLines($Config)) {
            $line=$raw.Trim().TrimStart([char]0xFEFF)
            if(-not $line -or $line.StartsWith('#') -or $line.StartsWith(';')) {continue}
            $equal=$line.IndexOf('=')
            if($equal -lt 1) {throw 'Linha invalida em config.ini.'}
            $key=$line.Substring(0,$equal).Trim()
            if($values.Contains($key)) {throw ('Chave repetida: '+$key)}
            $values.Add($key,$line.Substring($equal+1).Trim())
        }
    }
    return ,$values
}
function Set-DestiniaPreferences([string]$Config,[hashtable]$Changes,[scriptblock]$Validate,[switch]$Reset) {
    if(-not $Validate) {throw 'O validador do jogo e obrigatorio para guardar preferencias.'}
    $directory=[IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($Config))
    [void][IO.Directory]::CreateDirectory($directory)
    $lock=$null;$temporary=$Config+'.'+[guid]::NewGuid().ToString('N')+'.tmp'
    try {
        $lock=[IO.File]::Open((Join-Path $directory 'config.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
        $values=Read-DestiniaPreferences $Config
        if($Reset) {
            foreach($key in @($values.Keys)) {
                if($key -cmatch '^(idioma|jogo\.modo|video\.(host|host_zoom)\.(largura|altura|tela_cheia)|audio\.(musica|efeitos|silenciar_sem_foco)|pausa\.(sem_foco|sem_controle)|interface\.(host|host_zoom)\.(controles_virtuais|escala)|controle_perfil|perfil\..*\.(tecla|botao)\..*)$') {$values.Remove($key)}
            }
        }
        foreach($key in $Changes.Keys) {
            $value=[string]$Changes[$key]
            if($key.Contains("`n") -or $key.Contains("`r") -or $key.Contains('=') -or $value.Contains("`n") -or $value.Contains("`r")) {throw 'Chave ou valor com quebra de linha.'}
            $values[$key]=$value.Trim()
        }
        $lines=@(foreach($key in $values.Keys) {$key+'='+$values[$key]})
        [IO.File]::WriteAllText($temporary,($lines -join "`r`n")+"`r`n",(New-Object Text.UTF8Encoding -ArgumentList $false))
        & $Validate $temporary
        if(Test-Path -LiteralPath $Config) {[IO.File]::Replace($temporary,$Config,($Config+'.previous'))}
        else {[IO.File]::Move($temporary,$Config)}
    } finally {
        if(Test-Path -LiteralPath $temporary) {Remove-Item -LiteralPath $temporary -Force}
        if($lock) {$lock.Dispose()}
    }
}
function Reset-DestiniaPreferences([string]$Config,[scriptblock]$Validate) {Set-DestiniaPreferences $Config @{} $Validate -Reset}
