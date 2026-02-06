#ifndef TE_SHADER_DETAIL_GLSLANG_BACKEND_HPP
#define TE_SHADER_DETAIL_GLSLANG_BACKEND_HPP

#include <string>

namespace te::shader {
class ShaderHandleImpl;

bool CompileGlslToSpirv(ShaderHandleImpl* handle, std::string& outError);
}  // namespace te::shader

#endif
