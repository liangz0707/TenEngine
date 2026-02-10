/**
 * @file ShaderModuleInit.h
 * @brief Shader module initialization (contract: specs/_contracts/010-shader-ABI.md).
 *
 * Registers Shader resource factory with 013 and ShaderAssetDesc with 002-Object.
 */
#ifndef TE_SHADER_SHADER_MODULE_INIT_H
#define TE_SHADER_SHADER_MODULE_INIT_H

namespace te {
namespace resource {
class IResourceManager;
}

namespace shader {

/**
 * Initialize shader module.
 * Registers Shader resource factory with manager and ShaderAssetDesc/CompileOptions with 002-Object.
 * Should be called during engine initialization after ResourceManager is ready.
 *
 * @param manager Resource manager (must not be null)
 */
void InitializeShaderModule(resource::IResourceManager* manager);

/**
 * Load all shaders from a manifest file.
 * Each line is a path to a .shader file. If any load fails, stops and returns false.
 *
 * @param manager Resource manager
 * @param manifestPath Path to manifest file (one .shader path per line)
 * @return true if all shaders loaded successfully, false on first failure or invalid input
 */
bool LoadAllShaders(resource::IResourceManager* manager, char const* manifestPath);

/**
 * Shutdown shader module.
 * Unregisters types and cleans up resources.
 */
void ShutdownShaderModule();

}  // namespace shader
}  // namespace te

#endif  // TE_SHADER_SHADER_MODULE_INIT_H
