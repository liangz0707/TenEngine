# Merge T0-contracts into each T0-NNN worktree (contracts + docs + .cursor + .specify).
# Run from repo root. Worktrees must exist at G:\AIHUMAN\WorkSpaceSDD\TenEngine-NNN-modulename.

param(
    [string]$WorktreeBase = "G:\AIHUMAN\WorkSpaceSDD",
    [string[]]$OnlyModules = @(),  # e.g. @("002-object","003-application") to run only those
    [string]$MergeMsg = "Merge T0-contracts: 契约、docs、.cursor、.specify 更新"
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
if ($OnlyModules.Count -gt 0) { $Modules = $OnlyModules }

foreach ($m in $Modules) {
    $branch = "T0-$m"
    $wtPath = Join-Path $WorktreeBase "TenEngine-$m"
    if (-not (Test-Path $wtPath)) { Write-Warning "Skip ${branch}: worktree not found $wtPath"; continue }
    Write-Host "=== $branch at $wtPath ==="
    Push-Location $wtPath
    try {
        git fetch origin T0-contracts 2>&1 | Out-Null
        git merge origin/T0-contracts --allow-unrelated-histories -m $MergeMsg 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            git checkout --theirs specs/_contracts/000-module-dependency-map.md specs/_contracts/README.md specs/_contracts/pipeline-to-rci.md docs/module-specs/README.md 2>&1 | Out-Null
            foreach ($f in @("docs/contracts-and-specs-T0.md","docs/multi-agent-interface-sync.md","docs/tenengine-full-module-spec.md","docs/modules/README.md","docs/proposed-module-architecture.md","docs/three-engines-modules-and-dependencies.md","docs/reference-unity-unreal-modules.md",".cursor/rules/interface-sync.mdc",".specify/templates/spec-template.md")) {
                if (Test-Path $f) { git checkout --theirs $f 2>&1 | Out-Null }
            }
            git add specs/_contracts/ docs/module-specs/ docs/modules/ docs/contracts-and-specs-T0.md docs/multi-agent-interface-sync.md docs/tenengine-full-module-spec.md docs/proposed-module-architecture.md docs/three-engines-modules-and-dependencies.md docs/reference-unity-unreal-modules.md .cursor/rules/ .specify/templates/ 2>&1 | Out-Null
            git commit -m $MergeMsg 2>&1 | Out-Null
        }
        git push origin $branch 2>&1 | Out-Null
        if ($LASTEXITCODE -eq 0) { Write-Host "Pushed ${branch}" } else { Write-Host "Fail ${branch}" }
    } finally {
        Pop-Location
    }
}
Write-Host "Done."
