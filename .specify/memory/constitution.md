<!--
Sync Impact Report
==================
Version change: 1.0.0 → 1.1.0
Modified principles: Added VI. Module Boundaries & Contract-First (T0); renamed Development Workflow → Code Quality & Testing
Added sections: VI. Module Boundaries & Contract-First (T0); Code Quality & Testing (refined from Development Workflow)
Removed sections: None
Templates: unchanged; Constitution Check remains generic, aligned with contract-first and T0-contracts
Ref: docs/agent-interface-sync.md (T0 contract strategy)
Follow-up TODOs: None.
-->

# TenEngine Constitution

## Core Principles

### I. Modular Renderer Architecture

- The renderer MUST be composed of well-bounded modules (e.g. scene graph, materials, lighting, post-process).
- Each module MUST be self-contained, independently testable, and have a clear, documented purpose.
- No module may exist solely for organizational grouping; every module MUST own a distinct rendering or engine concern.
- **Rationale**: Enables Unreal-style extensibility, parallel development, and clear dependency boundaries.

### II. Modern Graphics API First

- The engine MUST target modern explicit graphics APIs (Vulkan, D3D12, Metal) as first-class backends.
- Fixed-function or legacy API assumptions MUST NOT leak into core design; resources and pipelines MUST be explicitly managed.
- Shader and pipeline definitions MUST be data-driven (e.g. SPIR-V / DXIL) and versioned with the engine.
- **Rationale**: Aligns with current industry direction and enables high performance and cross-platform consistency.

### III. Data-Driven Pipeline

- Materials, meshes, and pipeline configurations SHOULD be defined as data/assets where feasible; code changes MUST NOT be required for common content iterations.
- Asset and pipeline schemas MUST be versioned; breaking changes MUST be documented and migrated.
- **Rationale**: Supports artist and designer workflows and keeps engine code stable while content evolves.

### IV. Performance & Observability

- Frame budget and GPU/CPU timing instrumentation MUST be available in development builds; key metrics MUST be exposed for profiling.
- Structured logging MUST be used for renderer and engine events; performance regressions MUST be measurable and justified before merge.
- **Rationale**: Ensures real-time constraints are visible and regressions are caught early.

### V. Versioning & ABI Stability

- Public engine and renderer APIs MUST follow a clear versioning scheme (e.g. MAJOR.MINOR.PATCH); breaking changes MUST increment MAJOR and be documented with migration guidance.
- ABI stability for plugins or external integrations SHOULD be maintained within a MAJOR version where feasible.
- **Rationale**: Protects downstream users and tools from unexpected breakage and supports long-lived projects.

### VI. Module Boundaries & Contract-First (T0)

- **ABI and module boundaries** are defined by and MUST align with the **contracts** under `specs/_contracts/` (e.g. `NNN-modulename-public-api.md`). No module may expose types or interfaces that are not declared in its contract.
- **Single source of truth for contracts**: The **authoritative** source for all contract files is the **`T0-contracts`** branch. Master/main is for repo and build configuration only; it MUST NOT be used as the source for contract content.
- **Multi-Agent collaboration**: When working on any T0 module branch (e.g. `T0-001-core`, `T0-002-object`), agents MUST pull the latest contracts from **`origin/T0-contracts`** (e.g. `git fetch origin T0-contracts` then `git merge origin/T0-contracts`) before implementing or changing specs. Contract updates MUST be committed and pushed on the `T0-contracts` branch so other agents can pull them.
- **Implementation constraint**: Implementations MUST use only **types and interfaces that are declared in the relevant contracts**. Downstream modules MUST depend only on what is declared in their upstream contracts (e.g. `specs/_contracts/001-core-public-api.md`). No reliance on internal or undocumented APIs.
- **Rationale**: Ensures interface consistency across parallel work, avoids integration failures, and keeps a single place (T0-contracts) for cross-module API definitions. See `docs/agent-interface-sync.md` for the full T0 contract workflow.

## Technology Stack & Performance Standards

- **Language**: C++17 or later (C++20 preferred for modules and concepts where applicable).
- **Graphics**: Vulkan as primary reference backend; D3D12 and Metal support via abstraction where scope permits.
- **Shaders**: SPIR-V (Vulkan); equivalent intermediate or bytecode for other backends.
- **Build**: Single, documented build system (e.g. CMake); reproducible builds MUST be achievable.
- **Performance goals**: Domain-specific targets (e.g. frame time, draw call budget) MUST be stated in feature specs and plans; deviations MUST be justified.
- **Constraints**: Memory and threading constraints MUST be explicit in design docs where they affect public API or pipeline.

## Code Quality & Testing

- **Code quality**: All changes that touch renderer or engine core MUST pass a Constitution Check against this document before merge. Code review MUST verify compliance with Core Principles; violations MUST be resolved or explicitly exempted with documented rationale.
- **Testing conventions**: Unit tests for core data structures and algorithms; integration tests for pipeline and backend **contracts** (behavior implied by `specs/_contracts/`); visual/regression tests where specified. Public API surfaces MUST have tests that validate contract guarantees where feasible.
- **Performance**: Performance-sensitive paths MUST have benchmarks or profiling steps documented; regressions require justification or revert.
- **Complexity**: Complexity beyond the minimum necessary MUST be justified in design or PR description.

## Governance

- This constitution supersedes ad-hoc practices for TenEngine; when in conflict, the constitution takes precedence.
- Amendments require: (1) documented proposal, (2) version bump per semantic versioning (MAJOR/MINOR/PATCH), (3) update of this file and any dependent templates or guidance.
- All PRs and reviews MUST verify alignment with the principles above; exemptions MUST be recorded in the Complexity Tracking table (or equivalent) in the implementation plan.
- For day-to-day development guidance, use README.md or docs/quickstart.md when available.

**Version**: 1.1.0 | **Ratified**: 2025-01-28 | **Last Amended**: 2025-01-28
