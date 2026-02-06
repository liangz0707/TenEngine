/**
 * @file ResourceLoader.h
 * @brief IResourceLoader (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_LOADER_H
#define TE_RESOURCE_RESOURCE_LOADER_H

#include <te/resource/Resource.h>
#include <te/resource/ResourceTypes.h>

namespace te {
namespace resource {

class IResourceManager;

/** Per-type loader; creates IResource from opaque payload. Dependencies loaded via manager. */
class IResourceLoader {
 public:
  virtual ~IResourceLoader() = default;
  virtual IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager) = 0;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_LOADER_H
