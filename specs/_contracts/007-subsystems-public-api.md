# Contract: 007-Subsystems Module Public API

## Applicable Modules

- **Implementer**: 007-Subsystems (L1; pluggable subsystems: descriptor, registration, lifecycle management; Display, XR, etc.)
- **Corresponding Spec**: `docs/module-specs/007-subsystems.md`
- **Dependencies**: 001-Core, 002-Object

## Consumers

- 027-XR
- Other modules requiring subsystem functionality

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| SubsystemDescriptor | Subsystem metadata: dependencies, priority, platform filter; typeInfo, name, version, dependencies, dependencyCount, priority, platformFilter, configData | Registered until unloaded |
| ISubsystem | Subsystem instance interface; lifecycle hooks: Initialize, Start, Stop, Shutdown; GetDescriptor, GetName | Consistent with Start/Stop |
| Registry | Subsystem registry singleton; manages subsystem registration and query | Application lifetime |
| Lifecycle | Subsystem lifecycle manager; responsible for batch lifecycle operations | Static class, no instance |
| Discovery | Plugin discovery and registration manager | Static class, no instance |
| SubsystemState | Subsystem state enumeration (Uninitialized, Initialized, Started, Stopped, Shutdown) | Enum type |
| RegisterResult | Registration operation result; contains success status and error message | Temporary value |
| LifecycleResult | Lifecycle operation result; contains success status, failed subsystem list, and error message | Temporary value |
| DiscoveryResult | Plugin discovery result; contains discovered subsystems and failed plugin list | Temporary value |
| GetSubsystem<T> | Query subsystem instance by type | Consistent with subsystem lifetime |
| PlatformFilter | Platform filter bits namespace: Windows, Linux, macOS, Android, iOS, All | Compile-time constants |
| SubsystemDescriptorBuilder | Builder for SubsystemDescriptor with fluent interface | Temporary value |

### Capabilities (Provider Guarantees)

| # | Capability | Description |
|---|------------|-------------|
| 1 | Descriptor | SubsystemDescriptor contains typeInfo, name, version, dependencies, dependencyCount, priority, platformFilter, configData; supports dependency declaration, priority sorting, platform filtering; serializable, reflectable (Object) |
| 2 | Registry | Registry singleton provides Register, GetSubsystem<T>, GetSubsystemByName, GetAllSubsystems, Unregister; supports type and name query; thread-safe; supports state query (GetSubsystemState, GetSubsystemStateByName) |
| 3 | Lifecycle Management | Lifecycle class provides InitializeAll, StartAll, StopAll, ShutdownAll; supports dependency topology sorting; supports single subsystem lifecycle operations (InitializeSubsystem, StartSubsystem, StopSubsystem, ShutdownSubsystem); supports dependency check (CheckDependencies); returns detailed operation results (LifecycleResult) |
| 4 | Plugin Discovery | Discovery class provides ScanPlugins, RegisterFromPlugin, UnregisterFromPlugin; supports scanning plugins from multiple paths; works with Core.ModuleLoad; returns detailed discovery results (DiscoveryResult) |
| 5 | State Management | Supports querying subsystem state (SubsystemState); supports state tracking (Uninitialized -> Initialized -> Started -> Stopped -> Shutdown) |
| 6 | Error Handling | RegisterResult, LifecycleResult, DiscoveryResult provide detailed error information and failure lists; supports partial failure scenarios |
| 7 | Helpers | SubsystemDescriptorBuilder for fluent descriptor construction; ValidateDescriptor for descriptor validation; PlatformFilter namespace for platform bits; MatchesCurrentPlatform, GetCurrentPlatformBits, IsValidForCurrentPlatform utilities |

## API Detailed Description

### Registry (Registry)

**Singleton Pattern**: Get instance via `Registry::GetInstance()`.

**Core Functions**:
- **Registration**: `RegisterInstance(SubsystemDescriptor, unique_ptr<ISubsystem>)` - Register subsystem, ownership transferred to registry
- **Query**: `GetSubsystem<T>()` - Query by type; `GetSubsystemByName(name)` - Query by name; `GetAllSubsystems()` - Get all subsystems
- **State**: `GetSubsystemState(typeInfo)` / `GetSubsystemStateByName(name)` - Query subsystem state
- **Unregister**: `Unregister(typeInfo)` - Unregister subsystem
- **Synchronization**: `Lock()` / `Unlock()` - External synchronization support
- **State Check**: `IsShutdown()` - Check if registry is in shutdown state

**Thread Safety**: All operations are protected by mutex.

### Lifecycle (Lifecycle)

**Static Class**: All methods are static, requires passing Registry reference.

