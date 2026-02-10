/**
 * @file ShaderResource.h
 * @brief IShaderResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_SHADER_RESOURCE_H
#define TE_RESOURCE_SHADER_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/**
 * Shader resource view; implemented by 010; 013 returns IResource* then caller may cast.
 * GetShaderHandle() returns void* which is actually te::shader::IShaderHandle*.
 */
class IShaderResource : public IResource {
 public:
  ~IShaderResource() override = default;

  /** Returns shader handle; actual type is te::shader::IShaderHandle*. Caller casts as needed. */
  virtual void* GetShaderHandle() const = 0;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_SHADER_RESOURCE_H
