# Rust independent graph toolkit architecture

This document defines the lightweight, decoupled architecture for the graph toolkit.

## Design constraints

- Graph runtime must not depend on TenEngine runtime headers.
- Domain features are provided as plugins.
- TenEngine integration is isolated in adapter crates only.
- Asset format stays engine-neutral.

## Layer model

1. `graph-spec`
   - Graph asset schema (`GraphDoc`, `Node`, `Edge`, `Pin`).
   - JSON parsing and structural validation.
2. `graph-core`
   - Generic DAG compile (`compile_graph`) and cycle checks.
   - No domain logic.
3. `graph-plugin-api`
   - Domain plugin trait and registry.
   - Node kind validation.
4. `graph-domain-*`
   - Domain packs (FrameGraph, ShaderGraph now; ScriptGraph/AI later).
5. `graph-adapter-tenengine`
   - Optional bridge from neutral graph to TenEngine-specific data.

## Why this is lightweight

- Rust workspace can run without CMake.
- CLI tooling works with only JSON graph files.
- Core modules have no graphics/runtime dependency.

## Next iterations

- Add `graph-domain-script` and `graph-domain-ai-task`.
- Add binary graph format option (FlatBuffers/MessagePack).
- Add UI editor as separate app (Tauri + React Flow or egui).
- Move `tools/te-graph` to a standalone repository when stable.
