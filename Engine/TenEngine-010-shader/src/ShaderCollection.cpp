/**
 * @file ShaderCollection.cpp
 * @brief ShaderCollection singleton and reflection deep-copy implementation.
 */
#include <te/shader/ShaderCollection.h>
#include <cstring>

namespace te {
namespace shader {

namespace {

void DeepCopyReflectionTo(rendercore::ShaderReflectionDesc const& src,
                          rendercore::ShaderReflectionDesc& dst,
                          std::vector<rendercore::UniformMember>& members,
                          std::vector<rendercore::ShaderResourceBinding>& bindings) {
  dst.uniformBlock.totalSize = src.uniformBlock.totalSize;
  dst.uniformBlock.memberCount = src.uniformBlock.memberCount;
  dst.uniformBlock.members = nullptr;
  if (src.uniformBlock.memberCount > 0 && src.uniformBlock.members) {
    members.resize(static_cast<size_t>(src.uniformBlock.memberCount));
    std::memcpy(members.data(), src.uniformBlock.members,
                static_cast<size_t>(src.uniformBlock.memberCount) * sizeof(rendercore::UniformMember));
    dst.uniformBlock.members = members.data();
  }
  dst.resourceBindingCount = src.resourceBindingCount;
  dst.resourceBindings = nullptr;
  if (src.resourceBindingCount > 0 && src.resourceBindings) {
    bindings.resize(static_cast<size_t>(src.resourceBindingCount));
    std::memcpy(bindings.data(), src.resourceBindings,
                static_cast<size_t>(src.resourceBindingCount) * sizeof(rendercore::ShaderResourceBinding));
    dst.resourceBindings = bindings.data();
  }
}

void DeepCopyVertexInput(rendercore::VertexFormatDesc const& src, ShaderCollectionEntry& out) {
  out.vertexInput.attributeCount = src.attributeCount;
  out.vertexInput.stride = src.stride;
  out.vertexInput.attributes = nullptr;
  if (src.attributeCount > 0 && src.attributes) {
    out.vertexInputAttributes.resize(static_cast<size_t>(src.attributeCount));
    std::memcpy(out.vertexInputAttributes.data(), src.attributes,
                static_cast<size_t>(src.attributeCount) * sizeof(rendercore::VertexAttribute));
    out.vertexInput.attributes = out.vertexInputAttributes.data();
  }
}

}  // namespace

void CopyVertexInputInto(rendercore::VertexFormatDesc const& src, ShaderCollectionEntry& out) {
  DeepCopyVertexInput(src, out);
}

void CopyVertexReflectionInto(rendercore::ShaderReflectionDesc const& src, ShaderCollectionEntry& out) {
  DeepCopyReflectionTo(src, out.vertexReflection, out.vertexUniformMembers, out.vertexResourceBindings);
}

void CopyFragmentReflectionInto(rendercore::ShaderReflectionDesc const& src, ShaderCollectionEntry& out) {
  DeepCopyReflectionTo(src, out.fragmentReflection, out.fragmentUniformMembers, out.fragmentResourceBindings);
}

ShaderCollection* ShaderCollection::GetInstance() {
  static ShaderCollection s_instance;
  return &s_instance;
}

void ShaderCollection::Clear() {
  entries_.clear();
}

bool ShaderCollection::Add(resource::ResourceId id, ShaderCollectionEntry entry) {
  entry.resourceId = id;
  entries_[id] = std::move(entry);
  ShaderCollectionEntry& e = entries_[id];
  e.vertexInput.attributes = e.vertexInputAttributes.empty() ? nullptr : e.vertexInputAttributes.data();
  e.vertexReflection.uniformBlock.members = e.vertexUniformMembers.empty() ? nullptr : e.vertexUniformMembers.data();
  e.vertexReflection.resourceBindings = e.vertexResourceBindings.empty() ? nullptr : e.vertexResourceBindings.data();
  e.fragmentReflection.uniformBlock.members = e.fragmentUniformMembers.empty() ? nullptr : e.fragmentUniformMembers.data();
  e.fragmentReflection.resourceBindings = e.fragmentResourceBindings.empty() ? nullptr : e.fragmentResourceBindings.data();
  return true;
}

ShaderCollectionEntry const* ShaderCollection::Get(resource::ResourceId id) const {
  auto it = entries_.find(id);
  return (it != entries_.end()) ? &it->second : nullptr;
}

std::size_t ShaderCollection::Size() const {
  return entries_.size();
}

}  // namespace shader
}  // namespace te
