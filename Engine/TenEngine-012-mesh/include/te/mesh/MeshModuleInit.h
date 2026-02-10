/**
 * @file MeshModuleInit.h
 * @brief Mesh module initialization.
 */
#ifndef TE_MESH_MESH_MODULE_INIT_H
#define TE_MESH_MESH_MODULE_INIT_H

namespace te {
namespace resource {
class IResourceManager;
}

namespace mesh {

/**
 * Initialize mesh module.
 * Registers resource factory with ResourceManager.
 * 
 * @param manager Resource manager instance
 */
void InitializeMeshModule(resource::IResourceManager* manager);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_MODULE_INIT_H
