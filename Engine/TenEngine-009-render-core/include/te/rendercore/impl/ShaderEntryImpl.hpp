/** @file ShaderEntryImpl.hpp
 *  009-RenderCore: IShaderEntry from raw bytecode and reflection (no resource refs).
 */
#pragma once

#include <te/rendercore/IShaderEntry.hpp>
#include <cstddef>

namespace te {
namespace rendercore {

struct VertexFormatDesc;
struct ShaderReflectionDesc;

/** Wraps bytecode and reflection pointers. Caller must keep data valid until no longer needed. */
class ShaderEntryImpl : public IShaderEntry {
 public:
  ShaderEntryImpl(void const* vertexBytecode, std::size_t vertexSize,
                  void const* fragmentBytecode, std::size_t fragmentSize,
                  VertexFormatDesc const* vertexInput,
                  ShaderReflectionDesc const* vertexReflection,
                  ShaderReflectionDesc const* fragmentReflection);
  void const* GetVertexBytecode() const override;
  std::size_t GetVertexBytecodeSize() const override;
  void const* GetFragmentBytecode() const override;
  std::size_t GetFragmentBytecodeSize() const override;
  VertexFormatDesc const* GetVertexInput() const override;
  ShaderReflectionDesc const* GetVertexReflection() const override;
  ShaderReflectionDesc const* GetFragmentReflection() const override;

 private:
  void const* vertexBytecode_;
  std::size_t vertexBytecodeSize_;
  void const* fragmentBytecode_;
  std::size_t fragmentBytecodeSize_;
  VertexFormatDesc const* vertexInput_;
  ShaderReflectionDesc const* vertexReflection_;
  ShaderReflectionDesc const* fragmentReflection_;
};

}  // namespace rendercore
}  // namespace te
