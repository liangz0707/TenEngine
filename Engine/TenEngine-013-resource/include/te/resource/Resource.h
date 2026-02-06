/**
 * @file Resource.h
 * @brief IResource interface (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_H
#define TE_RESOURCE_RESOURCE_H

#include <te/resource/ResourceTypes.h>

namespace te {
namespace resource {

class IResourceManager;

/** Abstract resource handle; not constructed directly; returned from RequestLoadAsync / LoadSync. */
class IResource {
 public:
  virtual ~IResource() = default;

  virtual ResourceType GetResourceType() const = 0;
  /** Decrement refcount; when zero, resource may be reclaimed. Call once per acquisition. */
  virtual void Release() = 0;

  /** Forward to implementation; 013 does not call 008-RHI or create DResource. */
  virtual void EnsureDeviceResources() {}
  virtual void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) { (void)on_done; (void)user_data; }
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_H
