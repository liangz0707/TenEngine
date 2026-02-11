// 011-Material MaterialSystemImpl: stub implementations until T009–T011 fill Load/Get*
#include "material_system_impl.hpp"
#include <te/shader/types.hpp>

namespace te {
namespace material {

MaterialSystemImpl::MaterialSystemImpl(te::shader::IShaderCompiler* compiler) : compiler_(compiler) {}
MaterialSystemImpl::~MaterialSystemImpl() = default;

MaterialHandle MaterialSystemImpl::Load(char const*) { return {}; }
uint32_t MaterialSystemImpl::GetParameters(MaterialHandle, te::rendercore::UniformMember*, uint32_t) { return 0; }
bool MaterialSystemImpl::GetDefaultValues(MaterialHandle, void*, size_t) { return false; }
te::shader::IShaderHandle* MaterialSystemImpl::GetShaderRef(MaterialHandle) { return nullptr; }
uint32_t MaterialSystemImpl::GetTextureRefs(MaterialHandle, ParameterSlot*, char const**, uint32_t) { return 0; }
te::rendercore::IUniformLayout* MaterialSystemImpl::GetUniformLayout(MaterialHandle) { return nullptr; }

void MaterialSystemImpl::SetScalar(MaterialInstanceHandle, ParameterSlot, void const*, size_t) {}
void MaterialSystemImpl::SetTexture(MaterialInstanceHandle, ParameterSlot, void*) {}
void MaterialSystemImpl::SetBuffer(MaterialInstanceHandle, ParameterSlot, void*) {}
uint32_t MaterialSystemImpl::GetSlotMapping(MaterialHandle, ParameterSlot*, uint32_t) { return 0; }

MaterialInstanceHandle MaterialSystemImpl::CreateInstance(MaterialHandle) { return {}; }
void MaterialSystemImpl::ReleaseInstance(MaterialInstanceHandle) {}

te::shader::VariantKey MaterialSystemImpl::GetVariantKey(MaterialHandle) { return te::shader::VariantKey{}; }
void MaterialSystemImpl::SubmitToPipeline(MaterialInstanceHandle, void*) {}

}  // namespace material
}  // namespace te
