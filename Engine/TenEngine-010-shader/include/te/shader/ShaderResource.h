/**
 * @file ShaderResource.h
 * @brief Shader resource implementation (contract: specs/_contracts/010-shader-ABI.md).
 *
 * ShaderResource implements IShaderResource: Load/Save/Import with .shader desc + same-dir source file.
 */
#ifndef TE_SHADER_SHADER_RESOURCE_H
#define TE_SHADER_SHADER_RESOURCE_H

#include <te/resource/ShaderResource.h>
#include <te/resource/Resource.h>
#include <te/shader/api.hpp>
#include <te/shader/ShaderAssetDesc.h>
#include <te/resource/ResourceId.h>
#include <cstddef>
#include <vector>

namespace te {
namespace shader {

/**
 * Shader resource implementation.
 * One directory per shader: .shader (AssetDesc) + source file (e.g. main.hlsl).
 */
class ShaderResource : public resource::IShaderResource {
 public:
  ShaderResource();
  ~ShaderResource() override;

  resource::ResourceType GetResourceType() const override;
  resource::ResourceId GetResourceId() const override;
  void Release() override;

  bool Load(char const* path, resource::IResourceManager* manager) override;
  bool Save(char const* path, resource::IResourceManager* manager) override;
  bool Import(char const* sourcePath, resource::IResourceManager* manager) override;

  void* GetShaderHandle() const override;

  /** Convenience: cast handle to IShaderHandle*. */
  IShaderHandle* GetShaderHandleTyped() const { return handle_; }

 protected:
  void OnLoadComplete() override;
  void OnPrepareSave() override;
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  resource::ResourceId resourceId_;
  int refCount_;
  IShaderCompiler* compiler_;
  IShaderHandle* handle_;
  ShaderAssetDesc desc_;
  std::vector<char> sourceBlob_;
};

}  // namespace shader
}  // namespace te

#endif  // TE_SHADER_SHADER_RESOURCE_H
