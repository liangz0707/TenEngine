#ifndef TE_SHADER_DETAIL_DXC_BACKEND_HPP
#define TE_SHADER_DETAIL_DXC_BACKEND_HPP

#include <string>

namespace te::shader {
class ShaderHandleImpl;

bool CompileHlslToDxil(ShaderHandleImpl* handle, std::string& outError);
}  // namespace te::shader

#endif
