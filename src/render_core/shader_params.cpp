// 009-RenderCore ShaderParams Implementation
// Contract: specs/_contracts/009-rendercore-public-api.md §1. ShaderParams

#include "shader_params.hpp"
#include <cstring>
#include <new>

namespace TenEngine::RenderCore {

// ============================================================================
// Internal implementation
// ============================================================================

struct UniformLayoutImpl {
    UniformMember members[kMaxUniformMembers];
    uint32_t memberCount = 0;
    uint32_t totalSize = 0;
};

namespace {

/// Get byte size for UniformMemberType.
uint32_t GetMemberTypeSize(UniformMemberType type) {
    switch (type) {
        case UniformMemberType::Float:  return 4;
        case UniformMemberType::Float2: return 8;
        case UniformMemberType::Float3: return 12;
        case UniformMemberType::Float4: return 16;
        case UniformMemberType::Mat3:   return 36; // 3x3 floats
        case UniformMemberType::Mat4:   return 64; // 4x4 floats
        case UniformMemberType::Int:    return 4;
        case UniformMemberType::Int2:   return 8;
        case UniformMemberType::Int3:   return 12;
        case UniformMemberType::Int4:   return 16;
        default: return 0;
    }
}

} // namespace

// ============================================================================
// T009: DefineLayout
// ============================================================================

UniformLayout DefineLayout(UniformLayoutDesc const& desc) {
    UniformLayout result{};

    // Validate input
    if (desc.members == nullptr || desc.memberCount == 0) {
        return result; // Invalid: empty handle
    }
    if (desc.memberCount > kMaxUniformMembers) {
        return result; // Too many members: reject
    }

    // Allocate impl
    UniformLayoutImpl* impl = new (std::nothrow) UniformLayoutImpl{};
    if (!impl) {
        return result; // Allocation failed
    }

    uint32_t currentOffset = 0;

    for (uint32_t i = 0; i < desc.memberCount; ++i) {
        auto const& src = desc.members[i];

        // Validate member
        if (src.type == UniformMemberType::Unknown) {
            delete impl;
            return UniformLayout{}; // Invalid type: reject
        }

        UniformMember& dst = impl->members[i];
        std::strncpy(dst.name, src.name, sizeof(dst.name) - 1);
        dst.name[sizeof(dst.name) - 1] = '\0';
        dst.type = src.type;

        // Calculate offset if not provided
        if (src.offset == 0 && i > 0) {
            dst.offset = currentOffset;
        } else {
            dst.offset = src.offset;
        }

        // Calculate size if not provided
        dst.size = src.size > 0 ? src.size : GetMemberTypeSize(src.type);

        currentOffset = dst.offset + dst.size;
    }

    impl->memberCount = desc.memberCount;
    impl->totalSize = desc.totalSize > 0 ? desc.totalSize : currentOffset;

    result.impl = impl;
    return result;
}

// ============================================================================
// T010: GetOffset
// ============================================================================

size_t GetOffset(UniformLayout layout, char const* memberName) {
    if (!layout.IsValid() || memberName == nullptr) {
        return 0; // Invalid: return 0 per contract
    }

    UniformLayoutImpl* impl = layout.impl;
    for (uint32_t i = 0; i < impl->memberCount; ++i) {
        if (std::strcmp(impl->members[i].name, memberName) == 0) {
            return static_cast<size_t>(impl->members[i].offset);
        }
    }

    return 0; // Member not found: return 0 per contract
}

// ============================================================================
// ReleaseLayout (cleanup)
// ============================================================================

void ReleaseLayout(UniformLayout& layout) {
    if (layout.impl) {
        delete layout.impl;
        layout.impl = nullptr;
    }
}

} // namespace TenEngine::RenderCore
