# TenEngine Project Memory

This file persists across Claude Code sessions. Keep it concise (under 200 lines).

## Project Overview

TenEngine is a modular C++17 game engine with 30 modules. Architecture follows Unreal-style extensibility with clear module boundaries.

## Key Architecture

### Module Dependency Chain
```
001-core → 002-object → 008-rhi → 009-render-core → 004-scene → 005-entity →
013-resource → 010-shader → 019-pipeline-core → 029-world → 028-texture →
011-material → 012-mesh → 020-pipeline
```

### Contract System
- **T0-contracts branch**: Single source of truth for all contracts
- **Location**: `specs/_contracts/`
- **API files**: `NNN-modulename-public-api.md` (capabilities, types)
- **ABI files**: `NNN-modulename-ABI.md` (namespaces, headers, symbols)
- **API is source of truth for ABI**

## Build Conventions

- **All dependencies use source-based inclusion** (add_subdirectory/FetchContent)
- **No pre-built libraries or stubs**
- Build root must be confirmed before cmake operations
- Reference: `docs/engine-build-module-convention.md`

## Git Rules

- **Commit messages in English only**
- Use Conventional Commits: `feat:`, `fix:`, `docs:`, `refactor:`
- Contract changes → push to T0-contracts branch

## Third-Party Integration

Follow 7-step workflow: `docs/third_party-integration-workflow.md`
1. Version selection → 2. **Auto-download (required)** → 3. Configure → 4. Install → 5. Test → 6. Deploy → 7. Implement

## Constitution Summary (v1.3.0)

1. **Modular Renderer**: Self-contained, testable modules
2. **Modern Graphics API**: Vulkan/D3D12/Metal first-class
3. **Data-Driven Pipeline**: Materials/meshes as data
4. **Performance**: Instrumentation, logging, measurable regressions
5. **Versioning**: MAJOR.MINOR.PATCH, ABI stability within MAJOR
6. **Contract-First**: Only declared interfaces allowed, full ABI implementation required

## Common Commands

- `/contracts-pull` - Pull from T0-contracts
- `/contracts-push` - Push to T0-contracts
- `/plan` - Generate implementation plan
- `/implement` - Execute tasks

## File Locations

| Type | Path |
|------|------|
| Contracts | `specs/_contracts/` |
| Module specs | `docs/module-specs/` |
| Templates | `.specify/templates/` |
| Scripts | `.specify/scripts/powershell/` |
| Build guide | `docs/engine-build-module-convention.md` |
