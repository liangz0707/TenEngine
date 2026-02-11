/**
 * @file MeshFactory.h
 * @brief Mesh creation and release (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_FACTORY_H
#define TE_MESH_MESH_FACTORY_H

#include <te/mesh/Mesh.h>
#include <te/mesh/MeshAssetDesc.h>

namespace te {
namespace mesh {

/**
 * Create mesh from asset description (memory only).
 * Allocates memory for vertex/index data and creates internal MeshData structure.
 * 
 * @param desc Mesh asset description (must be valid)
 * @return Mesh handle, or nullptr on failure
 */
MeshHandle CreateMesh(MeshAssetDesc const* desc);

/**
 * Release mesh handle.
 * Frees all memory associated with the mesh, including vertex/index data.
 * Does not release any resources outside this module.
 * 
 * @param h Mesh handle to release
 */
void ReleaseMesh(MeshHandle h);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_FACTORY_H
