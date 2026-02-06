/** 012-Mesh: EnsureDeviceResources, GetVertexBufferHandle, GetIndexBufferHandle. */
#ifndef TE_MESH_MESH_DEVICE_H
#define TE_MESH_MESH_DEVICE_H

#include "te/mesh/Mesh.h"

namespace te {
namespace mesh {

/** device 为 008-RHI IDevice*；对依赖链先 Ensure 再调 008 CreateBuffer。 */
bool EnsureDeviceResources(MeshHandle h, void* device);
/** EnsureDeviceResources 后可用；返回类型与 008 契约一致（此处为 void* 占位）。 */
void* GetVertexBufferHandle(MeshHandle h);
void* GetIndexBufferHandle(MeshHandle h);

}  // namespace mesh
}  // namespace te

#endif
