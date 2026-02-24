/**
 * @file RenderMaterial.cpp
 * @brief RenderMaterial implementation.
 */

#include <te/material/RenderMaterial.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/IShaderEntry.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>

#include <cstring>

namespace te::material {

// === RenderMaterial ===

RenderMaterial::RenderMaterial() = default;

RenderMaterial::~RenderMaterial() {
    // Cleanup GPU resources
    if (device_) {
        for (auto* pso : psos_) {
            if (pso) {
                device_->DestroyPSO(pso);
            }
        }
        if (descriptorSet_) {
            device_->DestroyDescriptorSet(descriptorSet_);
        }
    }
    uniformBuffer_.reset();
}

// === IRenderPipelineState ===

rhi::GraphicsPipelineStateDesc const* RenderMaterial::GetRHIStateDesc() const {
    return &rhiPipelineStateDesc_;
}

// === IShadingState ===

rendercore::IRenderPipelineState const* RenderMaterial::GetPipelineState() const {
    return pipelineState_;
}

rendercore::IShaderEntry const* RenderMaterial::GetShaderEntry() const {
    return shaderEntry_;
}

// === IRenderMaterial ===

rendercore::IUniformBuffer* RenderMaterial::GetUniformBuffer() {
    return uniformBuffer_.get();
}

rendercore::IUniformBuffer const* RenderMaterial::GetUniformBuffer() const {
    return uniformBuffer_.get();
}

rhi::IDescriptorSet* RenderMaterial::GetDescriptorSet() {
    return descriptorSet_;
}

rhi::IDescriptorSet const* RenderMaterial::GetDescriptorSet() const {
    return descriptorSet_;
}

rhi::IPSO* RenderMaterial::GetGraphicsPSO(uint32_t subpassIndex) {
    if (subpassIndex < psos_.size()) {
        return psos_[subpassIndex];
    }
    return nullptr;
}

rhi::IPSO const* RenderMaterial::GetGraphicsPSO(uint32_t subpassIndex) const {
    if (subpassIndex < psos_.size()) {
        return psos_[subpassIndex];
    }
    return nullptr;
}

void RenderMaterial::CreateDeviceResource() {
    CreateDeviceResource(nullptr, 1, nullptr);
}

void RenderMaterial::CreateDeviceResource(rhi::IRenderPass* renderPass,
                                          uint32_t subpassCount,
                                          rhi::IDescriptorSetLayout* skinLayout) {
    if (!device_) return;

    // Create PSO
    if (!psoCreated_) {
        if (CreatePSO(renderPass, subpassCount)) {
            psoCreated_ = true;
        }
    }

    // Create uniform buffer
    if (!uniformBuffer_) {
        CreateUniformBuffer();
    }

    // Create descriptor set
    if (!descriptorSetCreated_) {
        if (CreateDescriptorSet(skinLayout)) {
            descriptorSetCreated_ = true;
        }
    }

    deviceReady_ = psoCreated_ && descriptorSetCreated_;
}

void RenderMaterial::UpdateDeviceResource(rhi::IDevice* device, uint32_t frameSlot) {
    if (!device) return;
    device_ = device;

    // Set current frame slot for uniform buffer
    if (uniformBuffer_) {
        uniformBuffer_->SetCurrentFrameSlot(frameSlot);
    }

    // Upload parameters to GPU
    UploadParameters();

    // Update descriptor set with textures
    UpdateDescriptorTextures();
}

void RenderMaterial::SetDataParameter(char const* name, void const* data, size_t size) {
    if (!name || !data || size == 0) return;

    std::string key(name);
    auto& param = cpuParameters_[key];
    param.resize(size);
    std::memcpy(param.data(), data, size);

    // Mark device as needing update
    deviceReady_ = false;
}

void RenderMaterial::SetDataTexture(uint32_t binding, rhi::ITexture* texture) {
    cpuTextures_[binding] = texture;

    // Mark device as needing update
    deviceReady_ = false;
}

void RenderMaterial::SetDataTextureByName(char const* name, rhi::ITexture* texture) {
    if (!name) return;

    auto it = textureBindingMap_.find(name);
    if (it != textureBindingMap_.end()) {
        SetDataTexture(it->second, texture);
    }
}

bool RenderMaterial::IsDeviceReady() const {
    return deviceReady_ && psoCreated_ && descriptorSetCreated_;
}

// === Configuration ===

void RenderMaterial::SetShaderEntry(rendercore::IShaderEntry* entry) {
    shaderEntry_ = entry;
}

void RenderMaterial::SetPipelineStateDesc(PipelineStateDesc const& desc) {
    pipelineStateDesc_ = desc;

    // Convert material::PipelineStateDesc to rhi::GraphicsPipelineStateDesc
    rhiPipelineStateDesc_.blendAttachmentCount = desc.blendAttachmentCount;
    for (uint32_t i = 0; i < desc.blendAttachmentCount && i < rhi::GraphicsPipelineStateDesc::kMaxBlendAttachments; ++i) {
        rhiPipelineStateDesc_.blendAttachments[i].blendEnable = desc.blendAttachments[i].blendEnable;
        rhiPipelineStateDesc_.blendAttachments[i].srcColorBlend = static_cast<rhi::BlendFactor>(desc.blendAttachments[i].srcColorBlend);
        rhiPipelineStateDesc_.blendAttachments[i].dstColorBlend = static_cast<rhi::BlendFactor>(desc.blendAttachments[i].dstColorBlend);
        rhiPipelineStateDesc_.blendAttachments[i].colorBlendOp = static_cast<rhi::BlendOp>(desc.blendAttachments[i].colorBlendOp);
        rhiPipelineStateDesc_.blendAttachments[i].srcAlphaBlend = static_cast<rhi::BlendFactor>(desc.blendAttachments[i].srcAlphaBlend);
        rhiPipelineStateDesc_.blendAttachments[i].dstAlphaBlend = static_cast<rhi::BlendFactor>(desc.blendAttachments[i].dstAlphaBlend);
        rhiPipelineStateDesc_.blendAttachments[i].alphaBlendOp = static_cast<rhi::BlendOp>(desc.blendAttachments[i].alphaBlendOp);
        rhiPipelineStateDesc_.blendAttachments[i].colorWriteMask = desc.blendAttachments[i].colorWriteMask;
    }
    rhiPipelineStateDesc_.depthStencil.depthTestEnable = desc.depthStencil.depthTestEnable;
    rhiPipelineStateDesc_.depthStencil.depthWriteEnable = desc.depthStencil.depthWriteEnable;
    rhiPipelineStateDesc_.depthStencil.depthCompareOp = static_cast<rhi::CompareOp>(desc.depthStencil.depthCompareOp);
    rhiPipelineStateDesc_.rasterization.cullMode = static_cast<rhi::CullMode>(desc.rasterization.cullMode);
    rhiPipelineStateDesc_.rasterization.frontFace = static_cast<rhi::FrontFace>(desc.rasterization.frontFace);
}

void RenderMaterial::SetDevice(rhi::IDevice* device) {
    device_ = device;
}

void RenderMaterial::SetName(char const* name) {
    if (name) name_ = name;
}

// === Private ===

bool RenderMaterial::CreatePSO(rhi::IRenderPass* renderPass, uint32_t subpassCount) {
    if (!device_ || !shaderEntry_) return false;

    // Ensure we have space for all subpasses
    psos_.resize(subpassCount > 0 ? subpassCount : 1);

    // Build GraphicsPSODesc using the pre-populated RHI pipeline state
    rhi::GraphicsPSODesc psoDesc{};
    psoDesc.vertex_shader = shaderEntry_->GetVertexBytecode();
    psoDesc.vertex_shader_size = shaderEntry_->GetVertexBytecodeSize();
    psoDesc.fragment_shader = shaderEntry_->GetFragmentBytecode();
    psoDesc.fragment_shader_size = shaderEntry_->GetFragmentBytecodeSize();
    psoDesc.pipelineState = &rhiPipelineStateDesc_;

    // Create PSO with render pass and subpass info
    rhi::IPSO* pso = device_->CreateGraphicsPSO(psoDesc, nullptr, renderPass, subpassCount > 0 ? 0 : 0);
    if (!pso) return false;

    // Use same PSO for all subpasses
    for (auto& p : psos_) {
        p = pso;
    }

    return true;
}

bool RenderMaterial::CreateUniformBuffer() {
    if (!device_ || !shaderEntry_) return false;

    // Get reflection to determine uniform layout
    auto const* refl = shaderEntry_->GetFragmentReflection();
    if (!refl || !refl->uniformBlock.members) {
        // No uniform block - create a minimal buffer
        rendercore::UniformLayoutDesc layoutDesc{};
        layoutDesc.totalSize = 256;  // Minimum alignment
        auto* layout = rendercore::CreateUniformLayout(layoutDesc);
        if (!layout) return false;

        uniformBuffer_.reset(rendercore::CreateUniformBuffer(layout, device_));
        return uniformBuffer_ != nullptr;
    }

    // Build uniform layout from reflection
    rendercore::UniformLayoutDesc layoutDesc{};
    layoutDesc.members = refl->uniformBlock.members;
    layoutDesc.memberCount = refl->uniformBlock.memberCount;
    layoutDesc.totalSize = refl->uniformBlock.totalSize;

    auto* layout = rendercore::CreateUniformLayout(layoutDesc);
    if (!layout) return false;

    uniformBuffer_.reset(rendercore::CreateUniformBuffer(layout, device_));
    return uniformBuffer_ != nullptr;
}

bool RenderMaterial::CreateDescriptorSet(rhi::IDescriptorSetLayout* skinLayout) {
    if (!device_) return false;
    (void)skinLayout;  // Not used in this simplified implementation

    // Create descriptor set layout if not provided
    if (!descriptorSetLayout_) {
        // Build from shader reflection
        rhi::DescriptorSetLayoutDesc layoutDesc{};
        layoutDesc.bindingCount = 0;

        // Add uniform buffer binding (usually slot 0)
        layoutDesc.bindings[layoutDesc.bindingCount].binding = 0;
        layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<uint32_t>(rhi::DescriptorType::UniformBuffer);
        layoutDesc.bindings[layoutDesc.bindingCount].descriptorCount = 1;
        layoutDesc.bindingCount++;

        // Add texture bindings
        for (auto const& [binding, tex] : cpuTextures_) {
            if (layoutDesc.bindingCount >= rhi::DescriptorSetLayoutDesc::kMaxBindings) break;
            layoutDesc.bindings[layoutDesc.bindingCount].binding = binding;
            layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<uint32_t>(rhi::DescriptorType::CombinedImageSampler);
            layoutDesc.bindings[layoutDesc.bindingCount].descriptorCount = 1;
            layoutDesc.bindingCount++;
        }

        descriptorSetLayout_ = device_->CreateDescriptorSetLayout(layoutDesc);
    }

    if (!descriptorSetLayout_) return false;

    // Create descriptor set
    descriptorSet_ = device_->AllocateDescriptorSet(descriptorSetLayout_);
    return descriptorSet_ != nullptr;
}

void RenderMaterial::UploadParameters() {
    if (!uniformBuffer_ || cpuParameters_.empty()) return;

    // Get layout for offset lookup
    auto* layout = uniformBuffer_.get();
    if (!layout) return;

    // Build contiguous CPU buffer from parameters
    auto const* refl = shaderEntry_ ? shaderEntry_->GetFragmentReflection() : nullptr;
    if (!refl || !refl->uniformBlock.members) return;

    size_t totalSize = refl->uniformBlock.totalSize;
    std::vector<uint8_t> bufferData(totalSize, 0);

    // Copy each parameter to its offset
    for (uint32_t i = 0; i < refl->uniformBlock.memberCount; ++i) {
        auto const& member = refl->uniformBlock.members[i];
        auto it = cpuParameters_.find(member.name);
        if (it != cpuParameters_.end() && it->second.size() <= member.size) {
            std::memcpy(bufferData.data() + member.offset, it->second.data(), it->second.size());
        }
    }

    // Upload to GPU
    uniformBuffer_->Update(bufferData.data(), totalSize);
}

void RenderMaterial::UpdateDescriptorTextures() {
    if (!descriptorSet_ || cpuTextures_.empty()) return;

    // Update texture bindings
    for (auto const& [binding, texture] : cpuTextures_) {
        if (texture) {
            rhi::DescriptorWrite write{};
            write.dstSet = descriptorSet_;
            write.binding = binding;
            write.type = static_cast<uint32_t>(rhi::DescriptorType::CombinedImageSampler);
            write.texture = texture;
            write.sampler = nullptr;  // Use default sampler
            write.buffer = nullptr;
            write.bufferOffset = 0;

            device_->UpdateDescriptorSet(descriptorSet_, &write, 1);
        }
    }

    // Also bind uniform buffer to descriptor set
    if (uniformBuffer_) {
        auto* buffer = uniformBuffer_->GetBuffer();
        if (buffer) {
            rhi::DescriptorWrite write{};
            write.dstSet = descriptorSet_;
            write.binding = 0;
            write.type = static_cast<uint32_t>(rhi::DescriptorType::UniformBuffer);
            write.buffer = buffer;
            write.bufferOffset = 0;
            write.texture = nullptr;
            write.sampler = nullptr;

            device_->UpdateDescriptorSet(descriptorSet_, &write, 1);
        }
    }
}

// === Factory functions ===

RenderMaterial* CreateRenderMaterial(
    rendercore::IShaderEntry* shaderEntry,
    PipelineStateDesc const& pipelineState) {
    auto* mat = new RenderMaterial();
    mat->SetShaderEntry(shaderEntry);
    mat->SetPipelineStateDesc(pipelineState);
    return mat;
}

void DestroyRenderMaterial(RenderMaterial* material) {
    delete material;
}

}  // namespace te::material
