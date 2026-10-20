# Amostra o uso de CPU do processo destinia_host a cada intervalo e grava em CSV.
# Colunas: interval_s, cores_utilizados, pct_de_todos_os_cores, wall_s, cpu_s
# "cores_utilizados" = delta de segundos de CPU do processo / delta de segundos de relogio.
#   1.0 significa "um nucleo inteiro ocupado o tempo todo"; 0.5 = meio nucleo; 4.0 = 4 nucleos.
param(
  [string]$ProcessName = "destinia_host",
  [string]$OutCsv      = "cpu_usage.csv",
  [int]$IntervalMs     = 1000,
  [int]$StopAfterAbsent = 3   # encerra depois de N checagens sem o processo (jogo fechou)
)

$allCores = [Environment]::ProcessorCount
$sw       = [Diagnostics.Stopwatch]::StartNew()
$inv      = [System.Globalization.CultureInfo]::InvariantCulture   # ponto decimal, nao virgula
$lastCpu  = $null
$lastWall = 0.0
$absent   = 0

# cabecalho
"interval_s,cores_utilizados,pct_todos_cores,wall_s,cpu_s,cores_total" | Out-File -FilePath $OutCsv -Encoding ascii

while ($true) {
  Start-Sleep -Milliseconds $IntervalMs
  $p = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
  if (-not $p) {
    $absent++
    if ($absent -ge $StopAfterAbsent) { break }
    continue
  }
  $absent = 0
  # se varias instancias, soma o tempo de CPU (TotalSeconds e numerico)
  $cpu  = ($p | ForEach-Object { $_.TotalProcessorTime.TotalSeconds } | Measure-Object -Sum).Sum
  $wall = $sw.Elapsed.TotalSeconds
  if ($null -ne $lastCpu) {
    $dWall = $wall - $lastWall
    $dCpu  = $cpu  - $lastCpu
    if ($dWall -gt 0 -and $dCpu -ge 0) {
      $cores   = $dCpu / $dWall
      $pctAll  = 100.0 * $cores / $allCores
      ("{0},{1},{2},{3},{4},{5}" -f $dWall.ToString("F2",$inv), $cores.ToString("F3",$inv), $pctAll.ToString("F1",$inv), $wall.ToString("F2",$inv), $cpu.ToString("F2",$inv), $allCores) |
        Out-File -FilePath $OutCsv -Append -Encoding ascii
    }
  }
  $lastCpu  = $cpu
  $lastWall = $wall
}
