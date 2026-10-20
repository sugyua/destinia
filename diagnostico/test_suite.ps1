# Diagnostico externo: nao altera fontes, biblioteca ARM ou saves originais.
[CmdletBinding()]
param(
    [string]$Root = (Join-Path $PSScriptRoot '..'),
    [ValidateSet('Guiado', 'Automatico', 'Camera')][string]$Mode = 'Guiado',
    [ValidateSet('Ambos', 'host', 'host_zoom')][string]$Variant = 'Ambos',
    [string]$RuntimeDir = 'C:\msys64\ucrt64\bin',
    [string]$SaveSource = '',
    [switch]$Fresh,
    [ValidateRange(10, 1000)][int]$Frames = 60,
    [ValidateRange(1, 600)][int]$TimeoutSeconds = 120,
    [ValidateRange(30, 3600)][int]$InteractiveTimeoutSeconds = 900
)

function Get-SaveInventory([string]$Directory) {
    if (-not $Directory -or -not (Test-Path -LiteralPath $Directory -PathType Container)) { return }
    $base = [IO.Path]::GetFullPath($Directory).TrimEnd([IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
    foreach ($file in (Get-ChildItem -LiteralPath $base -Recurse -File -Force | Sort-Object FullName)) {
        [pscustomobject]@{ File = $file.FullName.Substring($base.Length + 1); Hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash }
    }
}

function Copy-IsolatedSave([string]$Source, [string]$Destination) {
    if (Test-Path -LiteralPath $Destination) { throw 'A pasta de teste ja existe; nao sera sobrescrita.' }
    if ($Source) {
        if (-not (Test-Path -LiteralPath $Source -PathType Container)) { throw 'A pasta de save indicada nao existe.' }
        $sourcePath = [IO.Path]::GetFullPath($Source).TrimEnd([IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
        $destinationPath = [IO.Path]::GetFullPath($Destination)
        if ($destinationPath.Equals($sourcePath, [StringComparison]::OrdinalIgnoreCase) -or
            $destinationPath.StartsWith($sourcePath + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
            throw 'A copia de teste nao pode ficar dentro do save original.'
        }
        $entries = @((Get-Item -LiteralPath $Source -Force)) + @(Get-ChildItem -LiteralPath $Source -Recurse -Force)
        if (@($entries | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) {
            throw 'Save contem link/junction; copia recusada para preservar o isolamento.'
        }
    }
    [void][IO.Directory]::CreateDirectory($Destination)
    if ($Source) {
        foreach ($item in (Get-ChildItem -LiteralPath $Source -Force)) {
            Copy-Item -LiteralPath $item.FullName -Destination $Destination -Recurse -Force -ErrorAction Stop
        }
        $before = @(Get-SaveInventory $Source) | ConvertTo-Json -Compress
        $copied = @(Get-SaveInventory $Destination) | ConvertTo-Json -Compress
        if ($before -ne $copied) { throw 'A copia do save nao corresponde a origem; teste cancelado.' }
    }
}

function ConvertTo-NativeArgument([string]$Value) {
    if ($Value.Contains('"') -or $Value.Contains("`n") -or $Value.Contains("`r")) { throw 'Argumento contem caracteres nao permitidos.' }
    return '"' + ($Value -replace '(\\+)$', '$1$1') + '"'
}

function Invoke-OwnedProcess([string]$Executable, [string[]]$Arguments, [string]$WorkingDirectory, [string]$LogPrefix, [int]$LimitSeconds) {
    $outFile = $LogPrefix + '.stdout.log'; $errFile = $LogPrefix + '.stderr.log'
    $start = New-Object Diagnostics.ProcessStartInfo
    $start.FileName = $Executable
    $start.Arguments = ($Arguments | ForEach-Object { ConvertTo-NativeArgument $_ }) -join ' '
    $start.WorkingDirectory = $WorkingDirectory
    $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true; $start.RedirectStandardError = $true
    $process = New-Object Diagnostics.Process
    $process.StartInfo = $start
    $started = $false; $timedOut = $false; $errorText = ''; $code = $null; $processId = $null
    $stdout = ''; $stderr = ''
    try {
        if (-not $process.Start()) { throw 'Nao foi possivel iniciar o processo.' }
        $started = $true; $processId = $process.Id
        $outTask = $process.StandardOutput.ReadToEndAsync()
        $errTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit($LimitSeconds * 1000)) {
            $timedOut = $true
            $process.Kill() # Somente o processo criado por esta chamada; nunca por nome.
            if (-not $process.WaitForExit(5000)) { throw 'Processo nao encerrou apos o limite.' }
        }
        $code = $process.ExitCode
        if (-not $outTask.Wait(5000) -or -not $errTask.Wait(5000)) { throw 'Captura de log nao terminou.' }
        $stdout = $outTask.Result; $stderr = $errTask.Result
    } catch {
        $errorText = $_.Exception.Message
    } finally {
        if ($started -and -not $process.HasExited) { $process.Kill(); [void]$process.WaitForExit(5000) }
        $process.Dispose()
        [IO.File]::WriteAllText($outFile, $stdout)
        [IO.File]::WriteAllText($errFile, $stderr)
    }
    [pscustomobject]@{ ExitCode = $code; TimedOut = $timedOut; Error = $errorText; Stdout = $outFile; Stderr = $errFile; ProcessId = $processId }
}

function Get-HeadlessVerdict($ProcessResult, [string]$Log, [string]$FrameDirectory, [int]$Width, [int]$Height, [int]$ExpectedFrames) {
    $reason = ''
    if ($ProcessResult.Error) { $reason = $ProcessResult.Error }
    elseif ($ProcessResult.TimedOut) { $reason = 'Limite de tempo excedido.' }
    elseif ($null -eq $ProcessResult.ExitCode -or $ProcessResult.ExitCode -ne 0) { $reason = 'Processo retornou erro: ' + $ProcessResult.ExitCode }
    elseif ($Log -match '=== PROBLEMA:|JNI nao implementado|funcao nao implementada|heap esgotado|SDL_Init:|Nao consegui abrir|Falha ao mapear') { $reason = 'Log registra falha, mesmo que o codigo de saida seja zero.' }
    elseif ($Log -notmatch 'initGame ok' -or $Log -notmatch ('(?m)\[host\] ' + $ExpectedFrames + ' quadros executados\.')) { $reason = 'Inicializacao ou total de quadros nao confirmado no log.' }
    if (-not $reason) {
        $expected = @(1) + @(10..$ExpectedFrames | Where-Object { $_ % 10 -eq 0 })
        foreach ($frame in $expected) {
            $path = Join-Path $FrameDirectory ('frame_{0:D5}.bmp' -f $frame)
            if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { $reason = 'Captura ausente: ' + [IO.Path]::GetFileName($path); break }
            $bytes = [IO.File]::ReadAllBytes($path)
            if ($bytes.Length -lt 54 -or $bytes[0] -ne 66 -or $bytes[1] -ne 77 -or
                [BitConverter]::ToInt32($bytes, 18) -ne $Width -or [Math]::Abs([BitConverter]::ToInt32($bytes, 22)) -ne $Height) {
                $reason = 'BMP invalido ou dimensoes incorretas: ' + [IO.Path]::GetFileName($path); break
            }
            $bits = [BitConverter]::ToUInt16($bytes, 28)
            $offset = [BitConverter]::ToUInt32($bytes, 10)
            $rowBytes = [Math]::Floor(($Width * [double]$bits + 31) / 32) * 4
            if ($bits -notin @(24, 32) -or [BitConverter]::ToUInt16($bytes, 26) -ne 1 -or
                [BitConverter]::ToUInt32($bytes, 30) -ne 0 -or $offset -lt 54 -or
                [BitConverter]::ToUInt32($bytes, 2) -ne $bytes.Length -or $offset + $rowBytes * $Height -gt $bytes.Length) {
                $reason = 'BMP incompleto ou formato inesperado: ' + [IO.Path]::GetFileName($path); break
            }
        }
    }
    $passed = -not [bool]$reason
    if ($passed) { $reason = 'Inicializacao, contagem e cabecalhos BMP confirmados; conteudo visual nao avaliado.' }
    [pscustomobject]@{ Passed = $passed; Detail = $reason }
}

function Get-SessionObservations([string]$Log) {
    $purchases = [regex]::Matches($Log, '\[host\] cash local (\d+) -> (\d+)')
    if ($purchases.Count) {
        $last = $purchases[$purchases.Count - 1]
        $detail = 'Compra acionada durante a execucao; alteracoes de cash: ' + $purchases.Count + '. Ultima alteracao: ' + $last.Groups[1].Value + ' -> ' + $last.Groups[2].Value + '. Persistencia em arquivo nao verificada.'
        if ($Log -match 'resposta da loja nao simulada|callback nativo desativado') { $detail += ' O host informou que nao concluiu a resposta nativa da loja.' }
        [pscustomobject]@{ Teste = 'Compra observada'; Estado = 'AVISO'; Detalhe = $detail }
    }
    $network = [regex]::Matches($Log, '(?m)^\[sonda\][^\r\n]*\brede=(\d+)\b')
    if ($network.Count) {
        $lastState = $network[$network.Count - 1].Groups[1].Value
        if ($lastState -eq '1') {
            [pscustomobject]@{ Teste = 'Espera de rede'; Estado = 'AVISO'; Detalhe = 'Ultima sonda do log registra rede=1 (aguardando). A operacao nao aparece como concluida; isso nao confirma crash ou espera infinita.' }
        } elseif ($lastState -in @('2', '3')) {
            [pscustomobject]@{ Teste = 'Estado de rede'; Estado = 'AVISO'; Detalhe = ('Ultima sonda registra rede=' + $lastState + ' (erro/aviso). Revise a operacao iniciada no jogo.') }
        }
    }
}

function Write-ReportFile([string]$Path, [string]$Text) {
    $temporary = $Path + '.tmp'
    $encoding = New-Object Text.UTF8Encoding -ArgumentList $true
    [IO.File]::WriteAllText($temporary, $Text, $encoding)
    if ([IO.File]::Exists($Path)) { [IO.File]::Replace($temporary, $Path, [NullString]::Value) }
    else { [IO.File]::Move($temporary, $Path) }
}

function Initialize-TestReport([string]$Directory, [string]$ReportMode, [string]$ReportVariant) {
    [void][IO.Directory]::CreateDirectory($Directory)
    $script:TestResults = New-Object 'System.Collections.Generic.List[object]'
    $script:ReportState = [pscustomobject]@{ Directory = $Directory; Started = (Get-Date -Format 'o'); Stage = 'Preparacao'; Completed = $false; Mode = $ReportMode; Variant = $ReportVariant }
    Save-TestReport
}

function Save-TestReport([switch]$Complete) {
    if (-not $script:ReportState) { return }
    $state = $script:ReportState
    if ($Complete) { $state.Completed = $true; $state.Stage = 'Finalizado' }
    $failures = @($script:TestResults | Where-Object { $_.Estado -in @('FALHA', 'MANUAL_FALHA') }).Count
    $warnings = @($script:TestResults | Where-Object { $_.Estado -eq 'AVISO' }).Count
    $pending = @($script:TestResults | Where-Object { $_.Estado -in @('PENDENTE', 'NAO_TESTADO') }).Count
    $code = $null
    if ($state.Completed) { $code = 0; if ($failures) { $code = 1 } }
    $report = [pscustomobject]@{ Data = $state.Started; Atualizado = (Get-Date -Format 'o'); PowerShell = $PSVersionTable.PSVersion.ToString(); Modo = $state.Mode; Variante = $state.Variant; Concluido = $state.Completed; Etapa = $state.Stage; Codigo = $code; Falhas = $failures; Avisos = $warnings; Pendentes = $pending; Resultados = @($script:TestResults.ToArray()) }
    Write-ReportFile (Join-Path $state.Directory 'resultados.json') ($report | ConvertTo-Json -Depth 6)
    $condition = 'PARCIAL - sessao ainda nao concluida. Nao considerar como aprovada.'
    if ($state.Completed) { $condition = 'FINALIZADO - consulte falhas, avisos e pendencias; nao e aprovacao de todos os recursos.' }
    $lines = @('DESTINIA - RELATORIO DE TESTES', ('Data: ' + $report.Data), ('Atualizado: ' + $report.Atualizado), ('Modo: ' + $state.Mode), $condition, ('Etapa: ' + $state.Stage), ('Falhas: {0} | Avisos: {1} | Pendencias/nao testados: {2}' -f $failures, $warnings, $pending), '', 'OK automatico nao confirma gameplay. MANUAL_OK e relato do usuario. AVISO/PENDENTE/NAO_TESTADO nao significam aprovado.', '')
    $lines += @($script:TestResults | ForEach-Object { '[{0}] {1}: {2}' -f $_.Estado, $_.Teste, $_.Detalhe })
    $lines += @('', 'Nao envie pastas de save. Revise relatorio/logs/capturas antes de compartilhar.', 'Nenhum arquivo e enviado a internet por este script.')
    Write-ReportFile (Join-Path $state.Directory 'relatorio.txt') ($lines -join [Environment]::NewLine)
}

function Set-TestStage([string]$Stage) {
    if ($script:ReportState) { $script:ReportState.Stage = $Stage; Save-TestReport }
    Write-Host ('Etapa: ' + $Stage)
}

function Add-TestResult([string]$Name, [string]$Status, [string]$Detail) {
    $script:TestResults.Add([pscustomobject]@{ Teste = $Name; Estado = $Status; Detalhe = $Detail }) | Out-Null
    Write-Host ('[{0}] {1}: {2}' -f $Status, $Name, $Detail)
    Save-TestReport
}

function Add-LogObservations([string]$Label, [string]$Log) {
    foreach ($observation in @(Get-SessionObservations $Log)) {
        Add-TestResult ($Label + ' - ' + $observation.Teste) $observation.Estado $observation.Detalhe
    }
}

function Read-ManualResult([string]$Name, [string]$Question) {
    Set-TestStage ('Aguardando resposta: ' + $Name)
    $answer = (Read-Host ($Question + ' [S=sim/N=falhou/P=nao testado]')).Trim().ToUpperInvariant()
    $status = 'PENDENTE'
    if ($answer -eq 'S') { $status = 'MANUAL_OK' }
    elseif ($answer -eq 'N') { $status = 'MANUAL_FALHA' }
    Add-TestResult $Name $status 'Resposta informada pelo usuario; nao e uma verificacao automatica.'
}

function Get-GuidedArguments([string]$Name, [string]$TestMode, [string]$GamePath, [string]$SavePath, [string]$TracePath) {
    $arguments = @('--game', $GamePath, '--save', $SavePath, '--probe', '--profile')
    if ($Name -eq 'host_zoom') {
        $arguments += @('--lang', 'pt', '--trace-viewport', '--trace-frames', '1200', '--trace-out', $TracePath)
        if ($TestMode -eq 'Camera') { $arguments += '--trace-camera' }
    }
    return $arguments
}

function Invoke-DestiniaSuite {
    $ErrorActionPreference = 'Stop'
    $rootPath = [IO.Path]::GetFullPath($Root)
    $session = Join-Path $rootPath ('relatorios/testes_' + (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
    [void][IO.Directory]::CreateDirectory($session)
    if ($Mode -eq 'Camera') { $Variant = 'host_zoom' }
    Initialize-TestReport $session $Mode $Variant
    $oldPath = $env:PATH; $source = ''; $saveBefore = $null
    $exitCode = 0; $reachedEnd = $false; $caughtFailure = $false
    try {
        Write-Host 'DESTINIA - TESTES AUTOMATICOS E ROTEIRO GUIADO'
        Write-Host ('Resultados: ' + $session)
        if ($env:OS -ne 'Windows_NT') { throw 'O executor completo requer Windows. Apenas as funcoes auxiliares sao testaveis em outros sistemas.' }
        if (Get-Process -Name destinia_host -ErrorAction SilentlyContinue) { throw 'Feche as janelas do Destinia antes do teste. Nenhum processo existente sera encerrado.' }
        if (-not (Test-Path -LiteralPath $RuntimeDir -PathType Container)) { throw 'Runtime UCRT64 nao encontrado. Informe -RuntimeDir ou consulte Dependencias no README.' }
        $env:PATH = $RuntimeDir + [IO.Path]::PathSeparator + $oldPath
        Add-TestResult 'Runtime' 'OK' ('Pasta adicionada somente ao PATH deste diagnostico: ' + $RuntimeDir + '. Dependencias transitivas serao exercitadas ao iniciar o host.')
        $game = Join-Path $rootPath 'game'
        $library = Join-Path $game 'lib/armeabi/libdestinia_jni.so'
        if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw 'Biblioteca ARM nao encontrada em game/lib/armeabi/libdestinia_jni.so.' }
        $header = [IO.File]::ReadAllBytes($library)
        if ($header.Length -lt 52 -or $header[0] -ne 127 -or $header[1] -ne 69 -or $header[2] -ne 76 -or $header[3] -ne 70 -or
            $header[4] -ne 1 -or $header[5] -ne 1 -or [BitConverter]::ToUInt16($header, 18) -ne 40) { throw 'Biblioteca nao tem cabecalho ELF ARM 32 bits little-endian compativel.' }
        if (-not (Test-Path -LiteralPath (Join-Path $game 'assets') -PathType Container) -or
            -not @(Get-ChildItem -LiteralPath (Join-Path $game 'assets') -File).Count) { throw 'Pasta de assets ausente ou vazia.' }
        Add-TestResult 'Dados originais' 'OK' ('ELF ARM e pasta de assets encontrados. SHA-256 da biblioteca: ' + (Get-FileHash -LiteralPath $library).Hash)
        if (-not (Test-Path -LiteralPath (Join-Path $game 'res/raw') -PathType Container)) { Add-TestResult 'Audio' 'AVISO' 'Pasta res/raw ausente; audio precisa de teste manual.' }
        if (-not $Fresh) {
            if ($SaveSource) { $source = [IO.Path]::GetFullPath($SaveSource) }
            elseif ($env:LOCALAPPDATA) { $source = Join-Path $env:LOCALAPPDATA 'Destinia/save' }
            if ($source -and -not (Test-Path -LiteralPath $source -PathType Container)) {
                if ($SaveSource) { throw 'A origem de save informada nao existe.' }
                Add-TestResult 'Origem do save' 'AVISO' 'Save compartilhado nao encontrado; serao usadas pastas novas de teste.'
                $source = ''
            }
        }
        if ($source) { $saveBefore = ConvertTo-Json -InputObject @(Get-SaveInventory $source) -Compress }
        $variants = @('host', 'host_zoom')
        if ($Variant -ne 'Ambos') { $variants = @($Variant) }
        foreach ($name in $variants) {
            $hostDirectory = Join-Path $rootPath $name
            $exe = Join-Path $hostDirectory 'build/destinia_host.exe'
            if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
                Add-TestResult ($name + ' executavel') 'FALHA' ('Nao encontrado: ' + $exe + '. Compile antes de testar; o script nao instala nem compila automaticamente.')
                continue
            }
            Add-TestResult ($name + ' executavel') 'OK' ('SHA-256: ' + (Get-FileHash -LiteralPath $exe).Hash)
            $width = 400; $height = 240
            if ($name -eq 'host_zoom') { $width = 800; $height = 480 }
            $save = Join-Path $session ($name + '_save_automatico')
            Copy-IsolatedSave $source $save
            $frameDir = Join-Path $session ($name + '_frames')
            $arguments = @('--game', $game, '--save', $save, '--headless', [string]$Frames, '--out', $frameDir)
            if ($name -eq 'host_zoom') { $arguments += @('--lang', 'pt') }
            Set-TestStage ($name + ': inicializacao sem janela em execucao')
            $run = Invoke-OwnedProcess $exe $arguments $hostDirectory (Join-Path $session ($name + '_automatico')) $TimeoutSeconds
            $log = [IO.File]::ReadAllText($run.Stdout) + "`n" + [IO.File]::ReadAllText($run.Stderr)
            $verdict = Get-HeadlessVerdict $run $log $frameDir $width $height $Frames
            $state = 'FALHA'; if ($verdict.Passed) { $state = 'OK' }
            Add-TestResult ($name + ' inicializacao sem janela') $state $verdict.Detail
            Add-LogObservations ($name + ' automatico') $log
            if ($Mode -eq 'Automatico') {
                Add-TestResult ($name + ' gameplay, saves, camera e entradas') 'PENDENTE' 'Requer o modo Guiado e observacao humana.'
                continue
            }
            Write-Host ''
            if ($Mode -eq 'Camera') {
                Write-Host 'TESTE FOCADO: carregar a partida e repetir somente a passagem com defeito no mapa/camera.'
                Write-Host '1. F9 ANTES de atravessar: inicia gravacao e captura a tela anterior.'
                Write-Host '2. Atravesse, espere o defeito aparecer e aperte F9 para parar.'
                Write-Host '3. F9 NOVAMENTE, ainda com o defeito visivel: captura a tela posterior e inicia outro trecho.'
                Write-Host '4. Espere alguns segundos, aperte F9 para parar e feche o jogo normalmente.'
                Write-Host 'Nao e necessario testar audio, controles, salvar/reabrir ou acessar a loja.'
            } else {
                Write-Host 'ROTEIRO: iniciar/carregar partida, mover, atacar, abrir inventario/equipar, trocar de mapa e salvar.'
            }
            Write-Host 'Na janela do jogo, clique em TAP TO START e jogue manualmente. O roteiro nao faz os cliques por voce.'
            if ($Mode -ne 'Camera') { Write-Host 'Teste mouse (DPI 100/150% e janela redimensionada), audio, controle/desconexao e Alt+Tab segurando direcao.' }
            if ($name -eq 'host_zoom') { Write-Host 'Teste PT. Na falha de mapa: F9 antes da transicao, F9 para parar; F9 outra vez captura o estado posterior.' }
            Write-Host 'Compras/rede NAO sao acionadas por este roteiro. Use somente o save de teste. Feche a janela ao terminar.'
            Set-TestStage ($name + ': aguardando permissao para abrir janela guiada')
            if ((Read-Host ('Abrir teste guiado de ' + $name + '? [S/N]')).Trim().ToUpperInvariant() -ne 'S') {
                Add-TestResult ($name + ' teste guiado') 'PENDENTE' 'Usuario optou por nao abrir.'
                continue
            }
            $guiSave = Join-Path $session ($name + '_save_guiado')
            Copy-IsolatedSave $source $guiSave
            $guiArgs = @(Get-GuidedArguments $name $Mode $game $guiSave (Join-Path $session 'trace_viewport'))
            Set-TestStage ($name + ': janela guiada em execucao; feche o jogo ao concluir o roteiro')
            $gui = Invoke-OwnedProcess $exe $guiArgs $hostDirectory (Join-Path $session ($name + '_guiado')) $InteractiveTimeoutSeconds
            $guiLog = [IO.File]::ReadAllText($gui.Stdout) + [IO.File]::ReadAllText($gui.Stderr)
            Add-LogObservations ($name + ' guiado') $guiLog
            if ($gui.Error -or $gui.TimedOut -or $gui.ExitCode -ne 0 -or $guiLog -match '=== PROBLEMA:|JNI nao implementado|funcao nao implementada|heap esgotado|SDL_Init:') {
                Add-TestResult ($name + ' processo guiado') 'FALHA' 'Erro, timeout ou falha no log. Verifique stdout/stderr; o roteiro nao confirma sucesso.'
                continue
            }
            if ($Mode -eq 'Camera') {
                Read-ManualResult ($name + ' mapa/camera') 'A troca de mapa e a camera funcionaram sem duplicacao ou defeito?'
                Set-TestStage 'Descricao do defeito de camera'
                $description = Read-Host 'Descreva o defeito e a passagem/mapa (ou informe que nao ocorreu)'
                Add-TestResult 'Descricao da camera' 'INFO' $description
                $captures = @(Get-ChildItem -LiteralPath (Join-Path $session 'trace_viewport') -Filter 'viewport_frame_*.bmp' -File -ErrorAction SilentlyContinue)
                $captureState = 'PENDENTE'
                if ($captures.Count -ge 2) { $captureState = 'INFO' }
                Add-TestResult 'Capturas F9' $captureState ('BMPs encontrados: ' + $captures.Count + '. Sao necessarias imagens antes e depois; a contagem nao confirma a ordem ou o conteudo visual.')
                if ($guiLog -notmatch '\[camera\]') {
                    Add-TestResult 'Log de camera' 'AVISO' 'Nenhuma linha [camera] encontrada; confira se o executavel suporta --trace-camera.'
                } else { Add-TestResult 'Log de camera' 'INFO' 'Coordenadas e dimensoes registradas no stderr do teste guiado; nao validam a correcao visual.' }
                Add-TestResult 'Demais testes de gameplay' 'NAO_TESTADO' 'Modo focado em camera; audio, entradas e salvar/reabrir nao foram solicitados.'
                continue
            }
            Read-ManualResult ($name + ' gameplay') 'Conseguiu iniciar/carregar, mover, atacar, abrir inventario e equipar?'
            Read-ManualResult ($name + ' mapa/camera') 'A troca de mapa e a camera funcionaram sem duplicacao ou defeito?'
            Read-ManualResult ($name + ' mouse/DPI') 'Mouse correto em janela redimensionada e DPI 100/150%?'
            Read-ManualResult ($name + ' audio') 'Musica e efeitos funcionaram?'
            Read-ManualResult ($name + ' controle e foco') 'Controle/desconexao e Alt+Tab funcionaram sem direcao presa?'
            if ($name -eq 'host_zoom') { Read-ManualResult 'host_zoom PT e F9' 'Textos PT e captura/trace F9 funcionaram?' }
            Set-TestStage ($name + ': aguardando decisao de salvar/reabrir')
            if ((Read-Host 'Salvou e quer reabrir a MESMA copia para conferir o progresso? [S/N]').Trim().ToUpperInvariant() -eq 'S') {
                Set-TestStage ($name + ': reabertura em execucao; carregue a partida e confira o progresso')
                $reload = Invoke-OwnedProcess $exe $guiArgs $hostDirectory (Join-Path $session ($name + '_reabrir')) $InteractiveTimeoutSeconds
                $reloadLog = [IO.File]::ReadAllText($reload.Stdout) + [IO.File]::ReadAllText($reload.Stderr)
                Add-LogObservations ($name + ' reabrir') $reloadLog
                if ($reload.Error -or $reload.TimedOut -or $reload.ExitCode -ne 0 -or $reloadLog -match '=== PROBLEMA:|JNI nao implementado|funcao nao implementada|heap esgotado|SDL_Init:') {
                    Add-TestResult ($name + ' salvar/reabrir') 'FALHA' 'Falha ao executar a segunda abertura; confira logs.'
                } else { Read-ManualResult ($name + ' salvar/reabrir') 'O progresso salvo permaneceu apos fechar e carregar novamente?' }
            } else { Add-TestResult ($name + ' salvar/reabrir') 'PENDENTE' 'Segunda abertura nao executada.' }
        }
        Add-TestResult 'Validacao completa de compras e rede' 'NAO_TESTADO' 'Menus nao sao acionados automaticamente. Eventos observados nos logs aparecem separadamente como avisos; isso nao valida os recursos online.'
        Add-TestResult 'Casos internos de libc/ELF/SDL' 'NAO_TESTADO' 'fread/fwrite tamanho zero, entradas ELF corrompidas e falhas forjadas de SDL exigem testes de codigo; este BAT nao os valida.'
        $reachedEnd = $true
    } catch {
        $caughtFailure = $true
        Add-TestResult 'Execucao do diagnostico' 'FALHA' $_.Exception.Message
    } finally {
        $env:PATH = $oldPath
        if ($null -ne $saveBefore) {
            try {
                $after = ConvertTo-Json -InputObject @(Get-SaveInventory $source) -Compress
                if ($after -eq $saveBefore) { Add-TestResult 'Save de origem' 'OK' 'Hashes antes/depois iguais. Somente as copias foram usadas pelo roteiro.' }
                else { Add-TestResult 'Save de origem' 'FALHA' 'Conteudo mudou durante o teste. Verifique se outro processo usou a origem; nao ha restauracao automatica.' }
            } catch { Add-TestResult 'Save de origem' 'FALHA' ('Nao foi possivel conferir hashes: ' + $_.Exception.Message) }
        }
        if (@($script:TestResults | Where-Object { $_.Estado -in @('FALHA', 'MANUAL_FALHA') }).Count) { $exitCode = 1 }
        Save-TestReport -Complete:($reachedEnd -or $caughtFailure)
        if (-not $script:ReportState.Completed) { $exitCode = 2 }
        Write-Host ('Relatorio salvo: ' + (Join-Path $session 'relatorio.txt'))
        $warningCount = @($script:TestResults | Where-Object { $_.Estado -eq 'AVISO' }).Count
        if ($warningCount) { Write-Host ('ATENCAO: {0} aviso(s) registrado(s). Codigo zero nao elimina avisos ou testes pendentes.' -f $warningCount) }
    }
    return $exitCode
}

if ($MyInvocation.InvocationName -ne '.') { exit (Invoke-DestiniaSuite) }
