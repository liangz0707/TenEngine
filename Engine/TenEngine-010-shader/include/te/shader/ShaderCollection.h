/**
 * @file ShaderCollection.h
 * @brief Global shader collection: SPIR-V bytecode and reflection per ResourceId (contract: specs/_contracts/010-shader-ABI.md).
 */
#ifndef TE_SHADER_SHADER_COLLECTION_H
#define TE_SHADER_SHADER_COLLECTION_H

#include <te/resource/ResourceId.h>
#include <te/rendercore/IShaderEntry.hpp>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace te {
namespace shader {

/**
 * Single shader entry: SPIR-V bytecode (per stage), vertex input, and per-stage reflection.
 * All reflection pointers refer to owned vectors; valid as long as the entry exists.
 * Implements IShaderEntry interface for use with material system.
 */
struct ShaderCollectionEntry : public rendercore::IShaderEntry {
  resource::ResourceId resourceId{};
  std::vector<std::uint8_t> vertexBytecode;
  std::vector<std::uint8_t> fragmentBytecode;

  /** Vertex stage: input layout from stage_inputs. vertexInput.attributes points at vertexInputAttributes. */
  rendercore::VertexFormatDesc vertexInput{};
  std::vector<rendercore::VertexAttribute> vertexInputAttributes;

  /** Vertex stage: UBO/resource bindings. Pointers refer to vertexUniformMembers / vertexResourceBindings. */
  rendercore::ShaderReflectionDesc vertexReflection{};
  std::vector<rendercore::UniformMember> vertexUniformMembers;
  std::vector<rendercore::ShaderResourceBinding> vertexResourceBindings;

  /** Fragment stage: UBO/texture/sampler bindings. Pointers refer to fragmentUniformMembers / fragmentResourceBindings. */
  rendercore::ShaderReflectionDesc fragmentReflection{};
  std::vector<rendercore::UniformMember> fragmentUniformMembers;
  std::vector<rendercore::ShaderResourceBinding> fragmentResourceBindings;

  // === IShaderEntry implementation ===
  void const* GetVertexBytecode() const override { return vertexBytecode.data(); }
  std::size_t GetVertexBytecodeSize() const override { return vertexBytecode.size(); }
  void const* GetFragmentBytecode() const override { return fragmentBytecode.data(); }
  std::size_t GetFragmentBytecodeSize() const override { return fragmentBytecode.size(); }
  rendercore::VertexFormatDesc const* GetVertexInput() const override {
    return vertexInputAttributes.empty() ? nullptr : &vertexInput;
  }
  rendercore::ShaderReflectionDesc const* GetVertexReflection() const override { return &vertexReflection; }
  rendercore::ShaderReflectionDesc const* GetFragmentReflection() const override { return &fragmentReflection; }
};

/**
 * Global collection of shaders by ResourceId.
 * Populated at startup by LoadAllShaders; SPIR-V as intermediate format.
 */
class ShaderCollection {
 public:
  /** Returns the global singleton instance. */
  static ShaderCollection* GetInstance();

  void Clear();
  bool Add(resource::ResourceId id, ShaderCollectionEntry entry);
  ShaderCollectionEntry const* Get(resource::ResourceId id) const;
  std::size_t Size() const;

 private:
  ShaderCollection() = default;
  std::unordered_map<resource::ResourceId, ShaderCollectionEntry> entries_;
};

/** Convenience: get the global shader collection. */
inline ShaderCollection* GetShaderCollection() {
  return ShaderCollection::GetInstance();
}

/** Deep-copy vertex input (VertexFormatDesc) into entry.vertexInput and entry.vertexInputAttributes. */
void CopyVertexInputInto(rendercore::VertexFormatDesc const& src, ShaderCollectionEntry& out);

/** Deep-copy vertex-stage reflection into entry.vertexReflection and its owned vectors. */
void CopyVertexReflectionInto(rendercore::ShaderReflectionDesc const& src, ShaderCollectionEntry& out);

/** Deep-copy fragment-stage reflection into entry.fragmentReflection and its owned vectors. */
void CopyFragmentReflectionInto(rendercore::ShaderReflectionDesc const& src, ShaderCollectionEntry& out);

}  // namespace shader
}  // namespace te

#endif
