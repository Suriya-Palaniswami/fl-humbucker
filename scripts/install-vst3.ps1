# Install FL Humbucker VST3 into the system VST3 folder (run as Administrator).
$ErrorActionPreference = "Stop"

$src = Join-Path $PSScriptRoot "..\build\fl_humbucker_artefacts\Release\VST3\FL Humbucker.vst3"
$destRoot = "C:\Program Files\Common Files\VST3"

if (-not (Test-Path -LiteralPath $src)) {
    Write-Error "Built plugin not found at:`n  $src`nBuild Release first."
}

New-Item -ItemType Directory -Path $destRoot -Force | Out-Null
$dest = Join-Path $destRoot "FL Humbucker.vst3"
if (Test-Path -LiteralPath $dest) {
    Remove-Item -LiteralPath $dest -Recurse -Force
}

Copy-Item -LiteralPath $src -Destination $destRoot -Recurse -Force
Write-Host "Installed:`n  $dest"
Write-Host "Rescan plugins in FL Studio (Options > Manage plugins)."
