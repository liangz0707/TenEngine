# 启动 TE Graph Editor（确保 Cargo/Rust 在 PATH 中）
$cargoPath = Join-Path $env:USERPROFILE ".cargo\bin"
$gitPath = "C:\Program Files\Git\bin"
if (Test-Path $cargoPath) { $env:PATH = "$cargoPath;$env:PATH" }
if (Test-Path $gitPath)   { $env:PATH = "$gitPath;$env:PATH" }

$root = $PSScriptRoot
Set-Location $root

Write-Host "Starting TE Graph Editor from: $root" -ForegroundColor Cyan
Write-Host "If cargo was missing, it is now in PATH. Running: npm run tauri dev" -ForegroundColor Cyan
npm run tauri dev