**Batch Operations**:
- `InitializeAll(Registry&)` - Initialize all subsystems in dependency order
- `StartAll(Registry&)` - Start all subsystems in priority order
- `StopAll(Registry&)` - Stop all subsystems in reverse priority order
- `ShutdownAll(Registry&)` - Shutdown all subsystems in reverse dependency order

**Single Operations**:
- `InitializeSubsystem(Registry&, name)` - Initialize single subsystem
- `StartSubsystem(Registry&, name)` - Start single subsystem
- `StopSubsystem(Registry&, name)` - Stop single subsystem
- `ShutdownSubsystem(Registry&, name)` - Shutdown single subsystem

**Auxiliary Functions**:
- `CheckDependencies(Registry&, name)` - Check if dependencies are satisfied

**Error Handling**: All batch and single operations (except ShutdownAll/ShutdownSubsystem) return `LifecycleResult` containing success status, failed subsystem list, and error message.

### Discovery (Discovery)

**Static Class**: All methods are static, requires passing Registry reference.

**Core Functions**:
- `ScanPlugins(Registry&, pluginPaths, pathCount)` - Scan plugins from multiple paths and register subsystems
- `RegisterFromPlugin(Registry&, moduleHandle)` - Register subsystems from plugin module
- `UnregisterFromPlugin(Registry&, moduleHandle)` - Unregister subsystems from plugin module

**Error Handling**: `ScanPlugins` returns `DiscoveryResult` containing discovered subsystem list and failed plugin list.

### ISubsystem (Subsystem Interface)

**Lifecycle Hooks**:
- `Initialize()` - Initialize subsystem, returns success/failure
- `Start()` - Start subsystem
- `Stop()` - Stop subsystem
- `Shutdown()` - Shutdown subsystem

**Metadata Query**:
- `GetDescriptor()` - Get subsystem descriptor
- `GetName()` - Get subsystem name

### Helpers (te/subsystems/helpers.hpp)

**PlatformFilter Namespace**:
- `Windows = 1u`, `Linux = 2u`, `macOS = 4u`, `Android = 8u`, `iOS = 16u`, `All = 0u`

**Utility Functions**:
- `GetCurrentPlatformBits()` - Get current platform filter bits
- `MatchesCurrentPlatform(platformFilter)` - Check if platform filter matches current platform
- `ValidateDescriptor(desc, errorMsg)` - Validate subsystem descriptor
- `IsValidForCurrentPlatform(desc)` - Check if descriptor is valid for current platform

**SubsystemDescriptorBuilder Class**:
- `SetTypeInfo(typeInfo)`, `SetName(name)`, `SetVersion(version)`, `SetDependencies(deps, count)`
- `SetPriority(priority)`, `SetPlatformFilter(filter)`, `SetConfigData(config)`
- `Build()` - Returns constructed SubsystemDescriptor

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- **Current Version**: v1.1 (2026-02-22 code-aligned update)

## Constraints

- Must be used after Core and Object are available
- Subsystem start/stop order guaranteed by dependency graph
- Lifecycle operations should be called in main loop single-threaded environment
- Registry operations are thread-safe, but Lifecycle operations recommended in single-threaded environment
- XR, Display, etc. are implementations attached to this module

## Usage Example

```cpp
// Register subsystem
te::subsystems::SubsystemDescriptor desc{};
desc.typeInfo = &typeid(MySubsystem);
desc.name = "MySubsystem";
desc.dependencies = nullptr;
desc.dependencyCount = 0;
desc.priority = 0;
desc.platformFilter = 0;

auto subsystem = std::make_unique<MySubsystem>();
te::subsystems::Registry::Register(desc, std::move(subsystem));

// Batch lifecycle operations
auto& reg = te::subsystems::Registry::GetInstance();
auto initResult = te::subsystems::Lifecycle::InitializeAll(reg);
if (initResult.success) {
    auto startResult = te::subsystems::Lifecycle::StartAll(reg);
    // ...
}

// Query subsystem
MySubsystem* subsystem = te::subsystems::Registry::GetSubsystem<MySubsystem>();
if (subsystem) {
    // Use subsystem
}

// Stop and shutdown
te::subsystems::Lifecycle::StopAll(reg);
te::subsystems::Lifecycle::ShutdownAll(reg);
```

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 007-Subsystems contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-06 | Updated to match actual code implementation: Registry (was SubsystemRegistry), independent Lifecycle and Discovery classes, added state management and error handling, detailed API description |
| 2026-02-22 | Code-aligned update: added SubsystemDescriptorBuilder, PlatformFilter namespace, helper utilities (ValidateDescriptor, MatchesCurrentPlatform, GetCurrentPlatformBits, IsValidForCurrentPlatform); clarified header files |
