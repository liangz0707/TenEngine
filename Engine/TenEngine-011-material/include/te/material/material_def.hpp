// 011-Material IMaterialSystem: Load, Get*, Set*, CreateInstance, ReleaseInstance, GetVariantKey, BindToPSO, SubmitToPipeline
#ifndef TE_MATERIAL_MATERIAL_DEF_HPP
#define TE_MATERIAL_MATERIAL_DEF_HPP

#include "te/material/types.hpp"
#include <cstddef>
#include <cstdint>

namespace te {
namespace shader {
class IShaderCompiler;
class IShaderHandle;
struct VariantKey;
}
namespace rendercore {
struct UniformMember;
class IUniformLayout;
}
namespace rhi {
class ICommandList;
}
namespace material {

class IMaterialSystem {
public:
  virtual ~IMaterialSystem() = default;

  virtual MaterialHandle Load(char const* path) = 0;
  virtual uint32_t GetParameters(MaterialHandle h, te::rendercore::UniformMember* out, uint32_t maxCount) = 0;
  virtual bool GetDefaultValues(MaterialHandle h, void* outData, size_t size) = 0;
  virtual te::shader::IShaderHandle* GetShaderRef(MaterialHandle h) = 0;
  virtual uint32_t GetTextureRefs(MaterialHandle h, ParameterSlot* outSlots, char const** outPaths, uint32_t maxCount) = 0;
  virtual te::rendercore::IUniformLayout* GetUniformLayout(MaterialHandle h) = 0;

  virtual void SetScalar(MaterialInstanceHandle inst, ParameterSlot slot, void const* value, size_t size) = 0;
  virtual void SetTexture(MaterialInstanceHandle inst, ParameterSlot slot, void* textureHandle) = 0;
  virtual void SetBuffer(MaterialInstanceHandle inst, ParameterSlot slot, void* bufferHandle) = 0;
  virtual uint32_t GetSlotMapping(MaterialHandle h, ParameterSlot* outSlots, uint32_t maxCount) = 0;

  virtual MaterialInstanceHandle CreateInstance(MaterialHandle parent) = 0;
  virtual void ReleaseInstance(MaterialInstanceHandle inst) = 0;

  virtual te::shader::VariantKey GetVariantKey(MaterialHandle h) = 0;
  virtual void BindToPSO(MaterialInstanceHandle inst, te::rhi::ICommandList* cmd, uint32_t firstSet) = 0;
  virtual void SubmitToPipeline(MaterialInstanceHandle inst, void* pipelineContext) = 0;
};

}  // namespace material
}  // namespace te

#endif
