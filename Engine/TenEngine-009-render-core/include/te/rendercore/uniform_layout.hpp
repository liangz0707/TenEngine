/** @file uniform_layout.hpp
 *  009-RenderCore ABI: UniformMemberType, UniformMember, UniformLayoutDesc,
 *  IUniformLayout, CreateUniformLayout, ReleaseUniformLayout.
 */
#pragma once

#include <te/rendercore/types.hpp>
#include <cstddef>
#include <cstdint>

namespace te {
namespace rendercore {

enum class UniformMemberType : uint8_t {
  Float, Float2, Float3, Float4,
  Mat3, Mat4,
  Int, Int2, Int3, Int4,
  Unknown
};

struct UniformMember {
  char name[64] = {};
  UniformMemberType type = UniformMemberType::Unknown;
  uint32_t count = 1;               // Array element count (0 or 1 = scalar)
  uint32_t offset = 0;
  uint32_t size = 0;
};

struct UniformLayoutDesc {
  UniformMember const* members = nullptr;
  uint32_t memberCount = 0;
  uint32_t totalSize = 0;  // 0 = auto-calculate per std140
};

struct IUniformLayout {
  virtual size_t GetOffset(char const* name) const = 0;
  virtual size_t GetTotalSize() const = 0;
  virtual ~IUniformLayout() = default;
};

IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);

void ReleaseUniformLayout(IUniformLayout* layout);

}  // namespace rendercore
}  // namespace te
