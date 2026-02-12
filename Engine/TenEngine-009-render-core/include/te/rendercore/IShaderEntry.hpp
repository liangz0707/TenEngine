/** @file IShaderEntry.hpp
 *  009-RenderCore: Shader entry interface (bytecode and reflection for PSO creation).
 */
#pragma once

#include <te/rendercore/resource_desc.hpp>
#include <te/rendercore/shader_reflection.hpp>
#include <cstddef>

namespace te {
namespace rendercore {

/** Shader entry: bytecode and reflection per stage. Caller guarantees bytecode format matches device backend. */
struct IShaderEntry {
  virtual ~IShaderEntry() = default;
  virtual void const* GetVertexBytecode() const = 0;
  virtual std::size_t GetVertexBytecodeSize() const = 0;
  virtual void const* GetFragmentBytecode() const = 0;
  virtual std::size_t GetFragmentBytecodeSize() const = 0;
  virtual VertexFormatDesc const* GetVertexInput() const = 0;
  virtual ShaderReflectionDesc const* GetVertexReflection() const = 0;
  virtual ShaderReflectionDesc const* GetFragmentReflection() const = 0;
};

}  // namespace rendercore
}  // namespace te
