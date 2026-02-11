/** @file ShaderEntryImpl.cpp */
#include <te/rendercore/impl/ShaderEntryImpl.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/rendercore/shader_reflection.hpp>

namespace te {
namespace rendercore {

ShaderEntryImpl::ShaderEntryImpl(void const* vertexBytecode, std::size_t vertexSize,
                                void const* fragmentBytecode, std::size_t fragmentSize,
                                VertexFormatDesc const* vertexInput,
                                ShaderReflectionDesc const* vertexReflection,
                                ShaderReflectionDesc const* fragmentReflection)
    : vertexBytecode_(vertexBytecode),
      vertexBytecodeSize_(vertexSize),
      fragmentBytecode_(fragmentBytecode),
      fragmentBytecodeSize_(fragmentSize),
      vertexInput_(vertexInput),
      vertexReflection_(vertexReflection),
      fragmentReflection_(fragmentReflection) {}

void const* ShaderEntryImpl::GetVertexBytecode() const {
  return vertexBytecode_;
}

std::size_t ShaderEntryImpl::GetVertexBytecodeSize() const {
  return vertexBytecodeSize_;
}

void const* ShaderEntryImpl::GetFragmentBytecode() const {
  return fragmentBytecode_;
}

std::size_t ShaderEntryImpl::GetFragmentBytecodeSize() const {
  return fragmentBytecodeSize_;
}

VertexFormatDesc const* ShaderEntryImpl::GetVertexInput() const {
  return vertexInput_;
}

ShaderReflectionDesc const* ShaderEntryImpl::GetVertexReflection() const {
  return vertexReflection_;
}

ShaderReflectionDesc const* ShaderEntryImpl::GetFragmentReflection() const {
  return fragmentReflection_;
}

}  // namespace rendercore
}  // namespace te
