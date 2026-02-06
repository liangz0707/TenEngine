#ifndef TE_SHADER_DETAIL_SPIRV_CROSS_BACKEND_HPP
#define TE_SHADER_DETAIL_SPIRV_CROSS_BACKEND_HPP

#include <string>

namespace te::shader {
class ShaderHandleImpl;

enum class SpirvCrossTarget { MSL, HLSL };

bool CompileSpirvToTarget(ShaderHandleImpl* handle, SpirvCrossTarget target, std::string& outError);
}  // namespace te::shader

#endif
