$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../diagnostico/test_suite.ps1')
$script:responses=New-Object System.Collections.Queue
$script:readCount=0
function Read-Host($Prompt) {
    $script:readCount++
    if(-not $script:responses.Count) {return $null}
    return $script:responses.Dequeue()
}
foreach($value in @('','talvez',' s ')) {$script:responses.Enqueue($value)}
if(-not (Read-TestYesNo 'Abrir janela?') -or $script:readCount -ne 3) {throw 'Resposta vazia/invalida pulou o teste'}
$script:responses.Enqueue('n')
if(Read-TestYesNo 'Abrir janela?') {throw 'N explicito deveria recusar'}
$failed=$false
try{Read-TestYesNo 'Abrir janela?'}catch{$failed=$true}
if(-not $failed){throw 'Fim de entrada virou aprovacao/recusa silenciosa'}
Write-Host 'OK: resposta vazia/invalida pergunta novamente; S/N explicitos respeitados; fim de entrada tratado como erro.'
