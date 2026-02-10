/**
 * @file TextureModuleInit.h
 * @brief Texture module initialization (register with 002-Object and 013-Resource).
 */
#ifndef TE_TEXTURE_TEXTURE_MODULE_INIT_H
#define TE_TEXTURE_TEXTURE_MODULE_INIT_H

namespace te {
namespace resource {
class IResourceManager;
}

namespace texture {

/**
 * Initialize texture module.
 * Registers TextureAssetDesc with 002-Object TypeRegistry and resource factory with 013-Resource.
 *
 * @param manager Resource manager instance
 */
void InitializeTextureModule(resource::IResourceManager* manager);

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_MODULE_INIT_H
