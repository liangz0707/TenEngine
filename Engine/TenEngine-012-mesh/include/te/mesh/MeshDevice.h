/**
 * @file MeshDevice.h
 * @brief GPU device resource management for mesh (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_DEVICE_H
#define TE_MESH_MESH_DEVICE_H

#include <te/mesh/Mesh.h>
#include <te/rhi/device.hpp>
#include <cstddef>

namespace te {
namespace mesh {

/**
 * Ensure device resources are created for mesh (synchronous).
 * Creates GPU vertex and index buffers from mesh data.
 * 
 * @param h Mesh handle
 * @param device RHI device
 * @return true on success, false on failure
 */
bool EnsureDeviceResources(MeshHandle h, rhi::IDevice* device);

/**
 * Ensure device resources are created for mesh (asynchronous).
 * Creates GPU vertex and index buffers from mesh data asynchronously.
 * 
 * @param h Mesh handle
 * @param device RHI device
 * @param on_done Completion callback: void(*)(void* user_data)
 * @param user_data User data passed to callback
 */
void EnsureDeviceResourcesAsync(MeshHandle h, rhi::IDevice* device,
                                void (*on_done)(void*), void* user_data);

/**
 * Get vertex buffer handle.
 * Returns GPU buffer handle created by EnsureDeviceResources.
 * 
 * @param h Mesh handle
 * @return Vertex buffer handle, or nullptr if not created
 */
rhi::IBuffer* GetVertexBufferHandle(MeshHandle h);

/**
 * Get index buffer handle.
 * Returns GPU buffer handle created by EnsureDeviceResources.
 * 
 * @param h Mesh handle
 * @return Index buffer handle, or nullptr if not created
 */
rhi::IBuffer* GetIndexBufferHandle(MeshHandle h);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_DEVICE_H
