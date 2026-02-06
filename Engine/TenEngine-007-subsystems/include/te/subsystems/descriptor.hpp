/**
 * @file descriptor.hpp
 * @brief SubsystemDescriptor (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_DESCRIPTOR_HPP
#define TE_SUBSYSTEMS_DESCRIPTOR_HPP

#include <cstddef>
#include <cstdint>

namespace te {
namespace subsystems {

/**
 * Subsystem metadata: dependencies, priority, platform filter.
 * typeInfo: corresponds to Object TypeId or TypeDescriptor.
 * name: subsystem name for querying and logging (must be non-null and unique).
 * version: optional version string (can be nullptr).
 * dependencies: array of subsystem type names or IDs; use dependencyCount or null-terminated.
 * platformFilter: bitmask; 1=Windows, 2=Linux, 4=macOS, 8=Android, 16=iOS; 0=all platforms.
 * configData: optional configuration data pointer (can be nullptr).
 */
struct SubsystemDescriptor {
    void const*       typeInfo;         // Object TypeId or TypeDescriptor
    char const*       name;             // subsystem name (non-null, unique)
    char const*       version;          // optional version string (can be nullptr)
    char const* const* dependencies;    // dependency type names/IDs; null-terminated or use dependencyCount
    std::size_t       dependencyCount;
    int               priority;
    std::uint32_t     platformFilter;   // semantics aligned with Core platform detection
    void const*       configData;       // optional configuration data (can be nullptr)
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DESCRIPTOR_HPP
