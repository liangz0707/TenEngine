/**
 * @file TextureModule.h
 * @brief Global texture module (CPU-only: no GPU cache).
 */
#ifndef TE_TEXTURE_TEXTURE_MODULE_H
#define TE_TEXTURE_TEXTURE_MODULE_H

#include <te/texture/Texture.h>
#include <te/resource/ResourceId.h>

namespace te {
namespace resource {
class IResourceManager;
}

namespace texture {

/**
 * Global texture module: CPU-only; no GPU texture cache or device.
 */
class TextureModule {
 public:
  static TextureModule& GetInstance();

  void SetResourceManager(resource::IResourceManager* manager) { m_resourceManager = manager; }
  resource::IResourceManager* GetResourceManager() const { return m_resourceManager; }

  TextureModule(TextureModule const&) = delete;
  TextureModule& operator=(TextureModule const&) = delete;

 private:
  TextureModule() = default;
  ~TextureModule() = default;

  resource::IResourceManager* m_resourceManager = nullptr;
};

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_MODULE_H
