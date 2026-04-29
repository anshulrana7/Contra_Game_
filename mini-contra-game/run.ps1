param(
    [switch]$Rebuild,
    [switch]$BuildOnly
)

$ErrorActionPreference = "Stop"

$buildScript = Join-Path $PSScriptRoot "build.ps1"
$exePath = Join-Path $PSScriptRoot "mini_contra.exe"

if (-not (Test-Path $buildScript)) {
    throw "build.ps1 not found at $buildScript"
}

$needsBuild = $Rebuild -or -not (Test-Path $exePath)

if ($needsBuild) {
    Write-Host "Building project..."
    & $buildScript
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
}

if ($BuildOnly) {
    Write-Host "Build completed. Skipping launch because -BuildOnly was provided."
    exit 0
}

if (-not (Test-Path $exePath)) {
    throw "Executable not found at $exePath"
}

Write-Host "Launching mini_contra..."
& $exePath
