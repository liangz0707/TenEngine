# 007-Subsystems Module ABI

- **Contract**: [007-subsystems-public-api.md](./007-subsystems-public-api.md) (capabilities and types description)
- **This file**: 007-Subsystems external ABI explicit table.

## ABI Table

Column definitions: **Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description**

### Types and Enums

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | - | enum class | Subsystem state | te/subsystems/registry.hpp | SubsystemState | `enum class SubsystemState { Uninitialized, Initialized, Started, Stopped, Shutdown };` Subsystem lifecycle state |
| 007-Subsystems | te::subsystems | - | struct | Registration result | te/subsystems/registry.hpp | RegisterResult | `struct RegisterResult { bool success; char const* errorMessage; };` Registration operation result |
| 007-Subsystems | te::subsystems | - | struct | Subsystem descriptor | te/subsystems/descriptor.hpp | SubsystemDescriptor | `struct SubsystemDescriptor { void const* typeInfo; char const* name; char const* version; char const* const* dependencies; std::size_t dependencyCount; int priority; std::uint32_t platformFilter; void const* configData; };` Metadata, dependency list, priority, platform filter |
| 007-Subsystems | te::subsystems | - | struct | Lifecycle result | te/subsystems/lifecycle.hpp | LifecycleResult | `struct LifecycleResult { bool success; std::vector<char const*> failedSubsystems; char const* errorMessage; };` Lifecycle operation result |
| 007-Subsystems | te::subsystems | - | struct | Discovery result | te/subsystems/discovery.hpp | DiscoveryResult | `struct DiscoveryResult { bool success; std::vector<char const*> discoveredSubsystems; std::vector<char const*> failedPlugins; char const* errorMessage; };` Plugin discovery operation result |

### ISubsystem Interface (te/subsystems/subsystem.hpp)

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::Initialize | `bool Initialize() = 0;` Initialize subsystem; returns true on success, false on failure; called before Start() in dependency order |
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::Start | `void Start() = 0;` Start subsystem; called after Initialize() in dependency order |
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::Stop | `void Stop() = 0;` Stop subsystem; called before Shutdown() in reverse dependency order |
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::Shutdown | `void Shutdown() = 0;` Shutdown subsystem; called after Stop() in reverse dependency order |
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::GetDescriptor | `SubsystemDescriptor const& GetDescriptor() const = 0;` Get subsystem descriptor; must return valid reference |
| 007-Subsystems | te::subsystems | ISubsystem | abstract interface | Subsystem lifecycle interface | te/subsystems/subsystem.hpp | ISubsystem::GetName | `char const* GetName() const = 0;` Get subsystem name; must return non-null string |

### Registry Class (te/subsystems/registry.hpp)

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | Registry | singleton | Subsystem registry | te/subsystems/registry.hpp | Registry::GetInstance | `static Registry& GetInstance();` Get singleton instance |
| 007-Subsystems | te::subsystems | Registry | instance method | Register subsystem | te/subsystems/registry.hpp | Registry::RegisterInstance | `RegisterResult RegisterInstance(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);` Register subsystem, ownership transferred to registry |
| 007-Subsystems | te::subsystems | Registry | static method | Register subsystem (convenience) | te/subsystems/registry.hpp | Registry::Register | `static bool Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);` Returns false on failure (backward compatible) |
| 007-Subsystems | te::subsystems | Registry | template method | Get subsystem by type | te/subsystems/registry.hpp | Registry::GetSubsystem<T> | `template<typename T> static T* GetSubsystem();` Returns registered T*; nullptr if unregistered or after ShutdownAll |
| 007-Subsystems | te::subsystems | Registry | instance method | Get subsystem by name | te/subsystems/registry.hpp | Registry::GetSubsystemByName | `ISubsystem* GetSubsystemByName(char const* name);` Query subsystem by name; nullptr if not found or after ShutdownAll |
| 007-Subsystems | te::subsystems | Registry | instance method | Get all subsystems | te/subsystems/registry.hpp | Registry::GetAllSubsystems | `std::vector<ISubsystem*> GetAllSubsystems();` Returns all registered subsystems list |
| 007-Subsystems | te::subsystems | Registry | instance method | Get subsystem state | te/subsystems/registry.hpp | Registry::GetSubsystemState | `SubsystemState GetSubsystemState(void const* typeInfo) const;` Query subsystem state by typeInfo |
| 007-Subsystems | te::subsystems | Registry | instance method | Get subsystem state by name | te/subsystems/registry.hpp | Registry::GetSubsystemStateByName | `SubsystemState GetSubsystemStateByName(char const* name) const;` Query subsystem state by name |
| 007-Subsystems | te::subsystems | Registry | static method | Unregister subsystem | te/subsystems/registry.hpp | Registry::Unregister | `static void Unregister(void const* typeInfo);` Unregister subsystem by typeInfo |
| 007-Subsystems | te::subsystems | Registry | instance method | Lock registry | te/subsystems/registry.hpp | Registry::Lock | `void Lock();` Lock registry mutex (for external synchronization) |
| 007-Subsystems | te::subsystems | Registry | instance method | Unlock registry | te/subsystems/registry.hpp | Registry::Unlock | `void Unlock();` Unlock registry mutex |
| 007-Subsystems | te::subsystems | Registry | instance method | Check shutdown state | te/subsystems/registry.hpp | Registry::IsShutdown | `bool IsShutdown() const;` Check if registry is in shutdown state |

