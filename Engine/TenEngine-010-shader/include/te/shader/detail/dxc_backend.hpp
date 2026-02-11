#ifndef TE_SHADER_DETAIL_DXC_BACKEND_HPP
#define TE_SHADER_DETAIL_DXC_BACKEND_HPP

#include <te/shader/types.hpp>
#include <string>

namespace te::shader {
class ShaderHandleImpl;

bool CompileHlslToDxil(ShaderHandleImpl* handle, CompileOptions const& options, std::string& outError);
/** Compile HLSL to DXBC (sm_5) for D3D11. Uses DXC with vs_5_0/ps_5_0 when available. */
bool CompileHlslToDxbc(ShaderHandleImpl* handle, CompileOptions const& options, std::string& outError);
}  // namespace te::shader

#endif
