# CekyMonitor Build Script
# Usage: .\build.ps1         (build only)
#        .\build.ps1 -Run    (build and run)

param([switch]$Run)

$ErrorActionPreference = "Stop"

# MSYS2 MinGW path
$mingwPath = "C:\msys64\mingw64\bin"
if (!(Test-Path "$mingwPath\g++.exe")) {
    Write-Host "HATA: MinGW g++ bulunamadi!" -ForegroundColor Red
    Write-Host "MSYS2 kurun ve mingw-w64-x86_64-gcc paketini yukleyin:" -ForegroundColor Yellow
    Write-Host "  winget install MSYS2.MSYS2" -ForegroundColor Gray
    Write-Host "  # MSYS2 shell'de: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw" -ForegroundColor Gray
    exit 1
}
$env:PATH = "$mingwPath;$env:PATH"

# Build directory
if (!(Test-Path "build")) { New-Item -ItemType Directory -Path "build" | Out-Null }

Write-Host "`n  CEKY MONITOR - Build Script" -ForegroundColor Cyan
Write-Host "  ===========================`n" -ForegroundColor DarkCyan

$src = @(
    "src/main_win.cpp",
    "lib/imgui/imgui.cpp",
    "lib/imgui/imgui_draw.cpp",
    "lib/imgui/imgui_tables.cpp",
    "lib/imgui/imgui_widgets.cpp",
    "lib/imgui/imgui_impl_glfw.cpp",
    "lib/imgui/imgui_impl_opengl3.cpp"
)
$flags = "-std=c++17 -O2 -Wall -I./lib/imgui"
$libs = "-lglfw3 -lopengl32 -lgdi32 -liphlpapi -lpsapi -ldwmapi -limm32 -static"
$output = "build/CekyMonitor.exe"

Write-Host "  Derleniyor..." -ForegroundColor Yellow
$cmd = "g++ $flags $($src -join ' ') -o $output $libs"

$sw = [System.Diagnostics.Stopwatch]::StartNew()
Invoke-Expression $cmd
$sw.Stop()

if ($LASTEXITCODE -eq 0) {
    $size = [math]::Round((Get-Item $output).Length / 1MB, 1)
    Write-Host "  Basarili! -> $output ($size MB) [$($sw.Elapsed.TotalSeconds.ToString('0.0'))s]" -ForegroundColor Green
    
    if ($Run) {
        Write-Host "  Calistiriliyor...`n" -ForegroundColor Cyan
        Start-Process $output
    }
} else {
    Write-Host "  Derleme basarisiz!" -ForegroundColor Red
    exit 1
}
