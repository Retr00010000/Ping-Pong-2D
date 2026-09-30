# Build and run script for Pong 2D using MinGW-w64 (GCC / g++)
param(
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
Push-Location $root

try {
    Write-Host "===> Building Pong 2D with GCC (g++)..." -ForegroundColor Cyan

    # Terminate any running instance of Pong before compiling
    Stop-Process -Name "Pong" -Force -ErrorAction SilentlyContinue

    # 1. Locate g++.exe
    $gpp = (Get-Command g++ -ErrorAction SilentlyContinue).Source
    if (-not $gpp) {
        $wingetCandidate = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter "g++.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($wingetCandidate) {
            $gpp = $wingetCandidate.FullName
        }
    }

    if (-not $gpp) {
        Write-Host "Error: g++ compiler not found! Please ensure MinGW-w64 is installed." -ForegroundColor Red
        exit 1
    }

    Write-Host "Using compiler: $gpp" -ForegroundColor Green

    # 2. Output directory setup
    $outDir = Join-Path $root "Debug"
    if (-not (Test-Path $outDir)) {
        New-Item -ItemType Directory -Path $outDir | Out-Null
    }

    # 3. Source files
    $sources = @(
        "source/Pong.cpp",
        "source/engine/glad.c",
        "source/engine/EBO.cpp",
        "source/engine/ShaderClass.cpp",
        "source/engine/stb.cpp",
        "source/engine/Texture.cpp",
        "source/engine/VAO.cpp",
        "source/engine/VBO.cpp"
    )

    # 4. Compile with g++
    $exe = Join-Path $outDir "Pong.exe"
    $compileArgs = @(
        "-std=c++20",
        "-mwindows",
        "-static",
        "-static-libgcc",
        "-static-libstdc++",
        "-Isource",
        "-Isource/engine",
        "-IDependencies/GLAD/include",
        "-IDependencies/GLFW/include",
        "-IDependencies/stb",
        "-LDependencies/GLFW/lib-mingw-w64",
        "-lglfw3",
        "-lopengl32",
        "-lgdi32",
        "-o", $exe
    )

    & $gpp @sources @compileArgs

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit 1
    }

    # 5. Copy assets
    if (Test-Path "$root\assets") {
        Copy-Item "$root\assets" -Destination "$outDir" -Recurse -Force
    }

    Write-Host "===> Build successful! Binary: $exe" -ForegroundColor Green

    # 6. Run
    if (-not $NoRun) {
        Write-Host "===> Launching Pong 2D..." -ForegroundColor Cyan
        try {
            Start-Process -FilePath $exe -WorkingDirectory $root
        }
        catch {
            Write-Host ""
            Write-Host "=====================================================================" -ForegroundColor Yellow
            Write-Host "Failed to launch Pong.exe: $($_.Exception.Message)" -ForegroundColor Red
            Write-Host ""
            Write-Host "Cause: Windows Smart App Control (SAC) / Device Guard is blocking" -ForegroundColor Yellow
            Write-Host "       locally-compiled, unsigned binaries." -ForegroundColor Yellow
            Write-Host "Fix:" -ForegroundColor Cyan
            Write-Host "  1. Open Windows Settings -> Privacy & security -> Windows Security" -ForegroundColor White
            Write-Host "  2. Go to 'App & browser control' -> 'Smart App Control settings'" -ForegroundColor White
            Write-Host "  3. Set Smart App Control to 'Off' (or enable Windows Developer Mode)." -ForegroundColor White
            Write-Host "=====================================================================" -ForegroundColor Yellow
            Write-Host ""
        }
    }
}
finally {
    Pop-Location
}
