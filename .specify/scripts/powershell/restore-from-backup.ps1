# List backup branches or restore from a backup (created by backup-branches.ps1).
# Run from repo root.
#
# Usage:
#   .\restore-from-backup.ps1 -List                              # list available backup dates
#   .\restore-from-backup.ps1 -List -BackupDate "2026-01-29"     # list branches in that backup
#   .\restore-from-backup.ps1 -Checkout -BackupDate "2026-01-29" -Branch master   # read-only: create backup-view-master
#   .\restore-from-backup.ps1 -Restore -BackupDate "2026-01-29" -Branch master    # destructive: reset current branch to backup

param(
    [switch]$List,
    [switch]$Checkout,
    [switch]$Restore,
    [string]$BackupDate = "",
    [string]$Branch = ""
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot
for ($i = 0; $i -lt 3; $i++) { $RepoRoot = Split-Path $RepoRoot -Parent }
Set-Location $RepoRoot

$ea = $ErrorActionPreference
$ErrorActionPreference = "Continue"
cmd /c "git fetch origin 2>nul"
$ErrorActionPreference = $ea

function Get-BackupDates {
    $refs = git branch -r | Where-Object { $_ -match "origin/backup/(\d{4}-\d{2}-\d{2})/" }
    $dates = @{}
    foreach ($r in $refs) {
        if ($r -match "origin/backup/(\d{4}-\d{2}-\d{2})/") { $dates[$Matches[1]] = $true }
    }
    $dates.Keys | Sort-Object -Descending
}

if ($List) {
    if ($BackupDate) {
        $prefix = "backup/$BackupDate"
        Write-Host "Backup branches for date: $BackupDate"
        git branch -r | Where-Object { $_ -match "origin/$prefix/" } | ForEach-Object { $_.Trim().Replace("origin/", "") }
    } else {
        Write-Host "Available backup dates (backup/YYYY-MM-DD/*):"
        Get-BackupDates | ForEach-Object { Write-Host "  $_" }
    }
    exit 0
}

if ($Checkout) {
    if (-not $BackupDate -or -not $Branch) {
        Write-Host "Usage: -Checkout -BackupDate YYYY-MM-DD -Branch <branch>"
        exit 1
    }
    $backupBranch = "backup/$BackupDate/$Branch"
    cmd /c "git rev-parse --verify origin/$backupBranch 2>nul"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Backup branch not found: $backupBranch"
        exit 1
    }
    cmd /c "git checkout -B backup-view-$Branch origin/$backupBranch 2>nul"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Checked out backup as local branch: backup-view-$Branch (read-only view)"
    } else {
        cmd /c "git checkout origin/$backupBranch 2>nul"
        if ($LASTEXITCODE -eq 0) { Write-Host "Detached HEAD at $backupBranch" }
    }
    exit 0
}

if ($Restore) {
    if (-not $BackupDate -or -not $Branch) {
        Write-Host "Usage: -Restore -BackupDate YYYY-MM-DD -Branch <branch>"
        exit 1
    }
    $backupBranch = "backup/$BackupDate/$Branch"
    cmd /c "git rev-parse --verify origin/$backupBranch 2>nul"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Backup branch not found: $backupBranch"
        exit 1
    }
    $current = git branch --show-current
    if ($current -ne $Branch) {
        Write-Host "Current branch is '$current'. Switch to '$Branch' first, then run -Restore again."
        exit 1
    }
    $confirm = Read-Host "Reset branch '$Branch' to backup $BackupDate? Uncommitted changes will be lost. (y/N)"
    if ($confirm -ne "y" -and $confirm -ne "Y") { Write-Host "Aborted."; exit 0 }
    cmd /c "git reset --hard origin/$backupBranch 2>nul"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Restored $Branch to backup $BackupDate"
    } else {
        Write-Host "Restore failed"
        exit 1
    }
    exit 0
}

Write-Host @"
Usage:
  -List                          List available backup dates
  -List -BackupDate YYYY-MM-DD   List branches in that backup
  -Checkout -BackupDate YYYY-MM-DD -Branch <name>   Checkout backup (read-only: creates backup-view-<name>)
  -Restore -BackupDate YYYY-MM-DD -Branch <name>     Reset current branch to backup (destructive, prompts)
"@
