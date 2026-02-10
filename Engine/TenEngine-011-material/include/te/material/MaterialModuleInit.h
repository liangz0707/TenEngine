/**
 * @file MaterialModuleInit.h
 * @brief Material module initialization: register Material resource factory with 013.
 */
#ifndef TE_MATERIAL_MATERIAL_MODULE_INIT_H
#define TE_MATERIAL_MATERIAL_MODULE_INIT_H

namespace te {
namespace resource {
class IResourceManager;
}

namespace material {

/**
 * Initialize material module.
 * Registers Material resource factory with manager.
 * Call after ResourceManager is ready (e.g. after LoadAllShaders).
 *
 * @param manager Resource manager (must not be null)
 */
void InitializeMaterialModule(resource::IResourceManager* manager);

/**
 * Initialize shader and material resource modules and load all shaders from manifest.
 * Call once after ResourceManager is ready (e.g. in engine/app init).
 * Order: InitializeShaderModule -> LoadAllShaders -> InitializeMaterialModule.
 *
 * @param manager Resource manager (must not be null)
 * @param shaderManifestPath Path to shader manifest file (one .shader path per line); may be null to skip LoadAllShaders
 * @return true if all steps succeeded (if shaderManifestPath is null, LoadAllShaders is skipped and true)
 */
bool InitializeResourceModulesForEngine(resource::IResourceManager* manager, char const* shaderManifestPath);

}  // namespace material
}  // namespace te

#endif  // TE_MATERIAL_MATERIAL_MODULE_INIT_H
