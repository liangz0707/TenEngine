/**
 * @file TextureResource.h
 * @brief TextureResource implementing ITextureResource/IResource (contract: specs/_contracts/028-texture-ABI.md).
 */
#ifndef TE_TEXTURE_TEXTURE_RESOURCE_H
#define TE_TEXTURE_TEXTURE_RESOURCE_H

#include <te/resource/TextureResource.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceId.h>
#include <te/texture/Texture.h>
#include <te/texture/TextureAssetDesc.h>
#include <te/rhi/resources.hpp>
#include <te/rhi/device.hpp>
#include <memory>
#include <string>
#include <cstddef>

namespace te {
namespace resource {
class IResourceManager;
}

namespace texture {

/**
 * TextureResource: implements ITextureResource (inherits IResource).
 * Manages texture data lifecycle: Load, Save, Import, EnsureDeviceResources.
 */
class TextureResource : public resource::ITextureResource {
 public:
  TextureResource();
  ~TextureResource() override;

  resource::ResourceType GetResourceType() const override;
  resource::ResourceId GetResourceId() const override;
  void Release() override;

  bool Load(char const* path, resource::IResourceManager* manager) override;
  bool LoadAsync(char const* path, resource::IResourceManager* manager,
                 resource::LoadCompleteCallback on_done, void* user_data) override;
  bool Save(char const* path, resource::IResourceManager* manager) override;
  bool Import(char const* sourcePath, resource::IResourceManager* manager) override;
  void EnsureDeviceResources() override;
  void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) override;

  TextureHandle GetTextureHandle() const { return m_textureHandle; }
  void const* GetPixelData() const;
  size_t GetPixelDataSize() const;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  uint32_t GetMipCount() const;
  te::rendercore::TextureFormat GetFormat() const;

  void SetDevice(rhi::IDevice* device) { m_device = device; }
  rhi::ITexture* GetDeviceTexture() const;
  void SetTextureHandle(TextureHandle handle) { m_textureHandle = handle; }

 protected:
  void OnLoadComplete() override;
  void OnPrepareSave() override;
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  TextureHandle m_textureHandle = nullptr;
  resource::ResourceId m_resourceId;
  rhi::IDevice* m_device = nullptr;
  uint32_t m_refCount = 1;

  void CleanupGPUResources();
};

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_RESOURCE_H
