// 011-Material internal: concrete IMaterialSystem implementation (stub until T009–T011)
#ifndef TE_MATERIAL_MATERIAL_SYSTEM_IMPL_HPP
#define TE_MATERIAL_MATERIAL_SYSTEM_IMPL_HPP

#include "te/material/material_def.hpp"
#include "te/material/types.hpp"

namespace te {
namespace shader {
class IShaderCompiler;
struct VariantKey;
}
namespace material {

class MaterialSystemImpl : public IMaterialSystem {
public:
  explicit MaterialSystemImpl(te::shader::IShaderCompiler* compiler);
  ~MaterialSystemImpl() override;

  MaterialHandle Load(char const* path) override;
  uint32_t GetParameters(MaterialHandle h, te::rendercore::UniformMember* out, uint32_t maxCount) override;
  bool GetDefaultValues(MaterialHandle h, void* outData, size_t size) override;
  te::shader::IShaderHandle* GetShaderRef(MaterialHandle h) override;
  uint32_t GetTextureRefs(MaterialHandle h, ParameterSlot* outSlots, char const** outPaths, uint32_t maxCount) override;
  te::rendercore::IUniformLayout* GetUniformLayout(MaterialHandle h) override;

  void SetScalar(MaterialInstanceHandle inst, ParameterSlot slot, void const* value, size_t size) override;
  void SetTexture(MaterialInstanceHandle inst, ParameterSlot slot, void* textureHandle) override;
  void SetBuffer(MaterialInstanceHandle inst, ParameterSlot slot, void* bufferHandle) override;
  uint32_t GetSlotMapping(MaterialHandle h, ParameterSlot* outSlots, uint32_t maxCount) override;

  MaterialInstanceHandle CreateInstance(MaterialHandle parent) override;
  void ReleaseInstance(MaterialInstanceHandle inst) override;

  te::shader::VariantKey GetVariantKey(MaterialHandle h) override;
  void SubmitToPipeline(MaterialInstanceHandle inst, void* pipelineContext) override;

private:
  te::shader::IShaderCompiler* compiler_{nullptr};
};

}  // namespace material
}  // namespace te

#endif
