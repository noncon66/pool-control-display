$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$env:PLATFORMIO_CORE_DIR = Join-Path $projectRoot ".pio\bringup-core"

$pio = Get-Command pio -ErrorAction SilentlyContinue
if (-not $pio)
{
    $fallback = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\pio.exe"
    if (-not (Test-Path -LiteralPath $fallback))
    {
        throw "PlatformIO was not found. Install PlatformIO before running this script."
    }
    $pio = $fallback
}

Push-Location $projectRoot
try
{
    & $pio run -e esp32-s3-display-bringup
    if ($LASTEXITCODE -ne 0)
    {
        throw "Display bring-up build failed with exit code $LASTEXITCODE."
    }
}
finally
{
    Pop-Location
}
