# Create backup branches (backup/YYYY-MM-DD/<branch>) for master, T0-contracts, and all T0-NNN branches.
# Run from repo root. Pushes backup branches to origin.
#
# Usage: .\backup-branches.ps1 [-BackupDate "yyyy-MM-dd"]
# Example: .\backup-branches.ps1
#          .\backup-branches.ps1 -BackupDate "2026-01-29"
#
# Restore: use .\restore-from-backup.ps1 (see that script for -List, -Checkout, -Restore).

param(
    [string]$BackupDate = (Get-Date -Format "yyyy-MM-dd")
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot
for ($i = 0; $i -lt 3; $i++) { $RepoRoot = Split-Path $RepoRoot -Parent }
Set-Location $RepoRoot

$BranchesToBackup = @("master", "T0-contracts")
$Modules = @(
    "001-core", "002-object", "003-application", "004-scene", "005-entity", "006-input",
    "007-subsystems", "008-rhi", "009-render-core", "010-shader", "011-material", "012-mesh",
    "013-resource", "014-physics", "015-animation", "016-audio", "017-ui-core", "018-ui",
    "019-pipeline-core", "020-pipeline", "021-effects", "022-2d", "023-terrain", "024-editor",
    "025-tools", "026-networking", "027-xr"
)
foreach ($m in $Modules) { $BranchesToBackup += "T0-$m" }

$Prefix = "backup/$BackupDate"
Write-Host "=== Backup branches to ${Prefix}/* ==="
Write-Host "Date: $BackupDate  Repo: $RepoRoot"
Write-Host ""

$ea = $ErrorActionPreference
$ErrorActionPreference = "Continue"
cmd /c "git fetch origin 2>nul"
$ErrorActionPreference = $ea

$Created = 0
$Skipped = 0
foreach ($branch in $BranchesToBackup) {
    $backupBranch = "${Prefix}/${branch}"
    $ref = "origin/$branch"
    $exists = $false
    cmd /c "git rev-parse --verify $ref 2>nul"
    if ($LASTEXITCODE -eq 0) { $exists = $true }
    if (-not $exists) {
        Write-Host "Skip $branch (no origin/$branch)"
        $Skipped++
        continue
    }
    cmd /c "git branch $backupBranch $ref 2>nul"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Warn: could not create local $backupBranch"
        continue
    }
    cmd /c "git push origin $backupBranch 2>nul"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "OK  $backupBranch"
        $Created++
    } else {
        Write-Host "Fail push $backupBranch"
    }
}
Write-Host ""
Write-Host "Done. Created and pushed: $Created  Skipped: $Skipped"
