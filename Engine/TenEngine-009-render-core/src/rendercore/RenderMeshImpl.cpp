/** @file RenderMeshImpl.cpp */
#include <te/rendercore/impl/RenderMeshImpl.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <cstdint>
#include <cstring>

namespace te {
namespace rendercore {

RenderMeshImpl::RenderMeshImpl(rhi::IDevice* device) : device_(device) {}

RenderMeshImpl::~RenderMeshImpl() {
  if (device_) {
    if (vertexBuffer_) {
      device_->DestroyBuffer(vertexBuffer_);
      vertexBuffer_ = nullptr;
    }
    if (indexBuffer_) {
      device_->DestroyBuffer(indexBuffer_);
      indexBuffer_ = nullptr;
    }
  }
  device_ = nullptr;
}

rhi::IBuffer* RenderMeshImpl::GetVertexBuffer() {
  return vertexBuffer_;
}

rhi::IBuffer const* RenderMeshImpl::GetVertexBuffer() const {
  return vertexBuffer_;
}

rhi::IBuffer* RenderMeshImpl::GetIndexBuffer() {
  return indexBuffer_;
}

rhi::IBuffer const* RenderMeshImpl::GetIndexBuffer() const {
  return indexBuffer_;
}

std::uint32_t RenderMeshImpl::GetSubmeshCount() const {
  return static_cast<std::uint32_t>(submeshes_.size());
}

bool RenderMeshImpl::GetSubmesh(std::uint32_t index, SubmeshRange* out) const {
  if (!out || index >= submeshes_.size()) return false;
  *out = submeshes_[index];
  return true;
}

void RenderMeshImpl::SetDataVertex(void const* data, std::size_t size) {
  vertexData_.clear();
  if (data && size > 0) {
    vertexData_.resize(size);
    std::memcpy(vertexData_.data(), data, size);
  }
  dirty_ = true;
}

void RenderMeshImpl::SetDataIndex(void const* data, std::size_t size) {
  indexData_.clear();
  if (data && size > 0) {
    indexData_.resize(size);
    std::memcpy(indexData_.data(), data, size);
  }
  dirty_ = true;
}

void RenderMeshImpl::SetDataIndexType(IndexType type) {
  indexType_ = type;
}

void RenderMeshImpl::SetDataSubmeshCount(std::uint32_t count) {
  submeshes_.resize(count);
}

void RenderMeshImpl::SetDataSubmesh(std::uint32_t index, SubmeshRange const& range) {
  if (index >= submeshes_.size()) submeshes_.resize(index + 1);
  submeshes_[index] = range;
}

void RenderMeshImpl::UpdateDeviceResource(rhi::IDevice* device) {
  if (!device) return;
  device_ = device;
  if (!dirty_) return;
  if (vertexData_.empty()) {
    dirty_ = false;
    return;
  }

  std::size_t vertexDataSize = vertexData_.size();
  std::size_t indexDataSize = indexData_.size();
  bool hasIndexBuffer = indexDataSize > 0;

  if (vertexDataSize > 0) {
    bool needCreate = !vertexBuffer_ || vertexDataSize != vertexBufferSize_;
    if (needCreate && vertexBuffer_) {
      device_->DestroyBuffer(vertexBuffer_);
      vertexBuffer_ = nullptr;
      vertexBufferSize_ = 0;
    }
    if (!vertexBuffer_) {
      rhi::BufferDesc rhiBufferDesc{};
      rhiBufferDesc.size = vertexDataSize;
      rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex) |
                            static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
      vertexBuffer_ = device_->CreateBuffer(rhiBufferDesc);
      if (vertexBuffer_) vertexBufferSize_ = vertexDataSize;
    }
    if (vertexBuffer_)
      device_->UpdateBuffer(vertexBuffer_, 0, vertexData_.data(), vertexDataSize);
  }

  if (hasIndexBuffer) {
    bool needCreate = !indexBuffer_ || indexDataSize != indexBufferSize_;
    if (needCreate && indexBuffer_) {
      device_->DestroyBuffer(indexBuffer_);
      indexBuffer_ = nullptr;
      indexBufferSize_ = 0;
    }
    if (!indexBuffer_) {
      rhi::BufferDesc rhiBufferDesc{};
      rhiBufferDesc.size = indexDataSize;
      rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index) |
                            static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
      indexBuffer_ = device_->CreateBuffer(rhiBufferDesc);
      if (indexBuffer_) indexBufferSize_ = indexDataSize;
    }
    if (indexBuffer_)
      device_->UpdateBuffer(indexBuffer_, 0, indexData_.data(), indexDataSize);
  }

  dirty_ = false;
}

}  // namespace rendercore
}  // namespace te
