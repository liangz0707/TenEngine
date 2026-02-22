# Execute implementation planning workflow

## Purpose

Generate implementation plan with design artifacts for a feature.

## Steps

1. **Setup**: Run setup script
   ```powershell
   .specify/scripts/powershell/setup-plan.ps1 -Json
   ```
   Parse JSON for FEATURE_SPEC, IMPL_PLAN, SPECS_DIR, BRANCH.

2. **Load context**
   - Read FEATURE_SPEC and `.specify/memory/constitution.md`
   - Load IMPL_PLAN template

3. **Execute plan workflow**:
   - Fill Technical Context (mark unknowns as "NEEDS CLARIFICATION")
   - Fill Constitution Check section
   - **Build convention (TenEngine)**: If feature involves CMake, must fill "Dependency inclusion method" section
   - **Third-party deps (TenEngine)**: List each third-party with ID, inclusion method (header-only/source/sdk/system), doc link
   - Evaluate gates (ERROR if violations unjustified)
   - Phase 0: Generate research.md
   - Phase 1: Generate data-model.md, contracts/, quickstart.md
   - Update agent context

4. **Report**: Branch, IMPL_PLAN path, and generated artifacts

## Phase 0: Research

1. Extract unknowns from Technical Context
2. Research each NEEDS CLARIFICATION item
3. Consolidate findings in `research.md`:
   - Decision, Rationale, Alternatives considered

## Phase 1: Design & Contracts

1. Extract entities → `data-model.md`
2. Generate API/ABI design:
   - **Full ABI content**: Reference spec + Unity/Unreal docs for complete ABI
   - **Save only changes**: plan.md only saves new/modified entries
   - **Implementation uses full ABI**: tasks/implement must use full ABI (original + new + modified)
   - Output to `contracts/`
3. Run agent context update script

## Key Rules

- Use absolute paths
- ERROR on gate failures or unresolved clarifications
- **Build clarification**: Before cmake, must confirm build root directory
