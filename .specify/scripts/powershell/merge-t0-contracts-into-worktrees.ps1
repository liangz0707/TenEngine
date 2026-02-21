# Merge T0-contracts into each T0-NNN worktree (contracts + docs + .cursor + .specify).
# Run from repo root. Worktrees must exist at <WorktreeBase>/TenEngine-NNN-modulename.

param(
    [string]$WorktreeBase = "..",  # Relative to repo root; looks for worktrees at ../TenEngine-NNN-modulename
    [string[]]$OnlyModules = @(),  # e.g. @("002-object","003-application") to run only those
    [string]$MergeMsg = "Merge T0-contracts: contracts, docs, .cursor, .specify updates"
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
    $ea = $ErrorActionPreference
    Push-Location $wtPath
    try {
        $ErrorActionPreference = 'Continue'
        cmd /c "git fetch origin T0-contracts 2>nul"
        cmd /c "git merge origin/T0-contracts --allow-unrelated-histories -m `"$MergeMsg`" 2>nul"
        $mergeExit = $LASTEXITCODE
        if ($mergeExit -ne 0) {
            git checkout --theirs specs/_contracts/000-module-dependency-map.md specs/_contracts/README.md specs/_contracts/pipeline-to-rci.md docs/module-specs/README.md 2>$null
            foreach ($f in @("docs/README.md","docs/agent-contracts-and-specs-T0.md","docs/agent-interface-sync.md","docs/engine-modules-and-architecture.md","docs/engine-build-module-convention.md","docs/agent-docs-as-assets-codegen.md","docs/research/README.md","docs/research/engine-reference-unity-unreal-modules.md","docs/research/engine-proposed-module-architecture.md","docs/agent-workflow-complete-guide.md","docs/agent-build-guide.md",".cursor/commands/speckit.implement.md",".cursor/rules/interface-sync.mdc",".cursor/rules/git-commit-messages.mdc",".specify/templates/spec-template.md",".specify/memory/constitution.md")) {
                if (Test-Path $f) { git checkout --theirs $f 2>$null }
            }
            git add specs/_contracts/ docs/module-specs/ docs/research/ docs/README.md docs/agent-contracts-and-specs-T0.md docs/agent-interface-sync.md docs/engine-modules-and-architecture.md docs/engine-build-module-convention.md docs/agent-docs-as-assets-codegen.md docs/agent-workflow-complete-guide.md docs/agent-build-guide.md .cursor/commands/ .cursor/rules/ .specify/templates/ .specify/memory/ 2>$null
            cmd /c "git commit -m `"$MergeMsg`" 2>nul"
        }
        cmd /c "git push origin $branch 2>nul"
        if ($LASTEXITCODE -eq 0) { Write-Host "Pushed ${branch}" } else { Write-Host "Fail ${branch}" }
    } finally {
        $ErrorActionPreference = $ea
        Pop-Location
    }
}
Write-Host "Done."
