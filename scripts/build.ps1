param(
    [string] $Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildDir = Join-Path $Root "build"
$VsDevCmd = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat"
$CMake = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$Ninja = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

$ProcessPath = [Environment]::GetEnvironmentVariable("Path", "Process")
if ([string]::IsNullOrWhiteSpace($ProcessPath)) {
    $ProcessPath = [Environment]::GetEnvironmentVariable("PATH", "Process")
}
[Environment]::SetEnvironmentVariable("PATH", $null, "Process")
[Environment]::SetEnvironmentVariable("Path", $ProcessPath, "Process")

if (-not (Test-Path $CMake)) {
    $CMake = "cmake"
}
if (-not (Test-Path $Ninja)) {
    $Ninja = "ninja"
}

$Cache = Join-Path $BuildDir "CMakeCache.txt"
if (Test-Path $Cache) {
    $GeneratorLine = Select-String -Path $Cache -Pattern "CMAKE_GENERATOR:INTERNAL=" -ErrorAction SilentlyContinue
    if ($GeneratorLine -and ($GeneratorLine.Line -notlike "*Ninja*")) {
        Remove-Item -LiteralPath $BuildDir -Recurse -Force
    }
}

if (Test-Path $VsDevCmd) {
    cmd /c "`"$VsDevCmd`" -arch=x64 -host_arch=x64 && `"$CMake`" -S `"$Root`" -B `"$BuildDir`" -G Ninja -DCMAKE_MAKE_PROGRAM=`"$Ninja`" -DCMAKE_BUILD_TYPE=$Configuration"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cmd /c "`"$VsDevCmd`" -arch=x64 -host_arch=x64 && `"$CMake`" --build `"$BuildDir`""
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cmd /c "`"$VsDevCmd`" -arch=x64 -host_arch=x64 && `"$CMake`" --build `"$BuildDir`" --target AcidLab303Tests"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cmd /c "`"$VsDevCmd`" -arch=x64 -host_arch=x64 && `"$CMake`" --build `"$BuildDir`" --target test"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    & $CMake -S $Root -B $BuildDir -G Ninja -DCMAKE_MAKE_PROGRAM="$Ninja" -DCMAKE_BUILD_TYPE=$Configuration
    & $CMake --build $BuildDir
    & $CMake --build $BuildDir --target AcidLab303Tests
    & $CMake --build $BuildDir --target test
}

Write-Host "Built AcidLab303. Look in: $BuildDir\AcidLab303_artefacts\$Configuration\VST3"
