# Integra o fluxo real com processo/janela e respostas simulados; nao executa o jogo.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '../diagnostico/test_suite.ps1')
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('destinia-camera-' + [guid]::NewGuid().ToString('N'))
$oldOS = $env:OS
try {
    $Root = $fixture; $Mode = 'Camera'; $Variant = 'Ambos'; $RuntimeDir = $fixture
    $SaveSource = Join-Path $fixture 'original'
    foreach ($dir in @('original', 'game/lib/armeabi', 'game/assets', 'game/res/raw', 'host_zoom/build')) {
        [void][IO.Directory]::CreateDirectory((Join-Path $fixture $dir))
    }
    [IO.File]::WriteAllText((Join-Path $SaveSource 'msave.dat'), 'original intacto')
    [IO.File]::WriteAllText((Join-Path $fixture 'game/assets/data'), 'fixture')
    [IO.File]::WriteAllText((Join-Path $fixture 'host_zoom/build/destinia_host.exe'), 'fixture')
    $elf = New-Object byte[] 52
    $elf[0]=127; $elf[1]=69; $elf[2]=76; $elf[3]=70; $elf[4]=1; $elf[5]=1; $elf[18]=40
    [IO.File]::WriteAllBytes((Join-Path $fixture 'game/lib/armeabi/libdestinia_jni.so'), $elf)
    $env:OS = 'Windows_NT'
    $script:launches = 0; $script:answers = 0; $script:oldBinary = $false
    function Get-Process { return }
    function Read-Host($Prompt) {
        $script:answers++
        if ($Prompt -like 'Abrir teste guiado*') { return 'S' }
        if ($Prompt -like 'A troca de mapa*') { return 'N' }
        if ($Prompt -like 'UI menor*') { return 'S' }
        if ($Prompt -like 'Descreva o defeito*') { return 'Cenario duplicado na passagem de teste' }
        throw ('Pergunta fora do escopo: ' + $Prompt)
    }
    function Get-HeadlessVerdict { return [pscustomobject]@{ Passed=$true; Detail='Inicializacao simulada' } }
    function Invoke-OwnedProcess($Exe, $Arguments, $Directory, $Prefix, $Timeout) {
        $script:launches++
        if ($Exe -notlike '*host_zoom*') { throw 'Host normal nao deveria ser exigido' }
        if ($Arguments -notcontains '--headless') {
            if ($Arguments -notcontains '--trace-camera' -or $Arguments -notcontains '--fullscreen') { throw 'Falta log de camera' }
            $savePath = $Arguments[[array]::IndexOf($Arguments, '--save') + 1]
            if ($savePath -eq $SaveSource) { throw 'Tentativa de usar save original' }
            [IO.File]::WriteAllText((Join-Path $savePath 'msave.dat'), 'progresso de teste')
            # Sem F9: o relatorio deve manter a evidencia visual pendente.
        }
        [IO.File]::WriteAllText(($Prefix + '.stdout.log'), '')
        $processLog = '[host] initGame ok em 800x480. mapas pequenos centralizados [camera] quadro 60 camX=0 camY=0 mapa=30x30'
        if ($script:oldBinary) { $processLog = '[host] initGame ok em 400x240.' }
        [IO.File]::WriteAllText(($Prefix + '.stderr.log'), $processLog)
        return [pscustomobject]@{Stdout=($Prefix+'.stdout.log'); Stderr=($Prefix+'.stderr.log'); Error=$null; TimedOut=$false; ExitCode=0}
    }
    $code = Invoke-DestiniaSuite
    $report = Get-ChildItem -LiteralPath (Join-Path $fixture 'relatorios') -Recurse -Filter resultados.json | Select-Object -First 1
    $result = Get-Content -LiteralPath $report.FullName -Raw | ConvertFrom-Json
    if ($code -ne 1 -or $result.Falhas -ne 1 -or $result.Variante -ne 'host_zoom' -or -not $result.Concluido) { throw 'Falha manual ou variante nao registrada corretamente' }
    if ($script:launches -ne 2 -or $script:answers -ne 4) { throw 'Teste repetiu janelas ou perguntas gerais' }
    if (@($result.Resultados | Where-Object { $_.Teste -eq 'Capturas F9' -and $_.Estado -eq 'PENDENTE' }).Count -ne 1) { throw 'Ausencia de capturas nao foi destacada' }
    if ((Get-Content -LiteralPath (Join-Path $SaveSource 'msave.dat') -Raw) -ne 'original intacto') { throw 'Save original alterado' }
    $script:oldBinary = $true; $script:launches = 0; $script:answers = 0
    $oldCode = Invoke-DestiniaSuite
    $oldResult = Get-Content -LiteralPath (Join-Path $script:ReportState.Directory 'resultados.json') -Raw | ConvertFrom-Json
    if ($oldCode -ne 1 -or $script:launches -ne 1 -or $script:answers -ne 0) { throw 'EXE antigo nao bloqueou a janela guiada' }
    if (@($oldResult.Resultados | Where-Object { $_.Teste -eq 'host_zoom versao antiga' -and $_.Estado -eq 'FALHA' }).Count -ne 1) { throw 'Instrucao de recompilacao ausente para EXE antigo' }
    Write-Host 'OK: EXE antigo identificado sem abrir janela nem perguntar sobre gameplay.'
    Write-Host 'OK: fluxo Camera, falha manual, capturas pendentes, apenas zoom, sem reabertura e save preservado.'
} finally {
    $env:OS = $oldOS
    Remove-Item -LiteralPath $fixture -Recurse -Force
}
