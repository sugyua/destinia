// Windows GUI launcher for Destinia. Uses only the Go standard library.
// The original game/host, config, and saves are never modified by this launcher.
package main

import (
    "fmt"
    "os"
    "os/exec"
    "path/filepath"
    "strings"
    "syscall"
    "unsafe"
)

// Built with -X main.hostSubdir=host or host_zoom.
var hostSubdir = "host"

func errorDialog(detail string) {
    title, _ := syscall.UTF16PtrFromString("Destinia - nao foi possivel iniciar")
    body, _ := syscall.UTF16PtrFromString(detail)
    proc := syscall.NewLazyDLL("user32.dll").NewProc("MessageBoxW")
    _, _, _ = proc.Call(0, uintptr(unsafe.Pointer(body)), uintptr(unsafe.Pointer(title)), 0x10)
    os.Exit(1)
}

func withRuntimePath(env []string, dllDir string) []string {
    for i, item := range env {
        key, value, exists := strings.Cut(item, "=")
        if exists && strings.EqualFold(key, "PATH") {
            env[i] = key + "=" + dllDir + ";" + value
            return env
        }
    }
    return append(env, "PATH="+dllDir)
}

func main() {
    if hostSubdir != "host" && hostSubdir != "host_zoom" {
        errorDialog("Destino do launcher invalido.")
    }
    launcherExe, err := os.Executable()
    if err != nil {
        errorDialog("Nao foi possivel localizar Destinia.exe: " + err.Error())
    }
    root := filepath.Dir(launcherExe)
    hostDir := filepath.Join(root, hostSubdir)
    gameExe := filepath.Join(hostDir, "build", "destinia_host.exe")
    if info, err := os.Stat(gameExe); err != nil || info.IsDir() {
        errorDialog(fmt.Sprintf("Executavel do jogo ausente:\n%s\n\nExtraia o pacote Windows completo da Release. O ZIP de fontes nao inclui EXEs.", gameExe))
    }

    versionData, err := os.ReadFile(filepath.Join(root, "VERSION"))
    if err != nil { errorDialog("Arquivo VERSION ausente. Extraia o ZIP Windows completo.") }
    version := strings.TrimSpace(string(versionData))
    if version == "" || strings.ContainsAny(version, "\r\n ") {
        errorDialog("VERSION invalido. Extraia novamente o pacote Windows completo.")
    }

    msysRoot := strings.Trim(strings.TrimSpace(os.Getenv("DESTINIA_MSYS2_ROOT")), "\"")
    if msysRoot == "" { msysRoot = "C:\\msys64" }
    runtimeDir := filepath.Join(msysRoot, "ucrt64", "bin")
    if _, err := os.Stat(filepath.Join(runtimeDir, "SDL2.dll")); err != nil {
        errorDialog("SDL2.dll ausente em:\n" + runtimeDir + "\n\nInstale MSYS2 UCRT64 e os pacotes de INSTALL.md ou configure DESTINIA_MSYS2_ROOT.")
    }
    env := withRuntimePath(os.Environ(), runtimeDir)
    check := exec.Command(gameExe, "--version")
    check.Dir, check.Env = hostDir, env
    versionOutput, err := check.CombinedOutput()
    expected := "Destinia " + version + " Beta"
    if err != nil || strings.TrimSpace(string(versionOutput)) != expected {
        errorDialog(fmt.Sprintf("Host %s incompativel com VERSION (%s).\n\nUse o ZIP Windows correspondente e verifique MSYS2.\nDetalhe: %s", hostSubdir, version, strings.TrimSpace(string(versionOutput))))
    }

    originalLibrary := filepath.Join(root, "game", "lib", "armeabi", "libdestinia_jni.so")
    if _, err := os.Stat(originalLibrary); err != nil {
        errorDialog("A pasta game original nao foi encontrada:\n" + originalLibrary + "\n\nVeja INSTALL.md. O download nao inclui o APK extraido.")
    }
    game := exec.Command(gameExe, os.Args[1:]...)
    game.Dir, game.Env = hostDir, env
    if err := game.Start(); err != nil {
        errorDialog("Nao foi possivel abrir Destinia: " + err.Error())
    }
    _ = game.Process.Release()
}
