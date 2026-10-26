$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../scripts/game_preferences.ps1')
$root=Join-Path ([IO.Path]::GetTempPath()) ('destinia prefs '+[guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($root)
try {
    $config=Join-Path $root 'config.ini'
    [IO.File]::WriteAllText($config,"save_dir=C:\Partidas`nchave_futura=preservar`n")
    $validate={param($file) if([IO.File]::ReadAllText($file).Contains('audio.musica=999')) { throw 'Volume invalido' }}
    Set-DestiniaPreferences $config @{'drops.filtro_visual'='1';'drops.categorias'='6';'drops.rank_minimo'='2';'coleta.ouro'='1';'coleta.materiais'='1';'coleta.raio'='12';'jogo.modo'='expandido';'interface.host_zoom.controles_virtuais'='0';'audio.musica'='35';'controle_perfil'='meu_controle';'perfil.meu_controle.tecla.ataque'='Return,Z'} $validate
    $values=Read-DestiniaPreferences $config
    if($values['save_dir'] -cne 'C:\Partidas' -or $values['chave_futura'] -cne 'preservar' -or $values['audio.musica'] -cne '35') {throw 'Configuracao nao preservada'}
    $before=[IO.File]::ReadAllText($config); $rejected=$false
    try {Set-DestiniaPreferences $config @{'audio.musica'='999'} $validate} catch {$rejected=$true}
    if(-not $rejected -or [IO.File]::ReadAllText($config) -cne $before) {throw 'Configuracao invalida sobrescreveu original'}
    Reset-DestiniaPreferences $config $validate
    $values=Read-DestiniaPreferences $config
    if($values.Contains('jogo.modo') -or $values.Contains('interface.host_zoom.controles_virtuais') -or $values.Contains('audio.musica') -or $values.Contains('controle_perfil') -or $values.Contains('perfil.meu_controle.tecla.ataque')) {throw 'Reset incompleto'}
    foreach($key in @('drops.filtro_visual','drops.categorias','drops.rank_minimo','coleta.ouro','coleta.materiais','coleta.raio')) {if($values.Contains($key)) {throw 'Reset QoL incompleto'}}
    if($values['save_dir'] -cne 'C:\Partidas' -or $values['chave_futura'] -cne 'preservar') {throw 'Reset atingiu save ou chave desconhecida'}
    [IO.File]::WriteAllText($config,"audio.musica=35`naudio.musica=70`n")
    $rejected=$false
    try {Read-DestiniaPreferences $config} catch {$rejected=$true}
    if(-not $rejected) {throw 'Chave duplicada aceita'}
    Write-Host 'OK: mesclagem, validacao antes de substituir, reset sem tocar save e chaves duplicadas.'
} finally {Remove-Item -LiteralPath $root -Recurse -Force}
