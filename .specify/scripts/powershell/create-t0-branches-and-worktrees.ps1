# Create T0-NNN-modulename branches (constraints + module doc + global dependency only)
# and add worktrees under G:\AIHUMAN\WorkSpaceSDD\TenEngine-NNN-modulename
# Run from repo root (TenEngine). Requires: specs/_contracts/, docs/dependency-graph-full.md, docs/module-specs/*.md exist.

param(
    [switch]$BranchesOnly,
    [switch]$WorktreesOnly,
    [string]$WorktreeBase = "G:\AIHUMAN\WorkSpaceSDD"
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot
for ($i = 0; $i -lt 3; $i++) { $RepoRoot = Split-Path $RepoRoot -Parent }
Set-Location $RepoRoot

$Modules = @(
    "001-core", "002-object", "003-application", "004-scene", "005-entity", "006-input",
    "007-subsystems", "008-rhi", "009-render-core", "010-shader", "011-material", "012-mesh",
    "013-resource", "014-physics", "015-animation", "016-audio", "017-ui-core", "018-ui",
    "019-pipeline-core", "020-pipeline", "021-effects", "022-2d", "023-terrain", "024-editor",
    "025-tools", "026-networking", "027-xr"
)

foreach ($m in $Modules) {
    $branch = "T0-$m"
    $docFile = "docs/module-specs/$m.md"
    if (-not (Test-Path $docFile)) { Write-Warning "Skip ${branch} missing $docFile"; continue }

    if (-not $WorktreesOnly) {
        cmd /c "git checkout --orphan $branch 2>nul"
        cmd /c "git rm -rf --cached . 2>nul"

        $displayName = $m -replace "^(\d+)-", "`$1-"
        @"
# Branch: $branch

This branch contains only **constraint files**, **module description**, and **global dependency** for module **$displayName**.

- **Constraints**: ``specs/_contracts/``
- **Module description**: ``docs/module-specs/$m.md``
- **Global dependency**: ``docs/dependency-graph-full.md``, ``specs/_contracts/000-module-dependency-map.md``

"@ | Set-Content -Path "README.md" -Encoding UTF8 -NoNewline

        git add README.md
        git add specs/_contracts/
        git add docs/dependency-graph-full.md
        git add docs/module-specs/README.md
        git add $docFile
        cmd /c "git commit -m ""T0-$m constraints, module spec, global dependency"" 2>nul"
        cmd /c "git checkout master 2>nul"
        Write-Host "Created branch $branch"
    }

    if (-not $BranchesOnly) {
        $wtPath = Join-Path $WorktreeBase "TenEngine-$m"
        if (Test-Path $wtPath) {
            Write-Host "Worktree exists: $wtPath"
        } else {
            git worktree add $wtPath $branch
            Write-Host "Added worktree: $wtPath"
        }
    }
}

Write-Host "Done."
