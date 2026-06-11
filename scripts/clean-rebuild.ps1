param(
    [string] $Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildDir = Join-Path $Root "build"

if (Test-Path $BuildDir) {
    Remove-Item -LiteralPath $BuildDir -Recurse -Force
}

& (Join-Path $Root "scripts\build.ps1") -Configuration $Configuration
