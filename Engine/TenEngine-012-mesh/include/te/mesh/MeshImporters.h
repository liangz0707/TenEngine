/**
 * @file MeshImporters.h
 * @brief Forward declarations for mesh importers.
 */
#ifndef TE_MESH_MESH_IMPORTERS_H
#define TE_MESH_MESH_IMPORTERS_H

namespace te {
namespace mesh {

class MeshAssetDesc;

#ifdef TENENGINE_USE_FAST_OBJ
/**
 * Import mesh from OBJ file using fast_obj.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership of allocated data)
 * @return true on success, false on failure
 */
bool ImportMeshFromFastObj(char const* sourcePath, MeshAssetDesc* outDesc);
#endif

#ifdef TENENGINE_USE_ASSIMP
/**
 * Import mesh from source file using Assimp.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership of allocated data)
 * @return true on success, false on failure
 */
bool ImportMeshFromAssimp(char const* sourcePath, MeshAssetDesc* outDesc);
#endif

#ifdef TENENGINE_USE_CGLTF
/**
 * Import mesh from glTF file using cgltf.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership of allocated data)
 * @return true on success, false on failure
 */
bool ImportMeshFromCgltf(char const* sourcePath, MeshAssetDesc* outDesc);
#endif

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_IMPORTERS_H
