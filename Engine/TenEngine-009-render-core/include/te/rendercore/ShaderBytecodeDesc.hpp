/** @file ShaderBytecodeDesc.hpp
 *  009-RenderCore: Shader input - bytecode and reflection; no resource references.
 *  Caller (e.g. 010) fills this and passes to CreateDeviceResourceMaterial.
 */
#pragma once

#include <te/rendercore/resource_desc.hpp>
#include <te/rendercore/shader_reflection.hpp>
#include <cstddef>

namespace te {
namespace rendercore {

/** Shader bytecode + reflection for one material. Caller owns source data until CreateDeviceResourceMaterial returns. */
struct ShaderBytecodeDesc {
  void const* vertexBytecode = nullptr;
  std::size_t vertexBytecodeSize = 0;
  void const* fragmentBytecode = nullptr;
  std::size_t fragmentBytecodeSize = 0;
  VertexFormatDesc const* vertexInput = nullptr;
  ShaderReflectionDesc const* vertexReflection = nullptr;
  ShaderReflectionDesc const* fragmentReflection = nullptr;
};

}  // namespace rendercore
}  // namespace te
