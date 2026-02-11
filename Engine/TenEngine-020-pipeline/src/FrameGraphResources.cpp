/**
 * @file FrameGraphResources.cpp
 * @brief Create transient RTs from FrameGraph and build RenderPassDesc.
 */

#include <te/pipeline/detail/FrameGraphResources.h>
#include <te/pipelinecore/FrameGraph.h>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <algorithm>
#include <map>
#include <vector>

namespace te {
namespace pipeline {
namespace detail {

namespace {

static te::rhi::LoadOp ToRhiLoadOp(pipelinecore::AttachmentLoadOp op) {
  switch (op) {
    case pipelinecore::AttachmentLoadOp::LoadOp_Load: return te::rhi::LoadOp::Load;
    case pipelinecore::AttachmentLoadOp::Clear: return te::rhi::LoadOp::Clear;
    case pipelinecore::AttachmentLoadOp::DontCare: return te::rhi::LoadOp::DontCare;
    default: return te::rhi::LoadOp::Clear;
  }
}

static te::rhi::StoreOp ToRhiStoreOp(pipelinecore::AttachmentStoreOp op) {
  switch (op) {
    case pipelinecore::AttachmentStoreOp::StoreOp_Store: return te::rhi::StoreOp::Store;
    case pipelinecore::AttachmentStoreOp::DontCare: return te::rhi::StoreOp::DontCare;
    default: return te::rhi::StoreOp::Store;
  }
}

}  // namespace

FrameGraphResourceSet BuildFrameGraphResources(
    pipelinecore::IFrameGraph const* graph,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    te::rhi::IDevice* device,
    te::rhi::ITexture* backBuffer,
    FrameGraphPersistentCache* persistentCache) {
  FrameGraphResourceSet out;
  if (!graph || !device || viewportWidth == 0 || viewportHeight == 0) return out;
  size_t passCount = graph->GetPassCount();
  if (passCount == 0) return out;

  std::map<FrameGraphResKey, pipelinecore::AttachmentLifetime> resourceLifetime;
  for (size_t i = 0; i < passCount; ++i) {
    pipelinecore::PassCollectConfig config = {};
    graph->GetPassCollectConfig(i, &config);
    uint32_t w = config.output.width > 0 ? config.output.width : viewportWidth;
    uint32_t h = config.output.height > 0 ? config.output.height : viewportHeight;
    for (uint32_t c = 0; c < config.colorAttachmentCount; ++c) {
      uint64_t id = config.colorAttachments[c].handle.id;
      if (!config.colorAttachments[c].handle.IsValid()) continue;
      uint32_t cw = config.colorAttachments[c].width > 0 ? config.colorAttachments[c].width : w;
      uint32_t ch = config.colorAttachments[c].height > 0 ? config.colorAttachments[c].height : h;
      FrameGraphResKey key{id, cw, ch, config.colorAttachments[c].format, false};
      auto it = resourceLifetime.find(key);
      pipelinecore::AttachmentLifetime lt = config.colorAttachments[c].lifetime;
      if (it == resourceLifetime.end())
        resourceLifetime[key] = lt;
      else if (lt == pipelinecore::AttachmentLifetime::Transient)
        it->second = pipelinecore::AttachmentLifetime::Transient;
    }
    if (config.hasDepthStencil && config.depthStencilAttachment.handle.IsValid()) {
      uint64_t id = config.depthStencilAttachment.handle.id;
      uint32_t dw = config.depthStencilAttachment.width > 0 ? config.depthStencilAttachment.width : w;
      uint32_t dh = config.depthStencilAttachment.height > 0 ? config.depthStencilAttachment.height : h;
      FrameGraphResKey key{id, dw, dh, config.depthStencilAttachment.format, true};
      auto it = resourceLifetime.find(key);
      pipelinecore::AttachmentLifetime lt = config.depthStencilAttachment.lifetime;
      if (it == resourceLifetime.end())
        resourceLifetime[key] = lt;
      else if (lt == pipelinecore::AttachmentLifetime::Transient)
        it->second = pipelinecore::AttachmentLifetime::Transient;
    }
  }

  out.idToTexture[0] = backBuffer;
  for (auto const& kv : resourceLifetime) {
    FrameGraphResKey const& key = kv.first;
    if (key.id == pipelinecore::kResourceHandleIdBackBuffer) continue;
    te::rhi::ITexture* tex = nullptr;
    bool usePersistent = (kv.second == pipelinecore::AttachmentLifetime::Persistent && persistentCache);
    if (usePersistent) {
      auto it = persistentCache->cache.find(key);
      if (it != persistentCache->cache.end())
        tex = it->second;
    }
    if (!tex) {
      te::rhi::TextureDesc td = {};
      td.width = key.width;
      td.height = key.height;
      td.depth = 1;
      td.format = key.isDepthStencil ? (key.format != 0 ? key.format : kDefaultDepthStencilFormat) : (key.format & 0xFFFFu);
      tex = device->CreateTexture(td);
      if (!tex) {
        for (te::rhi::ITexture* t : out.ownedTextures) device->DestroyTexture(t);
        out.ownedTextures.clear();
        out.idToTexture.clear();
        return out;
      }
      if (usePersistent)
        persistentCache->cache[key] = tex;
      else
        out.ownedTextures.push_back(tex);
    }
    out.idToTexture[key.id] = tex;
  }

  std::vector<uint64_t> colorOrder;
  colorOrder.push_back(pipelinecore::kResourceHandleIdBackBuffer);
  uint64_t depthId = 0;
  bool hasDepth = false;
  for (auto const& kv : resourceLifetime) {
    FrameGraphResKey const& key = kv.first;
    if (key.id == pipelinecore::kResourceHandleIdBackBuffer) continue;
    if (key.isDepthStencil) {
      depthId = key.id;
      hasDepth = true;
    } else {
      if (std::find(colorOrder.begin(), colorOrder.end(), key.id) == colorOrder.end())
        colorOrder.push_back(key.id);
    }
  }
  std::sort(colorOrder.begin() + 1, colorOrder.end());

  uint32_t colorCount = static_cast<uint32_t>(colorOrder.size());
  out.rpDesc.colorAttachmentCount = colorCount;
  out.rpDesc.subpassCount = static_cast<uint32_t>(passCount);
  for (uint32_t i = 0; i < colorCount && i < te::rhi::kMaxColorAttachments; ++i) {
    te::rhi::ITexture* tex = out.idToTexture[colorOrder[i]];
    if (tex) {
      out.rpDesc.colorAttachments[i].texture = tex;
      out.rpDesc.colorAttachments[i].format = 0;
      out.rpDesc.colorAttachments[i].loadOp = te::rhi::LoadOp::Clear;
      out.rpDesc.colorAttachments[i].storeOp = te::rhi::StoreOp::Store;
      out.rpDesc.colorAttachments[i].clearColor[0] = 0.f;
      out.rpDesc.colorAttachments[i].clearColor[1] = 0.f;
      out.rpDesc.colorAttachments[i].clearColor[2] = 0.f;
      out.rpDesc.colorAttachments[i].clearColor[3] = 1.f;
    }
  }
  out.rpDesc.colorLoadOp = te::rhi::LoadOp::Clear;
  out.rpDesc.colorStoreOp = te::rhi::StoreOp::Store;
  out.rpDesc.depthLoadOp = te::rhi::LoadOp::Clear;
  out.rpDesc.depthStoreOp = te::rhi::StoreOp::Store;

  if (hasDepth && out.idToTexture.count(depthId)) {
    out.rpDesc.depthStencilAttachment.texture = out.idToTexture[depthId];
    out.rpDesc.depthStencilAttachment.format = 0;
    out.rpDesc.depthStencilAttachment.loadOp = te::rhi::LoadOp::Clear;
    out.rpDesc.depthStencilAttachment.storeOp = te::rhi::StoreOp::Store;
    out.rpDesc.depthStencilAttachment.clearDepth = 1.f;
    out.rpDesc.depthStencilAttachment.clearStencil = 0;
  } else {
    out.rpDesc.depthStencilAttachment.texture = nullptr;
  }

  auto colorIndexOf = [&](uint64_t id) -> uint32_t {
    for (size_t i = 0; i < colorOrder.size(); ++i)
      if (colorOrder[i] == id) return static_cast<uint32_t>(i);
    return 0;
  };

  for (uint32_t s = 0; s < static_cast<uint32_t>(passCount) && s < te::rhi::kMaxSubpasses; ++s) {
    pipelinecore::PassCollectConfig config = {};
    graph->GetPassCollectConfig(s, &config);
    uint32_t subColorCount = config.colorAttachmentCount;
    if (subColorCount > te::rhi::kMaxColorAttachments) subColorCount = te::rhi::kMaxColorAttachments;
    out.rpDesc.subpasses[s].colorAttachmentCount = subColorCount;
    for (uint32_t c = 0; c < subColorCount; ++c) {
      uint64_t id = config.colorAttachments[c].handle.id;
      out.rpDesc.subpasses[s].colorAttachmentIndices[c] = colorIndexOf(id);
    }
    out.rpDesc.subpasses[s].depthStencilAttachmentIndex = te::rhi::kDepthStencilAttachmentIndexNone;
    if (config.hasDepthStencil && config.depthStencilAttachment.handle.IsValid() && hasDepth)
      out.rpDesc.subpasses[s].depthStencilAttachmentIndex = colorCount;
  }

  out.renderPass = device->CreateRenderPass(out.rpDesc);
  return out;
}

void DestroyFrameGraphResources(FrameGraphResourceSet* set, te::rhi::IDevice* device) {
  if (!set || !device) return;
  if (set->renderPass) {
    device->DestroyRenderPass(set->renderPass);
    set->renderPass = nullptr;
  }
  for (te::rhi::ITexture* t : set->ownedTextures) {
    if (t) device->DestroyTexture(t);
  }
  set->ownedTextures.clear();
  set->idToTexture.clear();
}

void DestroyFrameGraphPersistentCache(FrameGraphPersistentCache* cache, te::rhi::IDevice* device) {
  if (!cache || !device) return;
  for (auto& kv : cache->cache) {
    if (kv.second) device->DestroyTexture(kv.second);
  }
  cache->cache.clear();
}

}  // namespace detail
}  // namespace pipeline
}  // namespace te
