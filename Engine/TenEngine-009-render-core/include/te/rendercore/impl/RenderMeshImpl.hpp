/** @file RenderMeshImpl.hpp
 *  009-RenderCore: IRenderMesh; SetData* for CPU data, UpdateDeviceResource uploads to GPU.
 */
#pragma once

#include <te/rendercore/IRenderMesh.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <cstddef>
#include <vector>

namespace te {
namespace rhi {
struct IDevice;
struct IBuffer;
}
namespace rendercore {

class RenderMeshImpl : public IRenderMesh {
 public:
  explicit RenderMeshImpl(rhi::IDevice* device);
  ~RenderMeshImpl() override;

  rhi::IBuffer* GetVertexBuffer() override;
  rhi::IBuffer const* GetVertexBuffer() const override;
  rhi::IBuffer* GetIndexBuffer() override;
  rhi::IBuffer const* GetIndexBuffer() const override;
  std::uint32_t GetSubmeshCount() const override;
  bool GetSubmesh(std::uint32_t index, SubmeshRange* out) const override;
  void SetDataVertex(void const* data, std::size_t size) override;
  void SetDataIndex(void const* data, std::size_t size) override;
  void SetDataIndexType(IndexType type) override;
  void SetDataSubmeshCount(std::uint32_t count) override;
  void SetDataSubmesh(std::uint32_t index, SubmeshRange const& range) override;
  void UpdateDeviceResource(rhi::IDevice* device) override;

 private:
  rhi::IDevice* device_;
  rhi::IBuffer* vertexBuffer_ = nullptr;
  rhi::IBuffer* indexBuffer_ = nullptr;
  std::vector<std::uint8_t> vertexData_;
  std::vector<std::uint8_t> indexData_;
  std::size_t vertexBufferSize_ = 0;
  std::size_t indexBufferSize_ = 0;
  IndexType indexType_ = IndexType::Unknown;
  std::vector<SubmeshRange> submeshes_;
  bool dirty_ = true;
};

}  // namespace rendercore
}  // namespace te
