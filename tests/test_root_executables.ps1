# Smoke dos inicializadores GUI: usa EXE inofensivo e dados sinteticos, nunca inicia o jogo real.
$ErrorActionPreference='Stop'
$fixture=Join-Path ([IO.Path]::GetTempPath()) ('destinia root exe '+[guid]::NewGuid().ToString('N'))
$oldRuntime=$env:DESTINIA_MSYS2_ROOT
$oldMarker=$env:DESTINIA_FIXTURE_MARKER
$oldVersion=$env:DESTINIA_FIXTURE_VERSION
try {
    [void][IO.Directory]::CreateDirectory($fixture)
    $dummy=Join-Path $fixture 'fixture.exe'
    Add-Type -OutputAssembly $dummy -OutputType ConsoleApplication -TypeDefinition @'
using System;
using System.IO;
public class DestiniaDummy {
 public static int Main(string[] args) {
  if(args.Length==1 && args[0]=="--version") {Console.WriteLine("Destinia "+Environment.GetEnvironmentVariable("DESTINIA_FIXTURE_VERSION")+" Beta");return 0;}
  File.WriteAllText(Environment.GetEnvironmentVariable("DESTINIA_FIXTURE_MARKER"),Environment.CurrentDirectory);
  return 0;
 }
}
'@
    foreach($dir in @('host/build','host_zoom/build','game/lib/armeabi','runtime/ucrt64/bin')) {
        [void][IO.Directory]::CreateDirectory((Join-Path $fixture $dir))
    }
    [IO.File]::WriteAllText((Join-Path $fixture 'VERSION'),([IO.File]::ReadAllText((Join-Path $PSScriptRoot '../VERSION'))))
    [IO.File]::WriteAllText((Join-Path $fixture 'game/lib/armeabi/libdestinia_jni.so'),'test-only')
    [IO.File]::WriteAllText((Join-Path $fixture 'runtime/ucrt64/bin/SDL2.dll'),'test-only')
    foreach($hostName in @('host','host_zoom')) {Copy-Item -LiteralPath $dummy -Destination (Join-Path $fixture ($hostName+'/build/destinia_host.exe'))}
    $env:DESTINIA_FIXTURE_VERSION=([IO.File]::ReadAllText((Join-Path $fixture 'VERSION'))).Trim()
    $env:DESTINIA_MSYS2_ROOT=Join-Path $fixture 'runtime'
    foreach($sample in @(@('Destinia.exe','host'),@('Destinia Zoom.exe','host_zoom'))) {
        $name=$sample[0];$hostName=$sample[1]
        $source=Join-Path $PSScriptRoot ('../'+$name)
        if(!(Test-Path -LiteralPath $source)) {throw ('EXE nao compilado: '+$name)}
        Copy-Item -LiteralPath $source -Destination (Join-Path $fixture $name)
        $marker=Join-Path $fixture ($hostName+'-opened.txt')
        $env:DESTINIA_FIXTURE_MARKER=$marker
        $proc=Start-Process -FilePath (Join-Path $fixture $name) -WorkingDirectory $fixture -PassThru
        try {
            if(!$proc.WaitForExit(15000)) {$proc.Kill();throw ($name+' travou')}
        } finally {$proc.Dispose()}
        for($n=0;$n -lt 100 -and !(Test-Path -LiteralPath $marker);$n++){Start-Sleep -Milliseconds 100}
        if(!(Test-Path -LiteralPath $marker)) {throw ($name+' nao abriu host de teste')}
        $cwd=[IO.File]::ReadAllText($marker)
        if($cwd -ne (Join-Path $fixture $hostName)) {throw ($name+' iniciou no diretorio errado: '+$cwd)}
    }
    Write-Host 'OK: Destinia.exe e Destinia Zoom.exe abriram seus hosts de teste sem BAT.'
} finally {
    $env:DESTINIA_MSYS2_ROOT=$oldRuntime
    $env:DESTINIA_FIXTURE_MARKER=$oldMarker
    $env:DESTINIA_FIXTURE_VERSION=$oldVersion
    Remove-Item -LiteralPath $fixture -Force -Recurse -ErrorAction SilentlyContinue
}
