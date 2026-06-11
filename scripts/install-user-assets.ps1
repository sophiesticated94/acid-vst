param(
    [string] $Destination = "$([Environment]::GetFolderPath('MyDocuments'))\AcidLab303"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$PresetSource = Join-Path $Root "presets"

if (-not (Test-Path $PresetSource)) {
    throw "Preset source not found at $PresetSource"
}

$PresetTarget = Join-Path $Destination "Presets"
$PatternTarget = Join-Path $Destination "Patterns"

New-Item -ItemType Directory -Force -Path $PresetTarget | Out-Null
New-Item -ItemType Directory -Force -Path $PatternTarget | Out-Null

Copy-Item -Path (Join-Path $PresetSource "factory\*.acidpreset.json") -Destination $PresetTarget -Force
if (Test-Path (Join-Path $PresetSource "patterns")) {
    Copy-Item -Path (Join-Path $PresetSource "patterns\*") -Destination $PatternTarget -Recurse -Force
}

Write-Host "Installed AcidLab303 user assets to $Destination"
