/**
 * @file TextureModuleInit.cpp
 * @brief Texture module initialization - register resource factory and TextureAssetDesc type.
 */
#include <te/texture/TextureModuleInit.h>
#include <te/texture/TextureResource.h>
#include <te/texture/TextureAssetDesc.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/texture/import/StbImageImporter.h>
#ifdef TENENGINE_USE_LIBPNG
#include <te/texture/import/LibPngImporter.h>
#endif
#ifdef TENENGINE_USE_JPEG_TURBO
#include <te/texture/import/LibJpegTurboImporter.h>
#endif
#ifdef TENENGINE_USE_LIBWEBP
#include <te/texture/import/LibWebPImporter.h>
#endif
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstddef>

namespace te {
namespace texture {

namespace {

constexpr object::TypeId kTextureAssetDescTypeId = 0x02800001u;

resource::IResource* CreateTextureResource(resource::ResourceType type) {
  if (type == resource::ResourceType::Texture) {
    return new TextureResource();
  }
  return nullptr;
}

void* CreateTextureAssetDesc() {
  void* ptr = te::core::Alloc(sizeof(TextureAssetDesc), alignof(TextureAssetDesc));
  if (ptr) {
    new (ptr) TextureAssetDesc();
  }
  return ptr;
}

object::PropertyDescriptor g_textureAssetDescProperties[] = {
    {"formatVersion", 0, offsetof(TextureAssetDesc, formatVersion), sizeof(uint32_t), nullptr},
    {"debugDescription", 0, offsetof(TextureAssetDesc, debugDescription), sizeof(std::string), nullptr},
    {"format", 0, offsetof(TextureAssetDesc, format), sizeof(rendercore::TextureFormat), nullptr},
    {"width", 0, offsetof(TextureAssetDesc, width), sizeof(uint32_t), nullptr},
    {"height", 0, offsetof(TextureAssetDesc, height), sizeof(uint32_t), nullptr},
    {"depth", 0, offsetof(TextureAssetDesc, depth), sizeof(uint32_t), nullptr},
    {"mipLevels", 0, offsetof(TextureAssetDesc, mipLevels), sizeof(uint32_t), nullptr},
    {"pixelData", 0, offsetof(TextureAssetDesc, pixelData), sizeof(void*), nullptr},
    {"pixelDataSize", 0, offsetof(TextureAssetDesc, pixelDataSize), sizeof(size_t), nullptr},
    {"isHDR", 0, offsetof(TextureAssetDesc, isHDR), sizeof(bool), nullptr},
};

}  // namespace

void InitializeTextureModule(resource::IResourceManager* manager) {
  if (!manager) return;

  manager->RegisterResourceFactory(resource::ResourceType::Texture, CreateTextureResource);

  object::TypeDescriptor textureAssetDescType;
  textureAssetDescType.id = kTextureAssetDescTypeId;
  textureAssetDescType.name = "TextureAssetDesc";
  textureAssetDescType.size = sizeof(TextureAssetDesc);
  textureAssetDescType.properties = g_textureAssetDescProperties;
  textureAssetDescType.propertyCount =
      sizeof(g_textureAssetDescProperties) / sizeof(g_textureAssetDescProperties[0]);
  textureAssetDescType.baseTypeId = object::kInvalidTypeId;
  textureAssetDescType.createInstance = CreateTextureAssetDesc;

  object::TypeRegistry::RegisterType(textureAssetDescType);

#ifdef TENENGINE_USE_LIBPNG
  static import::LibPngImporter s_libPngImporter;
  import::ImageImporterRegistry::GetInstance().RegisterImporter(&s_libPngImporter, 10);
#endif
#ifdef TENENGINE_USE_JPEG_TURBO
  static import::LibJpegTurboImporter s_libJpegTurboImporter;
  import::ImageImporterRegistry::GetInstance().RegisterImporter(&s_libJpegTurboImporter, 10);
#endif
#ifdef TENENGINE_USE_LIBWEBP
  static import::LibWebPImporter s_libWebPImporter;
  import::ImageImporterRegistry::GetInstance().RegisterImporter(&s_libWebPImporter, 10);
#endif

  static import::StbImageImporter s_stbImporter;
  import::ImageImporterRegistry::GetInstance().RegisterImporter(&s_stbImporter, 0);
}

}  // namespace texture
}  // namespace te
