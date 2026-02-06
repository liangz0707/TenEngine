/** 012-Mesh: CreateMesh, ReleaseMesh. */
#ifndef TE_MESH_MESH_FACTORY_H
#define TE_MESH_MESH_FACTORY_H

#include "te/mesh/Mesh.h"
#include "te/mesh/MeshAssetDesc.h"

namespace te {
namespace mesh {

/** 仅接受内存数据，入参由 013 传入；返回 MeshHandle。 */
MeshHandle CreateMesh(MeshAssetDesc const* desc);
void ReleaseMesh(MeshHandle h);

}  // namespace mesh
}  // namespace te

#endif
