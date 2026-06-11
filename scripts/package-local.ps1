param(
    [string] $Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Plugin = Join-Path $Root "build\AcidLab303_artefacts\$Configuration\VST3\AcidLab303.vst3"
$PackageDir = Join-Path $Root "out\AcidLab303-$Configuration"

if (-not (Test-Path (Join-Path $Plugin "Contents\x86_64-win"))) {
    throw "Plugin bundle not found or invalid at $Plugin. Run scripts\build.ps1 first."
}

if (Test-Path $PackageDir) {
    Remove-Item -LiteralPath $PackageDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null
Copy-Item -Path $Plugin -Destination (Join-Path $PackageDir "AcidLab303.vst3") -Recurse -Force
Copy-Item -Path (Join-Path $Root "presets") -Destination (Join-Path $PackageDir "presets") -Recurse -Force
Copy-Item -Path (Join-Path $Root "README.md") -Destination (Join-Path $PackageDir "README.md") -Force

Write-Host "Packaged AcidLab303 to $PackageDir"
