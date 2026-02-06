// 011-Material CreateMaterialSystem, DestroyMaterialSystem
#ifndef TE_MATERIAL_FACTORY_HPP
#define TE_MATERIAL_FACTORY_HPP

#include "te/material/material_def.hpp"

namespace te {
namespace shader {
class IShaderCompiler;
}
namespace material {

IMaterialSystem* CreateMaterialSystem(te::shader::IShaderCompiler* compiler);
void DestroyMaterialSystem(IMaterialSystem* sys);

}  // namespace material
}  // namespace te

#endif
