param(
    [string] $Configuration = "Release",
    [string] $Destination = "C:\Program Files\Common Files\VST3"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Plugin = Join-Path $Root "build\AcidLab303_artefacts\$Configuration\VST3\AcidLab303.vst3"
$Presets = Join-Path $Root "presets"

if (-not (Test-Path $Plugin)) {
    throw "Plugin not found at $Plugin. Run scripts\build.ps1 first."
}

if (-not (Test-Path (Join-Path $Plugin "Contents\x86_64-win"))) {
    throw "Invalid VST3 bundle at $Plugin. Copy/build the outer AcidLab303.vst3 folder, not the inner binary."
}

$Target = Join-Path $Destination "AcidLab303.vst3"
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
if (Test-Path $Target) {
    Remove-Item -LiteralPath $Target -Recurse -Force
}
Copy-Item -Path $Plugin -Destination $Target -Recurse -Force

if (Test-Path $Presets) {
    $PresetTarget = Join-Path $Destination "presets"
    if (Test-Path $PresetTarget) {
        Remove-Item -LiteralPath $PresetTarget -Recurse -Force
    }
    Copy-Item -Path $Presets -Destination $PresetTarget -Recurse -Force
}

if (-not (Test-Path (Join-Path $Target "Contents\x86_64-win"))) {
    throw "Install verification failed. $Target is not a valid VST3 bundle."
}

Write-Host "Installed AcidLab303 VST3 to $Target"
Write-Host "Installed AcidLab303 presets to $(Join-Path $Destination "presets")"
