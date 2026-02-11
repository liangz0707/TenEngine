/** @file RenderMaterialImpl.cpp */
#include <te/rendercore/impl/RenderMaterialImpl.hpp>
#include <te/rendercore/impl/ShadingStateImpl.hpp>
#include <te/rendercore/impl/RenderPipelineStateImpl.hpp>
#include <te/rendercore/impl/ShaderEntryImpl.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/uniform_buffer.hpp>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/rendercore/PipelineStateDesc.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/resources.hpp>
#include <cstring>
#include <algorithm>

namespace te {
namespace rendercore {

RenderMaterialImpl::RenderMaterialImpl(rhi::IDevice* device) : device_(device) {}

RenderMaterialImpl::~RenderMaterialImpl() {
  if (device_) {
    if (graphicsPSO_) {
      device_->DestroyPSO(graphicsPSO_);
      graphicsPSO_ = nullptr;
    }
    for (rhi::IPSO* p : graphicsPSOs_) {
      if (p) device_->DestroyPSO(p);
    }
    graphicsPSOs_.clear();
    if (descriptorSet_) {
      device_->DestroyDescriptorSet(descriptorSet_);
      descriptorSet_ = nullptr;
    }
    if (descriptorSetLayout_) {
      device_->DestroyDescriptorSetLayout(descriptorSetLayout_);
      descriptorSetLayout_ = nullptr;
    }
    if (defaultSampler_) {
      device_->DestroySampler(defaultSampler_);
      defaultSampler_ = nullptr;
    }
  }
  if (uniformBuffer_) {
    ReleaseUniformBuffer(uniformBuffer_);
    uniformBuffer_ = nullptr;
  }
  if (uniformLayout_) {
    ReleaseUniformLayout(uniformLayout_);
    uniformLayout_ = nullptr;
  }
  if (shadingState_) {
    delete shadingState_;
    shadingState_ = nullptr;
  }
  if (shaderEntryImpl_ && !shadingState_) {
    delete shaderEntryImpl_;
    shaderEntryImpl_ = nullptr;
  }
  if (pipelineStateImpl_ && !shadingState_) {
    delete pipelineStateImpl_;
    pipelineStateImpl_ = nullptr;
  }
}

void RenderMaterialImpl::SetDataShaderBytecode(ShaderBytecodeDesc const& desc) {
  vertexBytecode_.clear();
  fragmentBytecode_.clear();
  vertexInputAttributes_.clear();
  fragmentUniformMembers_.clear();
  fragmentResourceBindings_.clear();
  vertexInput_ = {};
  fragmentReflection_ = {};
  if (shaderEntryImpl_) {
    delete shaderEntryImpl_;
    shaderEntryImpl_ = nullptr;
  }
  if (shadingState_) {
    delete shadingState_;
    shadingState_ = nullptr;
  }

  if (desc.vertexBytecode && desc.vertexBytecodeSize > 0) {
    vertexBytecode_.assign(static_cast<uint8_t const*>(desc.vertexBytecode),
                           static_cast<uint8_t const*>(desc.vertexBytecode) + desc.vertexBytecodeSize);
  }
  if (desc.fragmentBytecode && desc.fragmentBytecodeSize > 0) {
    fragmentBytecode_.assign(static_cast<uint8_t const*>(desc.fragmentBytecode),
                              static_cast<uint8_t const*>(desc.fragmentBytecode) + desc.fragmentBytecodeSize);
  }
  if (desc.vertexInput && desc.vertexInput->attributeCount > 0) {
    vertexInputAttributes_.resize(desc.vertexInput->attributeCount);
    std::memcpy(vertexInputAttributes_.data(), desc.vertexInput->attributes,
                desc.vertexInput->attributeCount * sizeof(VertexAttribute));
    vertexInput_.attributes = vertexInputAttributes_.data();
    vertexInput_.attributeCount = desc.vertexInput->attributeCount;
    vertexInput_.stride = desc.vertexInput->stride;
  }
  if (desc.fragmentReflection) {
    ShaderReflectionDesc const& refl = *desc.fragmentReflection;
    if (refl.uniformBlock.members && refl.uniformBlock.memberCount > 0) {
      fragmentUniformMembers_.resize(refl.uniformBlock.memberCount);
      std::memcpy(fragmentUniformMembers_.data(), refl.uniformBlock.members,
                  refl.uniformBlock.memberCount * sizeof(UniformMember));
      fragmentReflection_.uniformBlock.members = fragmentUniformMembers_.data();
      fragmentReflection_.uniformBlock.memberCount = refl.uniformBlock.memberCount;
      fragmentReflection_.uniformBlock.totalSize = refl.uniformBlock.totalSize;
    }
    if (refl.resourceBindings && refl.resourceBindingCount > 0) {
      fragmentResourceBindings_.resize(refl.resourceBindingCount);
      std::memcpy(fragmentResourceBindings_.data(), refl.resourceBindings,
                  refl.resourceBindingCount * sizeof(ShaderResourceBinding));
      fragmentReflection_.resourceBindings = fragmentResourceBindings_.data();
      fragmentReflection_.resourceBindingCount = refl.resourceBindingCount;
    }
  }

  void const* vsPtr = vertexBytecode_.empty() ? nullptr : vertexBytecode_.data();
  std::size_t vsSize = vertexBytecode_.size();
  void const* fsPtr = fragmentBytecode_.empty() ? nullptr : fragmentBytecode_.data();
  std::size_t fsSize = fragmentBytecode_.size();
  VertexFormatDesc const* vinput = (vertexInput_.attributes && vertexInput_.attributeCount > 0) ? &vertexInput_ : nullptr;
  ShaderReflectionDesc const* fragRefl = (fragmentReflection_.uniformBlock.members || fragmentReflection_.resourceBindings) ? &fragmentReflection_ : nullptr;

  shaderEntryImpl_ = new ShaderEntryImpl(vsPtr, vsSize, fsPtr, fsSize, vinput, nullptr, fragRefl);
  BuildBindingNameToIndex();
  if (pipelineStateImpl_) {
    shadingState_ = new ShadingStateImpl(pipelineStateImpl_, shaderEntryImpl_);
    pipelineStateImpl_ = nullptr;
    shaderEntryImpl_ = nullptr;
  }
}

void RenderMaterialImpl::SetDataPipelineState(PipelineStateDesc const& desc) {
  pipelineStateDesc_ = desc;
  if (pipelineStateImpl_) {
    delete pipelineStateImpl_;
    pipelineStateImpl_ = nullptr;
  }
  rhi::GraphicsPipelineStateDesc rhiDesc = {};
  ConvertToRHI(desc, rhiDesc);
  pipelineStateImpl_ = new RenderPipelineStateImpl(rhiDesc);
  if (shaderEntryImpl_) {
    shadingState_ = new ShadingStateImpl(pipelineStateImpl_, shaderEntryImpl_);
    pipelineStateImpl_ = nullptr;
    shaderEntryImpl_ = nullptr;
  }
}

void RenderMaterialImpl::BuildBindingNameToIndex() {
  bindingNameToIndex_.clear();
  if (fragmentReflection_.resourceBindings) {
    for (uint32_t i = 0; i < fragmentReflection_.resourceBindingCount; ++i) {
      if (fragmentResourceBindings_[i].kind == ShaderResourceKind::SampledImage ||
          fragmentResourceBindings_[i].kind == ShaderResourceKind::Sampler)
        bindingNameToIndex_[fragmentResourceBindings_[i].name] = fragmentResourceBindings_[i].binding;
    }
  }
}

IUniformBuffer* RenderMaterialImpl::GetUniformBuffer() {
  return uniformBuffer_;
}

IUniformBuffer const* RenderMaterialImpl::GetUniformBuffer() const {
  return uniformBuffer_;
}

rhi::IDescriptorSet* RenderMaterialImpl::GetDescriptorSet() {
  return descriptorSet_;
}

rhi::IDescriptorSet const* RenderMaterialImpl::GetDescriptorSet() const {
  return descriptorSet_;
}

rhi::IPSO* RenderMaterialImpl::GetGraphicsPSO(std::uint32_t subpassIndex) {
  if (!graphicsPSOs_.empty())
    return (subpassIndex < graphicsPSOs_.size()) ? graphicsPSOs_[subpassIndex] : nullptr;
  return (subpassIndex == 0) ? graphicsPSO_ : nullptr;
}

rhi::IPSO const* RenderMaterialImpl::GetGraphicsPSO(std::uint32_t subpassIndex) const {
  if (!graphicsPSOs_.empty())
    return (subpassIndex < graphicsPSOs_.size()) ? graphicsPSOs_[subpassIndex] : nullptr;
  return (subpassIndex == 0) ? graphicsPSO_ : nullptr;
}

rhi::GraphicsPipelineStateDesc const* RenderMaterialImpl::GetRHIStateDesc() const {
  return shadingState_ ? shadingState_->GetRHIStateDesc() : nullptr;
}

IRenderPipelineState const* RenderMaterialImpl::GetPipelineState() const {
  return shadingState_ ? shadingState_->GetPipelineState() : nullptr;
}

IShaderEntry const* RenderMaterialImpl::GetShaderEntry() const {
  return shadingState_ ? shadingState_->GetShaderEntry() : shaderEntryImpl_;
}

void RenderMaterialImpl::CreateDeviceResource() {
  CreateDeviceResource(nullptr, 0, nullptr);
}

void RenderMaterialImpl::CreateDeviceResource(rhi::IRenderPass* renderPass, std::uint32_t subpassCount,
                                              rhi::IDescriptorSetLayout* skinLayout) {
  if (!device_) return;
  IShaderEntry const* shaderEntry = GetShaderEntry();
  if (!shaderEntry) return;
  if (!shadingState_ && shaderEntryImpl_ && pipelineStateImpl_) {
    shadingState_ = new ShadingStateImpl(pipelineStateImpl_, shaderEntryImpl_);
    pipelineStateImpl_ = nullptr;
    shaderEntryImpl_ = nullptr;
  }
  shaderEntry = GetShaderEntry();
  if (!shaderEntry) return;

  ShaderReflectionDesc const* refl = shaderEntry->GetFragmentReflection();
  bool hasRefl = refl && refl->uniformBlock.members && refl->uniformBlock.memberCount > 0 && refl->uniformBlock.totalSize > 0;

  if (!uniformBuffer_ && hasRefl && refl->uniformBlock.totalSize > 0) {
    uniformLayout_ = CreateUniformLayout(refl->uniformBlock);
    if (uniformLayout_)
      uniformBuffer_ = CreateUniformBuffer(uniformLayout_, device_);
    if (uniformBuffer_)
      paramBuffer_.resize(refl->uniformBlock.totalSize, 0);
  }

  if (!descriptorSetLayout_ && !descriptorSet_ && device_) {
    rhi::DescriptorSetLayoutDesc layoutDesc = {};
    layoutDesc.bindingCount = 0;
    if (uniformBuffer_) {
      layoutDesc.bindings[layoutDesc.bindingCount].binding = 0;
      layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<uint32_t>(rhi::DescriptorType::UniformBuffer);
      layoutDesc.bindings[layoutDesc.bindingCount].descriptorCount = 1;
      layoutDesc.bindingCount++;
    }
    if (refl && refl->resourceBindings) {
      for (uint32_t i = 0; i < refl->resourceBindingCount && layoutDesc.bindingCount < rhi::DescriptorSetLayoutDesc::kMaxBindings; ++i) {
        if (refl->resourceBindings[i].kind == ShaderResourceKind::SampledImage ||
            refl->resourceBindings[i].kind == ShaderResourceKind::Sampler) {
          layoutDesc.bindings[layoutDesc.bindingCount].binding = refl->resourceBindings[i].binding;
          layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<uint32_t>(rhi::DescriptorType::CombinedImageSampler);
          layoutDesc.bindings[layoutDesc.bindingCount].descriptorCount = 1;
          layoutDesc.bindingCount++;
        }
      }
    }
    if (layoutDesc.bindingCount > 0) {
      descriptorSetLayout_ = device_->CreateDescriptorSetLayout(layoutDesc);
      if (descriptorSetLayout_)
        descriptorSet_ = device_->AllocateDescriptorSet(descriptorSetLayout_);
    }
  }

  if (!defaultSampler_ && device_) {
    rhi::SamplerDesc sd = {};
    sd.filter = 1;
    defaultSampler_ = device_->CreateSampler(sd);
  }

  void const* vsPtr = shaderEntry->GetVertexBytecode();
  size_t vsSize = shaderEntry->GetVertexBytecodeSize();
  void const* fsPtr = shaderEntry->GetFragmentBytecode();
  size_t fsSize = shaderEntry->GetFragmentBytecodeSize();
  if (!vsPtr || vsSize == 0 || !fsPtr || fsSize == 0) return;

  bool wantMulti = (renderPass != nullptr && subpassCount > 0u);
  bool needPSO = descriptorSetLayout_ && (wantMulti ? (graphicsPSOs_.size() != subpassCount) : (graphicsPSO_ == nullptr));
  if (needPSO) {
    rhi::GraphicsPSODesc psoDesc = {};
    psoDesc.vertex_shader = vsPtr;
    psoDesc.vertex_shader_size = vsSize;
    psoDesc.fragment_shader = fsPtr;
    psoDesc.fragment_shader_size = fsSize;
    rhi::GraphicsPipelineStateDesc rhiState = {};
    ConvertToRHI(pipelineStateDesc_, rhiState);
    psoDesc.pipelineState = &rhiState;
    if (wantMulti) {
      for (rhi::IPSO* p : graphicsPSOs_) { if (p) device_->DestroyPSO(p); }
      graphicsPSOs_.clear();
      if (graphicsPSO_) { device_->DestroyPSO(graphicsPSO_); graphicsPSO_ = nullptr; }
      for (uint32_t i = 0; i < subpassCount; ++i) {
        rhi::IPSO* p = device_->CreateGraphicsPSO(psoDesc, descriptorSetLayout_, renderPass, i, skinLayout);
        if (p) graphicsPSOs_.push_back(p);
      }
    } else {
      for (rhi::IPSO* p : graphicsPSOs_) { if (p) device_->DestroyPSO(p); }
      graphicsPSOs_.clear();
      graphicsPSO_ = device_->CreateGraphicsPSO(psoDesc, descriptorSetLayout_, nullptr, 0u, skinLayout);
    }
  }
  deviceResourcesReady_ = true;
}

void RenderMaterialImpl::SetDataParameter(char const* name, void const* data, std::size_t size) {
  if (!name || !data || paramBuffer_.empty()) return;
  if (!fragmentReflection_.uniformBlock.members) return;
  for (uint32_t i = 0; i < fragmentReflection_.uniformBlock.memberCount; ++i) {
    UniformMember const& m = fragmentUniformMembers_[i];
    if (std::strcmp(m.name, name) != 0) continue;
    std::size_t writeSize = (std::min)(size, static_cast<std::size_t>(m.size));
    if (m.offset + writeSize > paramBuffer_.size()) return;
    std::memcpy(paramBuffer_.data() + m.offset, data, writeSize);
    return;
  }
}

void RenderMaterialImpl::SetDataTexture(std::uint32_t binding, rhi::ITexture* texture) {
  if (texture)
    textureBindings_[binding] = texture;
  else
    textureBindings_.erase(binding);
}

void RenderMaterialImpl::SetDataTextureByName(char const* name, rhi::ITexture* texture) {
  if (!name) return;
  auto it = bindingNameToIndex_.find(name);
  if (it != bindingNameToIndex_.end())
    SetDataTexture(it->second, texture);
}

void RenderMaterialImpl::UpdateDeviceResource(rhi::IDevice* device, std::uint32_t frameSlot) {
  if (!device || !descriptorSet_) return;

  if (uniformBuffer_) {
    uniformBuffer_->SetCurrentFrameSlot(static_cast<FrameSlotId>(frameSlot));
    size_t totalSize = uniformLayout_ ? uniformLayout_->GetTotalSize() : paramBuffer_.size();
    if (totalSize > 0 && !paramBuffer_.empty())
      uniformBuffer_->Update(paramBuffer_.data(), totalSize);
  }

  std::vector<rhi::DescriptorWrite> writes;
  if (uniformBuffer_ && uniformBuffer_->GetBuffer()) {
    rhi::DescriptorWrite w = {};
    w.dstSet = descriptorSet_;
    w.binding = 0;
    w.type = static_cast<uint32_t>(rhi::DescriptorType::UniformBuffer);
    w.buffer = uniformBuffer_->GetBuffer();
    w.bufferOffset = uniformBuffer_->GetRingBufferOffset(static_cast<FrameSlotId>(frameSlot));
    w.texture = nullptr;
    w.sampler = nullptr;
    writes.push_back(w);
  }

  for (auto const& kv : textureBindings_) {
    if (!kv.second) continue;
    rhi::DescriptorWrite w = {};
    w.dstSet = descriptorSet_;
    w.binding = kv.first;
    w.type = static_cast<uint32_t>(rhi::DescriptorType::CombinedImageSampler);
    w.buffer = nullptr;
    w.bufferOffset = 0;
    w.texture = kv.second;
    w.sampler = defaultSampler_;
    writes.push_back(w);
  }
  if (!writes.empty())
    device->UpdateDescriptorSet(descriptorSet_, writes.data(), static_cast<uint32_t>(writes.size()));
}

bool RenderMaterialImpl::IsDeviceReady() const {
  return deviceResourcesReady_ && descriptorSet_ != nullptr;
}

}  // namespace rendercore
}  // namespace te
