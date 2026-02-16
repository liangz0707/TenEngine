#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <algorithm>
#include <vector>

namespace te::pipelinecore {

namespace {

class LogicalCommandBufferImpl : public ILogicalCommandBuffer {
 public:
  std::vector<LogicalDraw> draws;

  size_t GetDrawCount() const override { return draws.size(); }
  void GetDraw(size_t index, LogicalDraw* out) const override {
    if (!out || index >= draws.size()) return;
    *out = draws[index];
  }
};

}  // namespace

te::rendercore::ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items,
                                                         ILogicalPipeline const* /*pipeline*/,
                                                         ILogicalCommandBuffer** out) {
  if (!out) return te::rendercore::ResultCode::InvalidHandle;
  LogicalCommandBufferImpl* impl = new LogicalCommandBufferImpl();
  if (items) {
    impl->draws.reserve(items->Size());
    for (size_t i = 0; i < items->Size(); ++i) {
      RenderItem const* r = items->At(i);
      if (!r || !r->element) continue;
      LogicalDraw d;
      d.element = r->element;
      d.submeshIndex = r->submeshIndex;
      d.instanceCount = 1;
      d.firstInstance = 0;
      d.skinMatrixBuffer = r->skinMatrixBuffer;
      d.skinMatrixOffset = r->skinMatrixOffset;

      te::rendercore::IRenderMesh const* reMesh = r->element->GetMesh();
      if (reMesh) {
        te::rendercore::SubmeshRange range;
        if (reMesh->GetSubmesh(d.submeshIndex, &range)) {
          d.indexCount = range.indexCount;
          d.firstIndex = range.indexOffset;
        }
      }
      impl->draws.push_back(d);
    }
    if (!impl->draws.empty()) {
      std::sort(impl->draws.begin(), impl->draws.end(), [](LogicalDraw const& a, LogicalDraw const& b) {
        if (a.element != b.element) return a.element < b.element;
        if (a.skinMatrixBuffer != b.skinMatrixBuffer) return a.skinMatrixBuffer < b.skinMatrixBuffer;
        return a.submeshIndex < b.submeshIndex;
      });
      std::vector<LogicalDraw> merged;
      merged.reserve(impl->draws.size());
      for (size_t i = 0; i < impl->draws.size(); ) {
        LogicalDraw d = impl->draws[i];
        size_t j = i + 1;
        while (j < impl->draws.size() &&
               impl->draws[j].element == d.element &&
               impl->draws[j].skinMatrixBuffer == d.skinMatrixBuffer &&
               impl->draws[j].skinMatrixOffset == d.skinMatrixOffset &&
               impl->draws[j].submeshIndex == d.submeshIndex) {
          d.instanceCount += impl->draws[j].instanceCount;
          ++j;
        }
        merged.push_back(d);
        i = j;
      }
      impl->draws = std::move(merged);
    }
  }
  *out = impl;
  return te::rendercore::ResultCode::Success;
}

void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb) { delete cb; }

te::rendercore::ResultCode ExecuteLogicalCommandBufferOnDeviceThread(
    te::rhi::ICommandList* cmd,
    ILogicalCommandBuffer const* logicalCb,
    te::rhi::IDevice* device,
    uint32_t frameSlot) {
  if (!cmd || !logicalCb) return te::rendercore::ResultCode::InvalidHandle;
  if (!device) return te::rendercore::ResultCode::InvalidHandle;

  size_t drawCount = logicalCb->GetDrawCount();
  for (size_t i = 0; i < drawCount; ++i) {
    LogicalDraw draw;
    logicalCb->GetDraw(i, &draw);
    if (!draw.element) continue;

    te::rendercore::IRenderMesh* mesh = draw.element->GetMesh();
    te::rendercore::IRenderMaterial* mat = draw.element->GetMaterial();
    if (!mesh || !mat) continue;

    // 更新材质的 DescriptorSet（每帧/每 Draw）
    mat->UpdateDeviceResource(device, frameSlot);

    // 设置 PSO
    te::rhi::IPSO* pso = mat->GetGraphicsPSO(0);
    if (pso) {
      cmd->SetGraphicsPSO(pso);
    }

    // 绑定材质的 DescriptorSet (set 0)
    te::rhi::IDescriptorSet* descSet = mat->GetDescriptorSet();
    if (descSet) {
      cmd->BindDescriptorSet(0u, descSet);
    }

    // 如果有蒙皮矩阵，绑定 set 1
    if (draw.skinMatrixBuffer) {
      te::rhi::IDescriptorSet* skinSet = static_cast<te::rhi::IDescriptorSet*>(draw.skinMatrixBuffer);
      cmd->BindDescriptorSet(1u, skinSet);
    }

    // 设置顶点缓冲
    te::rhi::IBuffer* vb = mesh->GetVertexBuffer();
    if (vb) {
      // 获取顶点步幅（假设为 32 字节，实际应从 mesh 元数据获取）
      uint32_t stride = 32u;
      cmd->SetVertexBuffer(0, vb, 0, stride);
    }

    // 设置索引缓冲
    te::rhi::IBuffer* ib = mesh->GetIndexBuffer();
    if (ib) {
      // 0 = 16bit, 1 = 32bit（应根据 mesh 的 IndexType 确定）
      uint32_t indexFormat = 1u;  // 默认 32-bit
      cmd->SetIndexBuffer(ib, 0, indexFormat);
    }

    // 执行 DrawIndexed
    if (ib && draw.indexCount > 0) {
      cmd->DrawIndexed(draw.indexCount, draw.instanceCount,
                       draw.firstIndex, draw.vertexOffset, draw.firstInstance);
    }
  }

  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode SubmitLogicalCommandBuffer(
    te::rhi::ICommandList* cmd,
    te::rhi::IQueue* queue,
    te::rhi::IFence* signalFence,
    te::rhi::ISemaphore* waitSem,
    te::rhi::ISemaphore* signalSem) {
  if (!cmd || !queue) return te::rendercore::ResultCode::InvalidHandle;

  // 使用 RHI 的 Submit 函数
  te::rhi::Submit(cmd, queue, signalFence, waitSem, signalSem);
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PresentSwapChain(te::rhi::ISwapChain* swapChain) {
  if (!swapChain) return te::rendercore::ResultCode::InvalidHandle;

  bool success = swapChain->Present();
  return success ? te::rendercore::ResultCode::Success
                 : te::rendercore::ResultCode::Unknown;
}

}  // namespace te::pipelinecore
