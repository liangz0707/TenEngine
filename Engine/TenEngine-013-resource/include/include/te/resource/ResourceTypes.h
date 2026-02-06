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

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_TYPES_H
