// 009-RenderCore UniformLayout (te::rendercore)
// Merges layout logic from shader_params and UniformLayoutAdapter

#include <te/rendercore/uniform_layout.hpp>
#include <cstring>
#include <new>

namespace te {
namespace rendercore {

// Internal implementation
struct UniformLayoutImpl {
    UniformMember members[kMaxUniformMembers];
    uint32_t memberCount = 0;
    uint32_t totalSize = 0;
};

namespace {

uint32_t GetMemberTypeSize(UniformMemberType type) {
    switch (type) {
        case UniformMemberType::Float:  return 4;
        case UniformMemberType::Float2: return 8;
        case UniformMemberType::Float3: return 12;
        case UniformMemberType::Float4: return 16;
        case UniformMemberType::Mat3:   return 36;
        case UniformMemberType::Mat4:   return 64;
        case UniformMemberType::Int:    return 4;
        case UniformMemberType::Int2:   return 8;
        case UniformMemberType::Int3:   return 12;
        case UniformMemberType::Int4:   return 16;
        default: return 0;
    }
}

} // namespace

static UniformLayout DefineLayout(UniformLayoutDesc const& desc) {
    UniformLayout result{};

    if (desc.members == nullptr || desc.memberCount == 0) return result;
    if (desc.memberCount > kMaxUniformMembers) return result;

    UniformLayoutImpl* impl = new (std::nothrow) UniformLayoutImpl{};
    if (!impl) return result;

    uint32_t currentOffset = 0;

    for (uint32_t i = 0; i < desc.memberCount; ++i) {
        auto const& src = desc.members[i];

        if (src.type == UniformMemberType::Unknown) {
            delete impl;
            return UniformLayout{};
        }

        UniformMember& dst = impl->members[i];
        std::strncpy(dst.name, src.name, sizeof(dst.name) - 1);
        dst.name[sizeof(dst.name) - 1] = '\0';
        dst.type = src.type;

        if (src.offset == 0 && i > 0) {
            dst.offset = currentOffset;
        } else {
            dst.offset = src.offset;
        }

        dst.size = src.size > 0 ? src.size : GetMemberTypeSize(src.type);
        currentOffset = dst.offset + dst.size;
    }

    impl->memberCount = desc.memberCount;
    impl->totalSize = desc.totalSize > 0 ? desc.totalSize : currentOffset;

    result.impl = impl;
    return result;
}

static size_t LayoutGetOffset(UniformLayout layout, char const* memberName) {
    if (!layout.IsValid() || memberName == nullptr) return 0;

    UniformLayoutImpl* impl = layout.impl;
    for (uint32_t i = 0; i < impl->memberCount; ++i) {
        if (std::strcmp(impl->members[i].name, memberName) == 0) {
            return static_cast<size_t>(impl->members[i].offset);
        }
    }
    return 0;
}

static void ReleaseLayout(UniformLayout& layout) {
    if (layout.impl) {
        delete layout.impl;
        layout.impl = nullptr;
    }
}

static size_t GetLayoutTotalSize(UniformLayout const& layout) {
    if (!layout.IsValid()) return 0;
    return static_cast<size_t>(layout.impl->totalSize);
}

// Adapter implementing IUniformLayout
class UniformLayoutAdapter : public IUniformLayout {
public:
    explicit UniformLayoutAdapter(UniformLayout layout) : layout_(layout) {}
    ~UniformLayoutAdapter() override { ReleaseLayout(layout_); }

    size_t GetOffset(char const* name) const override {
        return LayoutGetOffset(layout_, name);
    }
    size_t GetTotalSize() const override {
        return GetLayoutTotalSize(layout_);
    }

private:
    UniformLayout layout_;
};

IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc) {
    UniformLayout L = DefineLayout(desc);
    if (!L.IsValid()) return nullptr;
    return new UniformLayoutAdapter(L);
}

void ReleaseUniformLayout(IUniformLayout* layout) {
    delete layout;
}

} // namespace rendercore
} // namespace te
