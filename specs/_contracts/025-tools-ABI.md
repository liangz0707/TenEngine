# 025-Tools Module ABI

## Status: **TO BE IMPLEMENTED**

- **Contract**: [025-tools-public-api.md](./025-tools-public-api.md) (Capabilities and types description)
- **This Document**: 025-Tools external ABI explicit table.
- **Reference**: Unity Build/Asset Pipeline, UE Build/Editor Tools; build, batch processing, CLI, plugin/package management. This module **does not provide cross-boundary types to engine runtime modules**; exposes abstractions to users/CI/scripts.
- **Naming**: Member methods use **PascalCase**; Description column provides **complete function signatures**.

## Implementation Status

The module directory `Engine/TenEngine-025-tools/include/` currently contains no header files.
All interfaces listed below are planned but not yet implemented.

## ABI Table (Planned)

Column Definition: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Build (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 025-Tools | te::tools | IBuildSystem | Abstract Interface | Configure | te/tools/Build.h | IBuildSystem::Configure | `bool Configure(BuildConfig const& config);` Project compilation, dependencies, multi-target; CMake/MSBuild integration |
| 025-Tools | te::tools | IBuildSystem | Abstract Interface | Compile | te/tools/Build.h | IBuildSystem::Compile | `bool Compile();` Single build |
| 025-Tools | te::tools | — | Struct | Build Config | te/tools/Build.h | BuildConfig | Target platform, dependencies, options; single build |
| 025-Tools | te::tools | — | Free Function/Factory | Get Build System | te/tools/Build.h | GetBuildSystem | `IBuildSystem* GetBuildSystem();` Optional; provided by Tools or main program |

### Batch Processing (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 025-Tools | te::tools | IBatchProcessor | Abstract Interface | Batch Import | te/tools/Batch.h | IBatchProcessor::BatchImport | `bool BatchImport(char const* inputDir, char const* outputDir, BatchJob const& job);` Resource integration; single batch operation |
| 025-Tools | te::tools | IBatchProcessor | Abstract Interface | Batch Process | te/tools/Batch.h | IBatchProcessor::BatchProcess | `bool BatchProcess(BatchJob const& job);` Single batch operation |
| 025-Tools | te::tools | — | Struct | Batch Job | te/tools/Batch.h | BatchJob | Input/output, options; single batch operation |

### CLI (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 025-Tools | te::tools | — | Free Function | Parse Args | te/tools/CLI.h | ParseArgs | `CLIResult ParseArgs(int argc, char const** argv);` Command line, subcommands |
| 025-Tools | te::tools | — | Free Function | Run Command | te/tools/CLI.h | RunCommand | `int RunCommand(char const* name, CLIResult const& args);` Offline API; single invocation |
| 025-Tools | te::tools | — | Struct | CLI Result | te/tools/CLI.h | CLIResult | Subcommands, options; single invocation |
| 025-Tools | te::tools | — | Free Function | Offline API | te/tools/CLI.h | InvokeOfflineAPI | `int InvokeOfflineAPI(char const* apiName, void const* input, void* output);` Optional; single invocation |

### Package Management (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 025-Tools | te::tools | IPluginManager | Abstract Interface | Discover Plugins | te/tools/PluginManager.h | IPluginManager::DiscoverPlugins | `void DiscoverPlugins(char const* directory);` ModuleLoad integration |
| 025-Tools | te::tools | IPluginManager | Abstract Interface | Resolve Dependencies | te/tools/PluginManager.h | IPluginManager::ResolveDeps | `bool ResolveDeps(PluginDescriptor const* plugin);` |
| 025-Tools | te::tools | IPluginManager | Abstract Interface | Load Plugin | te/tools/PluginManager.h | IPluginManager::LoadPlugin | `void* LoadPlugin(char const* path);` Core.ModuleLoad integration |
| 025-Tools | te::tools | — | Struct | Plugin Descriptor | te/tools/PluginManager.h | PluginDescriptor | Version, dependencies; managed by Tools or package manager |

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 025-Tools ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |

*Source: Contract capabilities Build, Batch, CLI, PackageManager; Reference: Unity Build, UE Build/Tools.*
