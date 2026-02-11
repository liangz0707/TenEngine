/**
 * @file MaterialResource.h
 * @brief MaterialResource: shader by GUID only, params by name, texture name->GUID; no GPU resources.
 */
#ifndef TE_MATERIAL_MATERIAL_RESOURCE_H
#define TE_MATERIAL_MATERIAL_RESOURCE_H

#include <te/resource/MaterialResource.h>
#include <te/resource/Resource.h>
#include <te/material/material_json.hpp>
#include <te/material/types.hpp>
#include <te/material/MaterialParam.hpp>
#include <te/resource/ResourceId.h>
#include <cstdint>
#include <vector>
#include <string>
#include <map>

namespace te {
namespace resource {
class IResourceManager;
}
namespace material {

/** Material resource: .material JSON or programmatic; shader by GUID only, params by name, textures by name->GUID. No GPU resources. */
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

  /** Shader reference: GUID only. */
  resource::ResourceId GetShaderGuid() const { return shaderGuid_; }
  void SetShaderGuid(resource::ResourceId id) { shaderGuid_ = id; }
  /** Set resource ID (e.g. for programmatic creation before first Save). */
  void SetResourceId(resource::ResourceId id) { resourceId_ = id; }

  /** Pipeline state (blend, etc.); CPU-only data. */
  PipelineStateDesc const& GetPipelineStateDesc() const { return pipelineStateDesc_; }
  void SetPipelineStateDesc(PipelineStateDesc const& desc) { pipelineStateDesc_ = desc; }

  /** Set uniform parameter by name. count 0 or 1 = scalar; N = 1D array. Returns false if type/size invalid. */
  bool SetParameter(char const* name, te::rendercore::UniformMemberType type, void const* data, std::uint32_t count = 1);
  /** Get parameter data; returns false if name not found or outData too small. */
  bool GetParameter(char const* name, void* outData, std::size_t maxSize) const;
  /** Set texture slot by binding name (shader name). GUID only. */
  void SetTextureGuid(char const* name, char const* guid);

  /** Params by name. */
  std::map<std::string, MaterialParam> const& GetParams() const { return params_; }
  /** (bindingName, textureGUID) in order. */
  std::vector<std::pair<std::string, std::string>> const& GetTextureSlots() const { return textureSlots_; }

  /** True when Load succeeded (CPU data ready). */
  bool IsDeviceReady() const override;

 protected:
  void OnPrepareSave() override;
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  resource::ResourceId resourceId_;
  int refCount_;
  resource::ResourceId shaderGuid_;
  std::map<std::string, MaterialParam> params_;
  /** Texture binding name -> texture GUID string. Order preserved for GetTextureRefs (slot = 0, index). */
  std::vector<std::pair<std::string, std::string>> textureSlots_;
  PipelineStateDesc pipelineStateDesc_;
  /** For Save: rebuilt in OnPrepareSave from params_ and textureSlots_. */
  MaterialJSONData jsonData_;
  bool loaded_ = false;
};

/** Create a material resource from shader GUID and pipeline state (no file). Empty params and texture slots. */
MaterialResource* CreateMaterialResourceFromShader(resource::ResourceId shaderGuid, PipelineStateDesc const& pipelineState);

}  // namespace material
}  // namespace te

#endif
