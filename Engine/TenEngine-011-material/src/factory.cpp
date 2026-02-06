// 011-Material CreateMaterialSystem, DestroyMaterialSystem
#include "te/material/factory.hpp"
#include "material_system_impl.hpp"

namespace te {
namespace material {

IMaterialSystem* CreateMaterialSystem(te::shader::IShaderCompiler* compiler) {
  if (!compiler) return nullptr;
  return new MaterialSystemImpl(compiler);
}

void DestroyMaterialSystem(IMaterialSystem* sys) {
  if (sys) delete static_cast<MaterialSystemImpl*>(sys);
}

}  // namespace material
}  // namespace te
