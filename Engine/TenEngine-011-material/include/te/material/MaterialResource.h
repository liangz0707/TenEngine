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
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/resources.hpp>
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
struct IUniformLayout;
struct IUniformBuffer;
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
  /** Uniform buffer for material parameters; created in EnsureDeviceResources when shader has uniform block. May be null. */
  te::rendercore::IUniformBuffer* GetUniformBuffer() const { return uniformBuffer_; }

  /** Graphics PSO for this material; used by pipeline to bind before draw. When multi-subpass, use GetGraphicsPSO(subpassIndex). */
  te::rhi::IPSO* GetGraphicsPSO(std::uint32_t subpassIndex = 0) const;

  /** Descriptor set for this material (UB + textures). Updated by UpdateDescriptorSetForFrame. */
  te::rhi::IDescriptorSet* GetDescriptorSet() const { return descriptorSet_; }

  /** Update descriptor set with current frame slot (UB offset) and texture bindings. Call before BindDescriptorSet. */
  void UpdateDescriptorSetForFrame(te::rhi::IDevice* device, uint32_t frameSlot);

  /** Set RHI device for EnsureDeviceResources; call before EnsureDeviceResources. */
  void SetDevice(te::rhi::IDevice* device) { device_ = device; }
  void EnsureDeviceResources() override;
  /** When renderPass and subpassCount are set, creates one PSO per subpass. Otherwise single PSO (subpass 0). */
  void EnsureDeviceResources(te::rhi::IRenderPass* renderPass, uint32_t subpassCount);
  bool IsDeviceReady() const override;

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
  te::rhi::IDevice* device_ = nullptr;
  bool deviceResourcesReady_ = false;
  te::rendercore::IUniformLayout* uniformLayout_ = nullptr;
  te::rendercore::IUniformBuffer* uniformBuffer_ = nullptr;
  te::rhi::IPSO* graphicsPSO_ = nullptr;
  std::vector<te::rhi::IPSO*> graphicsPSOs_;  /* when multi-subpass, one PSO per subpass */
  te::rhi::IDescriptorSetLayout* descriptorSetLayout_ = nullptr;
  te::rhi::IDescriptorSet* descriptorSet_ = nullptr;
  te::rhi::ISampler* defaultSampler_ = nullptr;
};

}  // namespace material
}  // namespace te

#endif
