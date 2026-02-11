/** @file IRenderMesh.hpp
 *  009-RenderCore: GPU mesh = vertex/index buffers; supports submeshes.
 */
#pragma once

#include <te/rendercore/resource_desc.hpp>
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {
struct IDevice;
struct IBuffer;
}
namespace rendercore {

/** Submesh range for draw (aligned with SubmeshDesc). */
struct SubmeshRange {
  std::uint32_t indexOffset = 0;
  std::uint32_t indexCount = 0;
  std::uint32_t vertexOffset = 0;
};

/** GPU mesh. Set CPU data via SetData*, then UpdateDeviceResource(device). */
struct IRenderMesh {
  virtual ~IRenderMesh() = default;
  virtual rhi::IBuffer* GetVertexBuffer() = 0;
  virtual rhi::IBuffer const* GetVertexBuffer() const = 0;
  virtual rhi::IBuffer* GetIndexBuffer() = 0;
  virtual rhi::IBuffer const* GetIndexBuffer() const = 0;
  virtual std::uint32_t GetSubmeshCount() const = 0;
  virtual bool GetSubmesh(std::uint32_t index, SubmeshRange* out) const = 0;
  virtual void SetDataVertex(void const* data, std::size_t size) = 0;
  virtual void SetDataIndex(void const* data, std::size_t size) = 0;
  virtual void SetDataIndexType(IndexType type) = 0;
  virtual void SetDataSubmeshCount(std::uint32_t count) = 0;
  virtual void SetDataSubmesh(std::uint32_t index, SubmeshRange const& range) = 0;
  /** Upload CPU vertex/index data to GPU. */
  virtual void UpdateDeviceResource(rhi::IDevice* device) = 0;
};

}  // namespace rendercore
}  // namespace te
