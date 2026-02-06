// 009-RenderCore UniformLayout
// ABI: te/rendercore/uniform_layout.hpp, std140 alignment

#include <te/rendercore/uniform_layout.hpp>
#include <cstring>
#include <new>

namespace te {
namespace rendercore {

constexpr size_t kMaxUniformMembers = 32;

namespace {

uint32_t GetMemberTypeSize(UniformMemberType type) {
  switch (type) {
    case UniformMemberType::Float: return 4;
    case UniformMemberType::Float2: return 8;
    case UniformMemberType::Float3: return 12;
    case UniformMemberType::Float4: return 16;
    case UniformMemberType::Mat3: return 48;   // std140: 3 columns * 16
    case UniformMemberType::Mat4: return 64;
    case UniformMemberType::Int: return 4;
    case UniformMemberType::Int2: return 8;
    case UniformMemberType::Int3: return 12;
    case UniformMemberType::Int4: return 16;
    default: return 0;
  }
}

uint32_t GetMemberTypeAlignment(UniformMemberType type) {
  switch (type) {
    case UniformMemberType::Float:
    case UniformMemberType::Int: return 4;
    case UniformMemberType::Float2:
    case UniformMemberType::Int2: return 8;
    case UniformMemberType::Float3:
    case UniformMemberType::Float4:
    case UniformMemberType::Mat3:
    case UniformMemberType::Mat4:
    case UniformMemberType::Int3:
    case UniformMemberType::Int4: return 16;
    default: return 0;
  }
}

}  // namespace

class UniformLayoutImpl : public IUniformLayout {
 public:
  UniformMember members[kMaxUniformMembers];
  uint32_t memberCount = 0;
  uint32_t totalSize_ = 0;

  size_t GetOffset(char const* name) const override {
    if (name == nullptr) return 0;
    for (uint32_t i = 0; i < memberCount; ++i) {
      if (std::strcmp(members[i].name, name) == 0)
        return static_cast<size_t>(members[i].offset);
    }
    return 0;
  }

  size_t GetTotalSize() const override { return static_cast<size_t>(totalSize_); }
};

IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc) {
  if (desc.members == nullptr || desc.memberCount == 0)
    return nullptr;
  if (desc.memberCount > kMaxUniformMembers)
    return nullptr;

  UniformLayoutImpl* impl = new (std::nothrow) UniformLayoutImpl{};
  if (!impl) return nullptr;

  uint32_t offset = 0;
  for (uint32_t i = 0; i < desc.memberCount; ++i) {
    auto const& src = desc.members[i];
    if (src.type == UniformMemberType::Unknown) {
      delete impl;
      return nullptr;
    }

    UniformMember& dst = impl->members[i];
    std::strncpy(dst.name, src.name, sizeof(dst.name) - 1);
    dst.name[sizeof(dst.name) - 1] = '\0';
    dst.type = src.type;

    uint32_t align = GetMemberTypeAlignment(src.type);
    uint32_t size = src.size > 0 ? src.size : GetMemberTypeSize(src.type);
    if (src.offset != 0) {
      dst.offset = src.offset;
      offset = src.offset + size;
    } else {
      offset = (offset + align - 1) & ~(align - 1);
      dst.offset = offset;
      offset += size;
    }
    dst.size = size;
  }

  impl->memberCount = desc.memberCount;
  impl->totalSize_ = desc.totalSize > 0 ? desc.totalSize : offset;

  return impl;
}

void ReleaseUniformLayout(IUniformLayout* layout) {
  if (layout)
    delete static_cast<UniformLayoutImpl*>(layout);
}

}  // namespace rendercore
}  // namespace te
