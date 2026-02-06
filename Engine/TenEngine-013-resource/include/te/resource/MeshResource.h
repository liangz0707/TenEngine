/**
 * @file MeshResource.h
 * @brief IMeshResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_MESH_RESOURCE_H
#define TE_RESOURCE_MESH_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/** Mesh resource view; implemented by 012; 013 returns IResource* then caller may cast. */
class IMeshResource : public IResource {
 public:
  ~IMeshResource() override = default;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_MESH_RESOURCE_H
