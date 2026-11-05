$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('destinia-package-candidate-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Push-Location $project
try {
  & py scripts/package_release.py --out $scratch
  if ($LASTEXITCODE) { throw 'Source package validation failed' }
  $archives = @(Get-ChildItem -LiteralPath $scratch -File)
  if ($archives.Count -ne 2 -or @($archives.Name | Where-Object { $_ -notmatch '^destinia-0\.16\.0-beta-source\.zip(\.sha256)?$' }).Count) {
    throw 'Candidate package or SHA-256 file is missing'
  }
  Write-Output "PACKAGE_CANDIDATE_OK: $($archives.Name -join ', ')"
} finally {
  Pop-Location
  $resolvedScratch = [IO.Path]::GetFullPath($scratch)
  $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
  if ($resolvedScratch.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and (Test-Path -LiteralPath $resolvedScratch)) {
    Remove-Item -LiteralPath $resolvedScratch -Recurse -Force
  }
}
