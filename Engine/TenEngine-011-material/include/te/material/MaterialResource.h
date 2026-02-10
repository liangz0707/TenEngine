/**
 * @file MaterialResource.h
 * @brief MaterialResource implements resource::IMaterialResource (Load/Save .material JSON, GetTextureRefs as GUID).
 */
#ifndef TE_MATERIAL_MATERIAL_RESOURCE_H
#define TE_MATERIAL_MATERIAL_RESOURCE_H

#include <te/resource/MaterialResource.h>
#include <te/resource/Resource.h>
#include <te/material/material_json.hpp>
#include <te/material/types.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <map>

namespace te {
namespace resource {
class IResourceManager;
}
namespace shader {
class IShaderHandle;
class IShaderCompiler;
}
namespace rendercore {
struct ShaderReflectionDesc;
}
namespace material {

/** Material resource: .material JSON, shader by GUID, textures by name->GUID, parameters by name. */
class MaterialResource : public resource::IMaterialResource {
 public:
  MaterialResource();
  ~MaterialResource() override;

  resource::ResourceType GetResourceType() const override;
  resource::ResourceId GetResourceId() const override;
  void Release() override;

  bool Load(char const* path, resource::IResourceManager* manager) override;
  bool Save(char const* path, resource::IResourceManager* manager) override;
  bool Import(char const* sourcePath, resource::IResourceManager* manager) override;

  std::uint32_t GetTextureRefs(resource::MaterialTextureSlot* outSlots, char const** outPaths,
                              std::uint32_t maxCount) const override;

  te::shader::IShaderHandle* GetShaderHandle() const { return shaderHandle_; }

 protected:
  void OnLoadComplete() override;
  void OnPrepareSave() override;
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  resource::ResourceId resourceId_;
  int refCount_;
  te::shader::IShaderHandle* shaderHandle_ = nullptr;
  resource::IResource* shaderResource_ = nullptr;  /* IShaderResource* for Release */
  struct TextureEntry {
    resource::MaterialTextureSlot slot;
    std::string guidString;
    resource::IResource* textureResource = nullptr;  /* ITextureResource* for Release */
  };
  std::vector<TextureEntry> textureRefs_;
  std::vector<std::uint8_t> paramBuffer_;
  MaterialJSONData jsonData_;  /* for Save: shader guid, texture names->guid, param names->values */
};

}  // namespace material
}  // namespace te

#endif
