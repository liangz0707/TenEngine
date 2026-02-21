# Contract: 025-Tools Module Public API

## Status: **TO BE IMPLEMENTED**

## Applicable Module

- **Implementer**: 025-Tools (L4; build, batch processing, CLI, plugin/package management; on-demand dependencies)
- **Specification**: `docs/module-specs/025-tools.md`
- **Dependencies**: On-demand (typically Core, Object, Resource, etc.)

## Consumers

- None (L4 consumer side; provides build artifacts, batch processing, CLI, and plugin lists to users/CI)

## Capability List

### Types and Handles (Cross-Boundary)

This module does not provide cross-boundary types to engine runtime modules; the following are abstractions exposed to users/CI/scripts.

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| BuildConfig | Project compilation, dependencies, multi-target, CMake/MSBuild integration | Single build |
| BatchJob | Batch import, batch processing, Resource integration | Single batch operation |
| CLICommand | Command line, subcommands, offline API | Single invocation |
| PluginDescriptor | Plugin/package discovery, version, dependencies, ModuleLoad integration | Managed by Tools or package manager |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Build | BuildConfig, multi-target, CMake/MSBuild integration |
| 2 | Batch Processing | BatchJob, batch import, Resource integration |
| 3 | CLI | CLICommand, subcommands, offline API |
| 4 | Plugin/Package Management | PluginDescriptor, discovery, version, dependencies, ModuleLoad integration |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- **Current Status**: Implementation pending. No public headers available.

## Constraints

- On-demand dependencies; runtime may query build information or plugin list through read-only interfaces defined in this contract.

## Implementation Notes

The module directory `Engine/TenEngine-025-tools/include/` currently contains no header files.
The following interfaces are planned but not yet implemented:

- `te/tools/Build.h` - IBuildSystem interface
- `te/tools/Batch.h` - IBatchProcessor interface
- `te/tools/CLI.h` - CLI functions and types
- `te/tools/PluginManager.h` - IPluginManager interface

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 025-Tools contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |
