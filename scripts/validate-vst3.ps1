param(
    [string] $Configuration = "Release",
    [string] $Validator = "vst3validator.exe"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Plugin = Join-Path $Root "build\AcidLab303_artefacts\$Configuration\VST3\AcidLab303.vst3"

if (-not (Test-Path (Join-Path $Plugin "Contents\x86_64-win"))) {
    throw "Plugin bundle not found or invalid at $Plugin. Run scripts\build.ps1 first."
}

$ValidatorCommand = Get-Command $Validator -ErrorAction SilentlyContinue
if ($null -eq $ValidatorCommand) {
    Write-Host "vst3validator.exe not found. Skipping VST3 validation."
    exit 0
}

& $ValidatorCommand.Source $Plugin
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "VST3 validation passed for $Plugin"
