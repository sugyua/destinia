$ErrorActionPreference = 'Stop'
$scriptPath = Join-Path $PSScriptRoot '../diagnostico/test_suite.ps1'
if (-not (Test-Path -LiteralPath $scriptPath)) {
    throw 'FALHA esperada: o executor de diagnostico ainda nao foi implementado.'
}
. $scriptPath
$temp = Join-Path ([IO.Path]::GetTempPath()) ('destinia-tests-' + [guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($temp)
$passed = 0
function Assert-True($Condition, $Message) {
    if (-not $Condition) { throw $Message }
}
function Expect-Failure([scriptblock]$Action, $Message) {
    $failed = $false
    try { & $Action } catch { $failed = $true }
    Assert-True $failed $Message
}
function Write-TestBmp($Path, $Width, $Height) {
    $bytes = New-Object byte[] (54 + $Width * $Height * 4)
    $bytes[0] = 66; $bytes[1] = 77
    [BitConverter]::GetBytes([int]$bytes.Length).CopyTo($bytes, 2)
    [BitConverter]::GetBytes([int]54).CopyTo($bytes, 10)
    [BitConverter]::GetBytes([int]40).CopyTo($bytes, 14)
    [BitConverter]::GetBytes([int]$Width).CopyTo($bytes, 18)
    [BitConverter]::GetBytes([int]$Height).CopyTo($bytes, 22)
    [BitConverter]::GetBytes([uint16]1).CopyTo($bytes, 26)
    [BitConverter]::GetBytes([uint16]32).CopyTo($bytes, 28)
    [IO.File]::WriteAllBytes($Path, $bytes)
}
try {
    $shopLog = "[host] cash local 0 -> 170000; resposta da loja nao simulada (spinner pode permanecer).`n[sonda] quadro 100: rede=1 menu.estado=2"
    $observations = @(Get-SessionObservations $shopLog)
    Assert-True (@($observations | Where-Object { $_.Teste -eq 'Compra observada' -and $_.Estado -eq 'AVISO' }).Count -eq 1) 'Compra com cash alterado nao foi destacada'
    Assert-True (@($observations | Where-Object { $_.Teste -eq 'Espera de rede' -and $_.Estado -eq 'AVISO' }).Count -eq 1) 'Rede pendente no fim do log nao foi destacada'
    $recovered = @(Get-SessionObservations ($shopLog + "`n[sonda] quadro 110: rede=0 menu.estado=2"))
    Assert-True (@($recovered | Where-Object { $_.Teste -eq 'Espera de rede' }).Count -eq 0) 'Espera que terminou foi marcada como ainda pendente'
    Assert-True (@(Get-SessionObservations '[host] initGame ok.').Count -eq 0) 'Compra ou rede inventada sem evidencia'
    $passed++; Write-Host 'OK: compra observada, espera final e recuperacao de rede'

    $reportDir = Join-Path $temp 'relatorio parcial'
    Initialize-TestReport $reportDir 'Guiado' 'host'
    Add-TestResult 'Inicializacao' 'OK' 'Resultado ja concluido'
    Set-TestStage 'Aguardando resposta do usuario'
    $partial = Get-Content -LiteralPath (Join-Path $reportDir 'resultados.json') -Raw | ConvertFrom-Json
    Assert-True (-not $partial.Concluido -and $null -eq $partial.Codigo) 'Sessao ainda em andamento foi marcada como concluida'
    Assert-True ($partial.Resultados.Count -eq 1 -and $partial.Etapa -eq 'Aguardando resposta do usuario') 'Resultado parcial nao foi persistido antes da pergunta'
    Add-TestResult 'Loja' 'AVISO' 'Operacao nao concluida'
    Save-TestReport -Complete
    $final = Get-Content -LiteralPath (Join-Path $reportDir 'resultados.json') -Raw | ConvertFrom-Json
    Assert-True ($final.Concluido -and $final.Avisos -eq 1 -and $final.Codigo -eq 0) 'Aviso ou estado final incorreto'
    Assert-True ((Get-Content -LiteralPath (Join-Path $reportDir 'relatorio.txt') -Raw) -match 'AVISO') 'Aviso nao consta no relatorio legivel'
    Add-TestResult 'Erro de execucao' 'FALHA' 'Falha controlada de teste'
    Save-TestReport -Complete
    $failed = Get-Content -LiteralPath (Join-Path $reportDir 'resultados.json') -Raw | ConvertFrom-Json
    Assert-True ($failed.Codigo -eq 1 -and $failed.Resultados.Count -eq 3) 'Falha nao atualizou o relatorio ou apagou os resultados anteriores'
    $script:ReportState = $null
    $passed++; Write-Host 'OK: resultados parciais duraveis, estado final, avisos e falhas'

    $source = Join-Path $temp 'save original'
    [void][IO.Directory]::CreateDirectory($source)
    [IO.File]::WriteAllText((Join-Path $source 'msave.dat'), 'progresso original')
    $before = @(Get-SaveInventory $source)
    $clone = Join-Path $temp 'copia isolada'
    Copy-IsolatedSave $source $clone
    Assert-True ((@(Get-SaveInventory $clone) | ConvertTo-Json -Compress) -eq ($before | ConvertTo-Json -Compress)) 'Copia difere da origem'
    [IO.File]::WriteAllText((Join-Path $clone 'msave.dat'), 'progresso de teste')
    Assert-True ([IO.File]::ReadAllText((Join-Path $source 'msave.dat')) -eq 'progresso original') 'Save original foi alterado'
    Expect-Failure { Copy-IsolatedSave $source $clone } 'Nao deveria sobrescrever clone existente'
    Expect-Failure { Copy-IsolatedSave $source (Join-Path $source 'teste-interno') } 'Copia dentro da origem nao deveria ser permitida'
    $empty = Join-Path $temp 'save novo'
    Copy-IsolatedSave '' $empty
    Assert-True ((Test-Path -LiteralPath $empty) -and @(Get-SaveInventory $empty).Count -eq 0) 'Save novo nao foi criado vazio'
    $passed++; Write-Host 'OK: copia isolada, preservacao e recusa de sobrescrita'

    $frameDir = Join-Path $temp 'capturas'
    [void][IO.Directory]::CreateDirectory($frameDir)
    Write-TestBmp (Join-Path $frameDir 'frame_00001.bmp') 400 240
    Write-TestBmp (Join-Path $frameDir 'frame_00010.bmp') 400 240
    $result = [pscustomobject]@{ ExitCode = 0; TimedOut = $false; Error = '' }
    $log = '[host] initGame ok. Rodando o jogo...' + "`n" + '[host] 10 quadros executados. Imagens em capturas'
    $verdict = Get-HeadlessVerdict $result $log $frameDir 400 240 10
    Assert-True $verdict.Passed 'Execucao completa com BMPs validos deveria passar'
    Assert-True (-not (Get-HeadlessVerdict $result ($log + "`n=== PROBLEMA: falha ARM ===") $frameDir 400 240 10).Passed) 'Erro com exit zero foi ignorado'
    Remove-Item -LiteralPath (Join-Path $frameDir 'frame_00010.bmp')
    Assert-True (-not (Get-HeadlessVerdict $result $log $frameDir 400 240 10).Passed) 'Falta de captura final foi ignorada'
    Write-TestBmp (Join-Path $frameDir 'frame_00010.bmp') 800 480
    Assert-True (-not (Get-HeadlessVerdict $result $log $frameDir 400 240 10).Passed) 'Dimensoes erradas foram ignoradas'
    Write-TestBmp (Join-Path $frameDir 'frame_00010.bmp') 400 240
    $bmpPath = Join-Path $frameDir 'frame_00010.bmp'
    $truncated = [IO.File]::ReadAllBytes($bmpPath)[0..53]
    [IO.File]::WriteAllBytes($bmpPath, $truncated)
    Assert-True (-not (Get-HeadlessVerdict $result $log $frameDir 400 240 10).Passed) 'BMP truncado foi aceito como captura completa'
    $passed++; Write-Host 'OK: sem falso sucesso por exit zero, frames ausentes ou dimensoes incorretas'

    $shell = (Get-Process -Id $PID).Path
    $fixture = Join-Path $temp 'processo teste.ps1'
    [IO.File]::WriteAllText($fixture, "[Console]::Out.WriteLine('saida capturada'); [Console]::Error.WriteLine('erro capturado'); exit 0")
    $proc = Invoke-OwnedProcess $shell @('-NoProfile', '-File', $fixture) $temp (Join-Path $temp 'normal') 15
    Assert-True ($proc.ExitCode -eq 0 -and -not $proc.TimedOut) 'Processo de teste falhou'
    Assert-True ((Get-Content -LiteralPath $proc.Stdout -Raw) -match 'saida capturada') 'stdout nao foi capturado'
    Assert-True ((Get-Content -LiteralPath $proc.Stderr -Raw) -match 'erro capturado') 'stderr nao foi capturado'
    [IO.File]::WriteAllText($fixture, 'Start-Sleep -Seconds 30')
    $proc = Invoke-OwnedProcess $shell @('-NoProfile', '-File', $fixture) $temp (Join-Path $temp 'timeout') 1
    Assert-True $proc.TimedOut 'Limite de tempo nao foi respeitado'
    Assert-True (-not (Get-Process -Id $proc.ProcessId -ErrorAction SilentlyContinue)) 'Processo criado pelo teste ficou rodando'
    $passed++; Write-Host 'OK: captura real de stdout/stderr e encerramento do proprio processo por timeout'

    $interruptedDir = Join-Path $temp 'sessao interrompida'
    $escapedScript = $scriptPath.Replace("'", "''")
    $escapedDirectory = $interruptedDir.Replace("'", "''")
    $fixtureText = ". '$escapedScript'`nInitialize-TestReport '$escapedDirectory' 'Guiado' 'host'`nAdd-TestResult 'Inicializacao' 'OK' 'Concluida antes da interrupcao'`nSet-TestStage 'Janela guiada em execucao'`nStart-Sleep -Seconds 30"
    [IO.File]::WriteAllText($fixture, $fixtureText)
    $interrupted = Invoke-OwnedProcess $shell @('-NoProfile', '-File', $fixture) $temp (Join-Path $temp 'interrompido') 5
    Assert-True $interrupted.TimedOut 'Sessao de teste deveria ter sido interrompida'
    $saved = Get-Content -LiteralPath (Join-Path $interruptedDir 'resultados.json') -Raw | ConvertFrom-Json
    Assert-True (-not $saved.Concluido -and $null -eq $saved.Codigo -and $saved.Resultados.Count -eq 1) 'Interrupcao apagou o resultado parcial ou anunciou sucesso'
    Assert-True ($saved.Etapa -eq 'Janela guiada em execucao') 'Etapa interrompida nao foi preservada'
    $passed++; Write-Host 'OK: relatorio parcial sobrevive ao encerramento do processo'

    Write-Host "$passed grupos de testes aprovados. Nenhum jogo foi executado."
} finally {
    Remove-Item -LiteralPath $temp -Recurse -Force
}
