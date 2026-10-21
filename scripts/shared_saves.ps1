# Configuracao global usada pelo configurador e pelo diagnostico (PowerShell 5.1+).
function Test-DestiniaAbsolutePath([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path) -or $Path.Contains("`n") -or $Path.Contains("`r") -or $Path.Contains('"')) { return $false }
    if (-not [IO.Path]::IsPathRooted($Path)) { return $false }
    if ([IO.Path]::DirectorySeparatorChar -eq '\') {
        return ($Path -match '^[A-Za-z]:[\\/]' -or $Path -match '^\\\\[^\\]+\\[^\\]+')
    }
    return $true
}

function Get-DestiniaSaveDirectory([string]$LocalDataRoot = $env:LOCALAPPDATA) {
    $selected = $env:DESTINIA_SAVE_DIR
    if (-not $selected -and $LocalDataRoot) {
        $config = Join-Path $LocalDataRoot 'Destinia/config.ini'
        if (Test-Path -LiteralPath $config) {
            $found = $false
            foreach ($raw in [IO.File]::ReadAllLines($config)) {
                $line = $raw.Trim().TrimStart([char]0xFEFF)
                if (-not $line -or $line.StartsWith('#') -or $line.StartsWith(';')) { continue }
                $equal = $line.IndexOf('=')
                if ($equal -lt 0) { throw 'Linha invalida em config.ini.' }
                if ($line.Substring(0, $equal).Trim() -ceq 'save_dir') {
                    if ($found) { throw 'save_dir repetido em config.ini.' }
                    $selected = $line.Substring($equal + 1).Trim()
                    $found = $true
                    if (-not $selected) { throw 'save_dir vazio em config.ini; o save nao sera trocado silenciosamente.' }
                }
            }
        }
    }
    if (-not $selected) {
        if (-not $LocalDataRoot) { throw 'LOCALAPPDATA ausente. Informe DESTINIA_SAVE_DIR ou uma origem explicita para o diagnostico.' }
        return (Join-Path $LocalDataRoot 'Destinia/save')
    }
    if (-not (Test-DestiniaAbsolutePath $selected)) { throw 'A pasta de save precisa ser um caminho absoluto valido.' }
    $full = [IO.Path]::GetFullPath($selected)
    if ($full.TrimEnd('\', '/') -eq [IO.Path]::GetPathRoot($full).TrimEnd('\', '/')) { throw 'A raiz do disco nao pode ser usada como pasta de save.' }
    return $full
}

function Set-DestiniaSaveDirectory([string]$Directory, [string]$LocalDataRoot = $env:LOCALAPPDATA) {
    if (-not $LocalDataRoot) { throw 'LOCALAPPDATA ausente; nao foi possivel guardar a configuracao global.' }
    if (-not (Test-DestiniaAbsolutePath $Directory)) { throw 'Escolha um caminho absoluto, por exemplo C:\Jogos\DestiniaSaves.' }
    $selected = [IO.Path]::GetFullPath($Directory)
    if ($selected.TrimEnd('\', '/') -eq [IO.Path]::GetPathRoot($selected).TrimEnd('\', '/')) { throw 'Escolha uma pasta, nao a raiz do disco.' }
    if ((Test-Path -LiteralPath $selected) -and -not (Test-Path -LiteralPath $selected -PathType Container)) { throw 'O caminho escolhido e um arquivo.' }
    [void][IO.Directory]::CreateDirectory($selected)
    $data = Join-Path $LocalDataRoot 'Destinia'
    [void][IO.Directory]::CreateDirectory($data)
    $configLock = [IO.File]::Open((Join-Path $data 'config.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    try {
        $config = Join-Path $data 'config.ini'
        $lines = @('# Destinia: pasta compartilhada entre hosts e instalacoes 0.5 ou posteriores.')
        if (Test-Path -LiteralPath $config) {
            $lines = @([IO.File]::ReadAllLines($config) | Where-Object { $_ -cnotmatch '^\s*save_dir\s*=' })
        }
        $lines += 'save_dir=' + $selected
        $temporary = $config + '.' + [guid]::NewGuid().ToString('N') + '.tmp'
        try {
            [IO.File]::WriteAllText($temporary, ($lines -join "`r`n") + "`r`n", (New-Object Text.UTF8Encoding -ArgumentList $false))
            if (Test-Path -LiteralPath $config) { [IO.File]::Replace($temporary, $config, ($config + '.previous')) }
            else { [IO.File]::Move($temporary, $config) }
        } finally {
            if (Test-Path -LiteralPath $temporary) { Remove-Item -LiteralPath $temporary -Force }
        }
    } finally { $configLock.Dispose() }
}
