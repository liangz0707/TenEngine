/**
 * @file ResourceTypes.h
 * @brief Resource type enumeration (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_TYPES_H
#define TE_RESOURCE_RESOURCE_TYPES_H

namespace te {
namespace resource {

enum class ResourceType {
  Texture,
  Mesh,
  Material,
  Model,
  Effect,
  Terrain,
  Shader,
  Audio,
  Custom,
  // Reserve for extension
  _Count
};

/** Loading status enumeration. */
enum class LoadStatus {
  Pending,     // Request created, not yet started
  Loading,     // Currently loading
  Completed,   // Load completed successfully
  Failed,      // Load failed
  Cancelled    // Load was cancelled
};

/** Loading result enumeration. */
enum class LoadResult {
  Ok,          // Load succeeded
  NotFound,    // Resource file not found
  Error,        // Load error occurred
  Cancelled     // Load was cancelled
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_TYPES_H