### Lifecycle Class (te/subsystems/lifecycle.hpp)

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | Lifecycle | static method | Initialize all subsystems | te/subsystems/lifecycle.hpp | Lifecycle::InitializeAll | `static LifecycleResult InitializeAll(Registry& reg);` Initialize all subsystems in dependency order; returns result with failure list |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Start all subsystems | te/subsystems/lifecycle.hpp | Lifecycle::StartAll | `static LifecycleResult StartAll(Registry& reg);` Start all subsystems in priority order; returns result with failure list |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Stop all subsystems | te/subsystems/lifecycle.hpp | Lifecycle::StopAll | `static LifecycleResult StopAll(Registry& reg);` Stop all subsystems in reverse priority order; returns result with failure list |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Shutdown all subsystems | te/subsystems/lifecycle.hpp | Lifecycle::ShutdownAll | `static void ShutdownAll(Registry& reg);` Shutdown all subsystems in reverse dependency order |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Initialize single subsystem | te/subsystems/lifecycle.hpp | Lifecycle::InitializeSubsystem | `static LifecycleResult InitializeSubsystem(Registry& reg, char const* name);` Initialize single subsystem by name |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Start single subsystem | te/subsystems/lifecycle.hpp | Lifecycle::StartSubsystem | `static LifecycleResult StartSubsystem(Registry& reg, char const* name);` Start single subsystem by name |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Stop single subsystem | te/subsystems/lifecycle.hpp | Lifecycle::StopSubsystem | `static LifecycleResult StopSubsystem(Registry& reg, char const* name);` Stop single subsystem by name |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Shutdown single subsystem | te/subsystems/lifecycle.hpp | Lifecycle::ShutdownSubsystem | `static void ShutdownSubsystem(Registry& reg, char const* name);` Shutdown single subsystem by name |
| 007-Subsystems | te::subsystems | Lifecycle | static method | Check dependencies | te/subsystems/lifecycle.hpp | Lifecycle::CheckDependencies | `static bool CheckDependencies(Registry& reg, char const* subsystemName);` Check if all dependencies are registered and initialized |

### Discovery Class (te/subsystems/discovery.hpp)

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | Discovery | static method | Scan plugins | te/subsystems/discovery.hpp | Discovery::ScanPlugins | `static DiscoveryResult ScanPlugins(Registry& reg, char const* const* pluginPaths, size_t pathCount);` Scan plugins from multiple paths and register subsystems; returns discovered subsystems and failed plugins |
| 007-Subsystems | te::subsystems | Discovery | static method | Scan plugins (backward compat) | te/subsystems/discovery.hpp | Discovery::ScanPlugins | `static bool ScanPlugins(Registry& reg);` Backward compatibility - no-op |
| 007-Subsystems | te::subsystems | Discovery | static method | Register from plugin | te/subsystems/discovery.hpp | Discovery::RegisterFromPlugin | `static bool RegisterFromPlugin(Registry& reg, void* moduleHandle);` Register subsystems from plugin module; moduleHandle is Core module handle |
| 007-Subsystems | te::subsystems | Discovery | static method | Unregister from plugin | te/subsystems/discovery.hpp | Discovery::UnregisterFromPlugin | `static bool UnregisterFromPlugin(Registry& reg, void* moduleHandle);` Unregister subsystems from plugin module; should be called before unloading plugin |

