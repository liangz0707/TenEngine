// 012-mesh: EnsureDeviceResources, GetVertexBufferHandle, GetIndexBufferHandle (stub)
#include "te/mesh/MeshDevice.h"

namespace te {
namespace mesh {

bool EnsureDeviceResources(MeshHandle h, void* device) {
  (void)h;
  (void)device;
  return false;
}

void* GetVertexBufferHandle(MeshHandle h) {
  (void)h;
  return nullptr;
}

void* GetIndexBufferHandle(MeshHandle h) {
  (void)h;
  return nullptr;
}

}  // namespace mesh
}  // namespace te
