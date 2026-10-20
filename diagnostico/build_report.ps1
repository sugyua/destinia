# Monta o relatorio de calor a partir do log do profiler e do CSV de uso de CPU.
param(
  [string]$Log  = "perfil_bruto.log",
  [string]$Cpu  = "cpu_usage.csv",
  [string]$Out  = "relatorio_calor.txt"
)

$emulAvg = @(); $emulMax = @(); $sleepAvg = @(); $wallAvg = @(); $fpsEff = @(); $overBudget = @()
$fmtHost = 0; $fmtZoom = 0

foreach ($line in (Get-Content -Path $Log -ErrorAction SilentlyContinue)) {
  if ($line -match 'emul=([\d.]+)ms \(max ([\d.]+)ms\).*wall=([\d.]+)ms.*FPS efetivo=([\d.]+).*?([\d.]+)% quadros emul acima') {
    $fmtHost++
    $emulAvg += [double]$Matches[1]
    $emulMax += [double]$Matches[2]
    $wallAvg += [double]$Matches[3]
    $fpsEff  += [double]$Matches[4]
    $overBudget += [double]$Matches[5]
  }
  elseif ($line -match 'emulacao ([\d.]+) media\/([\d.]+) max.*espera ([\d.]+)') {
    $fmtZoom++
    $emulAvg += [double]$Matches[1]
    $emulMax += [double]$Matches[2]
    $sleepAvg += [double]$Matches[3]
  }
  if ($line -match '([\d.]+) FPS \|' -and $line -match 'por quadro') { $fpsEff += [double]$Matches[1] }
}

# --- CPU ---
$cores = @(); $pct = @(); $allCores = 0; $wallTotal = 0.0
Import-Csv -Path $Cpu -ErrorAction SilentlyContinue | ForEach-Object {
  $cores += [double]$_.cores_utilizados
  $pct   += [double]$_.pct_todos_cores
  $allCores = [int]$_.cores_total
  $wallTotal = [double]$_.wall_s
}
function Avg($a) { if ($a.Count -eq 0) { 0 } else { ($a | Measure-Object -Average).Average } }
function Max($a) { if ($a.Count -eq 0) { 0 } else { ($a | Measure-Object -Maximum).Maximum } }

$emulAvgV = Avg $emulAvg; $emulMaxV = Max $emulMax
$wallAvgV = Avg $wallAvg; $sleepAvgV = Avg $sleepAvg
$overV    = Avg $overBudget
$coresAvg = Avg $cores;   $coresMax = Max $cores
$pctAvg   = Avg $pct;     $pctMax   = Max $pct

# --- diagnostico ---
$diag = @()
if ($coresAvg -ge 0.85) {
  $diag += "CPU ALTO: em media $([string]::Format('{0:N2}', $coresAvg)) nucleo(s) ocupado(s) o tempo todo. Isso e a causa direta do aquecimento."
} elseif ($coresAvg -ge 0.4) {
  $diag += "CPU MODERADA: ~$([string]::Format('{0:N2}', $coresAvg)) de nucleo ocupado. Da para esquentar em uso prolongado, mas nao e maximo."
} else {
  $diag += "CPU BAIXA: ~$([string]::Format('{0:N2}', $coresAvg)) nucleo ocupado. Se o processador esquenta mesmo assim, o calor nao vem da CPU deste processo (verifique GPU/tela/outras abas)."
}
if ($overV -ge 50) {
  $diag += "EMULACAO GARGALO: $([string]::Format('{0:N0}', $overV))%% dos quadros a emulacao demorou MAIS que o tempo que o jogo pediu. Nesses quadros o host nao tem o que dormir e a CPU fica 100%% de um nucleo. Calor = custo inerente do Unicorn, nao ritmo forçado."
} elseif ($overV -gt 0) {
  $diag += "Estourados ocasionais: $([string]::Format('{0:N0}', $overV))%% dos quadros passaram do orçamento (geralmente carregamento de assets). Nao sustenta calor sozinho."
} else {
  $diag += "Nenhum quadro estourou o orçamento: o ritmo esta respeitando os ~16 FPS do jogo e o host esta dormindo direito."
}
if ($emulMaxV -gt 300) {
  $diag += "PONTOS DE CONGELAMENTO: pico de emulacao de $([string]::Format('{0:N0}', $emulMaxV)) ms em um quadro (provavel carga de arquivo). Isso e travada curta, nao calor constante."
}

$sb = New-Object System.Text.StringBuilder
[void]$sb.AppendLine("==========================================================")
[void]$sb.AppendLine("  RELATORIO DE CALOR - DESTINIA PC PORT")
[void]$sb.AppendLine("  Gerado: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
[void]$sb.AppendLine("==========================================================")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[AMBIENTE]")
[void]$sb.AppendLine("  Nucleos/logicos: $allCores")
[void]$sb.AppendLine("  Duracao medida:  $([string]::Format('{0:N1}', $wallTotal)) s")
[void]$sb.AppendLine("  Formato do profiler detectado: $(if($fmtHost -gt 0){'host (400x240)'}else{'host_zoom (800x480)'})")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[EMULACAO POR QUADRO]  (quanto a CPU trabalha por quadro)")
[void]$sb.AppendLine("  Media: $([string]::Format('{0:N2}', $emulAvgV)) ms   Maximo: $([string]::Format('{0:N2}', $emulMaxV)) ms")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[RITMO]  (o jogo roda a ~16 FPS = ~60 ms por quadro)")
if ($wallAvgV -gt 0) { [void]$sb.AppendLine("  Quadro medio (wall): $([string]::Format('{0:N2}', $wallAvgV)) ms") }
if ($sleepAvgV -gt 0) { [void]$sb.AppendLine("  Dormindo por quadro: $([string]::Format('{0:N2}', $sleepAvgV)) ms") }
if ($fpsEff.Count -gt 0) { [void]$sb.AppendLine("  FPS efetivo medio:   $([string]::Format('{0:N1}', (Avg $fpsEff)))  (esperado ~16)") }
[void]$sb.AppendLine("  Quadros acima do orcamento: $([string]::Format('{0:N0}', $overV)) %")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[USO DE CPU DO PROCESSO]  (1.0 = um nucleo 100%% ocupado)")
[void]$sb.AppendLine("  Media: $([string]::Format('{0:N2}', $coresAvg)) nucleos ($([string]::Format('{0:N0}', $pctAvg)) %% de todos os $allCores)")
[void]$sb.AppendLine("  Pico:  $([string]::Format('{0:N2}', $coresMax)) nucleos ($([string]::Format('{0:N0}', $pctMax)) %% de todos)")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[DIAGNOSTICO]")
foreach ($d in $diag) { [void]$sb.AppendLine("  - $d") }
[void]$sb.AppendLine("")
[void]$sb.AppendLine("[RAW] Ultimas linhas do profiler:")
Get-Content -Path $Log -ErrorAction SilentlyContinue | Select-Object -Last 6 | ForEach-Object { [void]$sb.AppendLine("    $_") }

$sb.ToString() | Out-File -FilePath $Out -Encoding utf8
Write-Host "Relatorio gerado: $Out"
