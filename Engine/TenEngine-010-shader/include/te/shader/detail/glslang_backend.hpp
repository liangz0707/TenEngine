#ifndef TE_SHADER_DETAIL_GLSLANG_BACKEND_HPP
#define TE_SHADER_DETAIL_GLSLANG_BACKEND_HPP

#include <te/shader/types.hpp>
#include <string>

namespace te::shader {
class ShaderHandleImpl;

bool CompileGlslToSpirv(ShaderHandleImpl* handle, CompileOptions const& options, std::string& outError);
bool CompileHlslToSpirv(ShaderHandleImpl* handle, CompileOptions const& options, std::string& outError);
}  // namespace te::shader

#endif
