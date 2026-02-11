/**
 * @file mesh_data.cpp
 * @brief MeshData destructor implementation.
 */
#include <te/mesh/detail/mesh_data.hpp>

namespace te {
namespace mesh {
namespace detail {

MeshData::~MeshData() {
  // Device buffers are not stored here (012 is CPU-only).
}

}  // namespace detail
}  // namespace mesh
}  // namespace te
