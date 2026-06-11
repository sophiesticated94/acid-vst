param(
    [string] $Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$PackageDir = Join-Path $Root "out\AcidLab303-$Configuration"
$Plugin = Join-Path $PackageDir "AcidLab303.vst3"
$Binary = Join-Path $Plugin "Contents\x86_64-win\AcidLab303.vst3"
$Readme = Join-Path $PackageDir "README.md"
$FactoryPresetDir = Join-Path $PackageDir "presets\factory"
$PatternDir = Join-Path $PackageDir "presets\patterns"

if (-not (Test-Path $Binary)) { throw "Missing VST3 binary at $Binary" }
if (-not (Test-Path $Readme)) { throw "Missing README at $Readme" }
if (-not (Test-Path $FactoryPresetDir)) { throw "Missing factory preset directory at $FactoryPresetDir" }
if (-not (Test-Path $PatternDir)) { throw "Missing pattern directory at $PatternDir" }

$PresetCount = @(Get-ChildItem -Path $FactoryPresetDir -Filter "*.acidpreset.json").Count
if ($PresetCount -lt 16) {
    throw "Expected at least 16 factory presets, found $PresetCount"
}

Write-Host "Package verified: $PackageDir"
