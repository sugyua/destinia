# Execute the actual BATs with a harmless Windows EXE; never starts the game.
$ErrorActionPreference='Stop'
$fixture=Join-Path ([IO.Path]::GetTempPath()) ('destinia launcher '+[guid]::NewGuid().ToString('N'))
$oldVersion=$env:DESTINIA_FIXTURE_VERSION;$oldMarker=$env:DESTINIA_FIXTURE_MARKER;$oldLocal=$env:LOCALAPPDATA;$oldRuntime=$env:DESTINIA_MSYS2_ROOT
try {
    [void][IO.Directory]::CreateDirectory($fixture)
    $exe=Join-Path $fixture 'fixture.exe'
    Add-Type -OutputType ConsoleApplication -OutputAssembly $exe -TypeDefinition @'
using System;
using System.IO;
public class LauncherFixture {
 public static int Main(string[] args) {
  if(args.Length==1 && args[0]=="--version") {Console.WriteLine("Destinia "+Environment.GetEnvironmentVariable("DESTINIA_FIXTURE_VERSION")+" Beta");return 0;}
  File.WriteAllText(Environment.GetEnvironmentVariable("DESTINIA_FIXTURE_MARKER"),"LAUNCHED");return 0;
 }
}
'@
    foreach($folder in @('host/build','host_zoom/build','runtime/ucrt64/bin','dados locais')) {[void][IO.Directory]::CreateDirectory((Join-Path $fixture $folder))}
    foreach($hostName in @('host','host_zoom')) {Copy-Item $exe (Join-Path $fixture ($hostName+'/build/destinia_host.exe'))}
    [IO.File]::WriteAllText((Join-Path $fixture 'runtime/ucrt64/bin/SDL2.dll'),'fixture')
    $env:DESTINIA_MSYS2_ROOT=Join-Path $fixture 'runtime';$env:LOCALAPPDATA=Join-Path $fixture 'dados locais'
    $versionFile=Join-Path $fixture 'VERSION';$current=([IO.File]::ReadAllText((Join-Path $PSScriptRoot '../VERSION'))).Trim()
    $inputFile=Join-Path $fixture 'input.txt';[IO.File]::WriteAllText($inputFile,"x`r`n")
    foreach($name in @('Abrir Destinia.bat','Abrir Destinia Zoom 800x480.bat')) {
        $bat=Join-Path $fixture $name;Copy-Item -LiteralPath (Join-Path $PSScriptRoot ('../'+$name)) -Destination $bat
        foreach($case in @(@($current,$current,$true),@('0.8','0.8',$true),@($current,'0.6',$false),@('invalid',$current,$false))) {
            [IO.File]::WriteAllText($versionFile,($case[0]+"`n"));$env:DESTINIA_FIXTURE_VERSION=$case[1]
            $marker=Join-Path $fixture 'launched.txt';$env:DESTINIA_FIXTURE_MARKER=$marker;if(Test-Path $marker){Remove-Item $marker}
            $args='/d /s /c ""'+$bat+'""'
            $start=New-Object Diagnostics.ProcessStartInfo
            $start.FileName=$env:ComSpec;$start.Arguments=$args;$start.WorkingDirectory=$fixture;$start.UseShellExecute=$false
            $start.RedirectStandardInput=$true;$start.RedirectStandardOutput=$true;$start.RedirectStandardError=$true
            $process=New-Object Diagnostics.Process;$process.StartInfo=$start
            try {
                if(-not $process.Start()){throw 'Nao iniciou CMD do teste'}
                $outTask=$process.StandardOutput.ReadToEndAsync();$errTask=$process.StandardError.ReadToEndAsync()
                $process.StandardInput.WriteLine('x');$process.StandardInput.Close()
                if(-not $process.WaitForExit(15000)) {$process.Kill();throw 'BAT ficou aguardando indefinidamente'}
                $code=$process.ExitCode;$output=$outTask.Result+$errTask.Result;$launched=Test-Path -LiteralPath $marker
                if($launched -ne $case[2] -or ($case[2] -and $code -ne 0) -or (-not $case[2] -and $code -eq 0)) {throw ($name+' nao respeitou VERSION/EXE: '+$case[0]+'/'+$case[1]+'; iniciou='+$launched+'; codigo='+$code+"`n"+$output)}
            }finally{$process.Dispose()}

        }
    }
    Write-Host 'OK: dois BATs executados no CMD, versao atual/futura, recusa de EXE antigo e VERSION invalido, pasta com espacos.'
} finally {
    $env:DESTINIA_FIXTURE_VERSION=$oldVersion;$env:DESTINIA_FIXTURE_MARKER=$oldMarker;$env:LOCALAPPDATA=$oldLocal;$env:DESTINIA_MSYS2_ROOT=$oldRuntime
    if(Test-Path $fixture){Remove-Item -LiteralPath $fixture -Recurse -Force}
}