### Helpers (te/subsystems/helpers.hpp)

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::Windows | `constexpr std::uint32_t Windows = 1u;` |
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::Linux | `constexpr std::uint32_t Linux = 2u;` |
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::macOS | `constexpr std::uint32_t macOS = 4u;` |
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::Android | `constexpr std::uint32_t Android = 8u;` |
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::iOS | `constexpr std::uint32_t iOS = 16u;` |
| 007-Subsystems | te::subsystems | PlatformFilter | namespace | Platform filter bits | te/subsystems/helpers.hpp | PlatformFilter::All | `constexpr std::uint32_t All = 0u;` All platforms |
| 007-Subsystems | te::subsystems | - | inline function | Get current platform bits | te/subsystems/helpers.hpp | GetCurrentPlatformBits | `inline std::uint32_t GetCurrentPlatformBits();` Returns current platform filter bits |
| 007-Subsystems | te::subsystems | - | inline function | Matches current platform | te/subsystems/helpers.hpp | MatchesCurrentPlatform | `inline bool MatchesCurrentPlatform(std::uint32_t platformFilter);` True if filter is 0 (all platforms) or matches current |
| 007-Subsystems | te::subsystems | - | inline function | Validate descriptor | te/subsystems/helpers.hpp | ValidateDescriptor | `inline bool ValidateDescriptor(SubsystemDescriptor const& desc, char const** errorMsg = nullptr);` Returns true if descriptor is valid |
| 007-Subsystems | te::subsystems | - | inline function | Is valid for current platform | te/subsystems/helpers.hpp | IsValidForCurrentPlatform | `inline bool IsValidForCurrentPlatform(SubsystemDescriptor const& desc);` True if descriptor platform filter matches |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | class | Descriptor builder | te/subsystems/helpers.hpp | SubsystemDescriptorBuilder | Fluent interface for constructing SubsystemDescriptor |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set typeInfo | te/subsystems/helpers.hpp | SetTypeInfo | `SubsystemDescriptorBuilder& SetTypeInfo(void const* typeInfo);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set name | te/subsystems/helpers.hpp | SetName | `SubsystemDescriptorBuilder& SetName(char const* name);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set version | te/subsystems/helpers.hpp | SetVersion | `SubsystemDescriptorBuilder& SetVersion(char const* version);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set dependencies | te/subsystems/helpers.hpp | SetDependencies | `SubsystemDescriptorBuilder& SetDependencies(char const* const* deps, std::size_t count);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set priority | te/subsystems/helpers.hpp | SetPriority | `SubsystemDescriptorBuilder& SetPriority(int priority);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set platform filter | te/subsystems/helpers.hpp | SetPlatformFilter | `SubsystemDescriptorBuilder& SetPlatformFilter(std::uint32_t filter);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Set config data | te/subsystems/helpers.hpp | SetConfigData | `SubsystemDescriptorBuilder& SetConfigData(void const* config);` |
| 007-Subsystems | te::subsystems | SubsystemDescriptorBuilder | member | Build descriptor | te/subsystems/helpers.hpp | Build | `SubsystemDescriptor Build() const;` Returns constructed descriptor |

*Source: User Story US-001 (Application starts and enters main loop). Contract capabilities: Descriptor, Registry, Lifecycle (Initialize/Start/Stop/Shutdown), Discovery.*

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 007-Subsystems initial ABI |
| 2026-02-06 | Updated to match actual code implementation: Registry (was SubsystemRegistry), Initialize (was Init), independent Lifecycle class, added state management and error handling |
| 2026-02-22 | Code-aligned update: added helpers.hpp (SubsystemDescriptorBuilder, PlatformFilter namespace, utility functions), clarified header files |
