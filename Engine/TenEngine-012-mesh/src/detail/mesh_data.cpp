/**
 * @file mesh_data.cpp
 * @brief MeshData destructor implementation.
 */
#include <te/mesh/detail/mesh_data.hpp>
#include <te/rhi/device.hpp>

namespace te {
namespace mesh {
namespace detail {

MeshData::~MeshData() {
  // Note: GPU resources (deviceVertexBuffer, deviceIndexBuffer) should be
  // destroyed by the resource manager (030-DeviceResourceManager) or
  // by the MeshResource destructor. We don't destroy them here to avoid
  // requiring RHI device access in this destructor.
  // The MeshResource should handle GPU resource cleanup.
}

}  // namespace detail
}  // namespace mesh
}  // namespace te
