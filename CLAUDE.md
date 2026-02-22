# TenEngine - Claude Code Project Configuration

This file configures Claude Code for the TenEngine project.

## Project Overview

TenEngine is a modular game engine written in C++17 with 30 modules. Key modules include:
- Core (001), Object (002), RHI (008), Render-Core (009)
- Scene (004), Entity (005), Resource (013), Shader (010)
- Pipeline-Core (019), Pipeline (020), Material (011), Mesh (012), Texture (028)

## Build System

- **CMake 3.16+** required
- **C++17** standard
- Build: `cmake -B build -S . && cmake --build build --config Release`
- All submodules use source-based dependency inclusion (add_subdirectory)
- Optional builds: `TENENGINE_BUILD_MESH`, `TENENGINE_BUILD_MATERIAL`, `TENENGINE_BUILD_TEXTURE`, `TENENGINE_BUILD_PIPELINE`, `TENENGINE_BUILD_EDITOR`

## Git Commit Rules

**All git commit messages must be written in English.**

- Use clear, imperative style (e.g. "Add X", "Fix Y", "Update Z")
- Follow Conventional Commits: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`
- Examples:
  - ✅ `feat(pipeline): add rendering pipeline core fixes`
  - ✅ `docs: sync interface-sync rule with T0-contracts`
  - ❌ `添加核心模块契约` (use English instead)

## Contract & API/ABI Rules

### 1. When modifying specs/_contracts

- **API files** (`specs/_contracts/NNN-modulename-public-api.md`) are the **source of truth** for ABI files
- **API content must not reference ABI** - no "see ABI" or "ABI is authoritative" statements
- **public-api is the only source** for ABI - do not derive public-api from ABI

### 2. When implementing (plan, task, implement)

- **First read upstream module API** (public-api) for capabilities and types
- **Then reference ABI** for concrete interfaces (namespace, headers, symbols)
- **Semantics from API, interfaces from ABI** - combine both, never rely only on ABI

## Interface Sync (Multi-Agent Collaboration)

- **Contracts live on T0-contracts branch** - `specs/_contracts/` is the single source of truth
- Before implementation or spec changes: **pull latest contracts** (see `/contracts-pull` command)
- Check `docs/module-specs/NNN-modulename.md` for Dependencies and Interface Contracts
- Dependency map: `specs/_contracts/000-module-dependency-map.md`

## Build Convention (Mandatory)

Per `docs/engine-build-module-convention.md`:

1. **All direct dependencies use source-based inclusion** (no pre-built libs)
2. **No hand-written add_subdirectory/find_package branches**
3. **No explicit linking of upstream to tests/executables**
4. When user says "build project" - **must confirm build root directory first**

## Third-Party Dependencies

If a module declares third-party libs in `specs/_contracts/NNN-modulename-public-api.md`:

1. Check `docs/third_party/<id>-<name>.md` for integration method
2. Follow 7-step workflow in `docs/third_party-integration-workflow.md`
3. **Never assume third-party exists** - always use FetchContent/git submodule/scripts

## Project Structure

```
Engine/
├── TenEngine-001-core/       # Foundation: memory, thread, platform, log, math
├── TenEngine-002-object/     # Object system
├── TenEngine-008-rhi/        # Render Hardware Interface
├── TenEngine-009-render-core/# Render core
├── TenEngine-013-resource/   # Resource management
├── TenEngine-019-pipeline-core/ # Pipeline core
├── TenEngine-020-pipeline/   # Rendering pipeline
├── TenEngine-028-texture/    # Texture system
├── TenEngine-011-material/   # Material system
├── TenEngine-012-mesh/       # Mesh system
└── ... (30 modules total)

specs/
├── _contracts/               # API/ABI contracts (single source of truth)
└── module-specs/             # Module specifications

docs/
├── agent-workflow-complete-guide.md
├── engine-build-module-convention.md
├── third_party-integration-workflow.md
└── ...
```

## Available Commands

Use `/command-name` to invoke:
- `/contracts-pull` - Pull latest contracts from T0-contracts
- `/contracts-push` - Push contract changes to T0-contracts
- `/contracts-writeback` - Write ABI/API changes back to contract files
- `/contracts-downstream-todo` - Add TODO compatibility notes to downstream modules
- `/plan` - Execute implementation planning workflow
- `/implement` - Execute implementation tasks
- `/tasks` - Generate task breakdown
- `/checklist` - Create validation checklist

## Code Style

- C++17 standard conventions
- Follow existing patterns in codebase
- Keep implementations minimal and focused
