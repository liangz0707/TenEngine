/**
 * @file RenderMaterial.hpp
 * @brief 011-Material: RenderMaterial - IRenderMaterial implementation for GPU rendering.
 */

#pragma once

#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rendercore/uniform_buffer.hpp>
#include <te/rendercore/IRenderPipelineState.hpp>
#include <te/rhi/resources.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace te {
namespace rhi {
struct IPSO;
struct IDescriptorSet;
struct IDescriptorSetLayout;
struct IRenderPass;
struct ITexture;
}

namespace material {

/**
 * @brief RenderMaterial - Concrete implementation of IRenderMaterial.
 *
 * Manages CPU-side parameters/textures and GPU-side resources (PSO, UB, DescriptorSet).
 * Usage:
 * 1. SetDataParameter() / SetDataTexture() to set CPU data
 * 2. CreateDeviceResource() to create GPU resources
 * 3. UpdateDeviceResource() per frame to upload data
 * 4. GetGraphicsPSO() / GetDescriptorSet() during draw
 */
class RenderMaterial : public rendercore::IRenderMaterial {
public:
    RenderMaterial();
    ~RenderMaterial() override;

    // === IShadingState ===
    rendercore::IRenderPipelineState const* GetPipelineState() const override;
    rendercore::IShaderEntry const* GetShaderEntry() const override;

    // === IRenderMaterial ===
    rendercore::IUniformBuffer* GetUniformBuffer() override;
    rendercore::IUniformBuffer const* GetUniformBuffer() const override;
    rhi::IDescriptorSet* GetDescriptorSet() override;
    rhi::IDescriptorSet const* GetDescriptorSet() const override;
    rhi::IPSO* GetGraphicsPSO(uint32_t subpassIndex) override;
    rhi::IPSO const* GetGraphicsPSO(uint32_t subpassIndex) const override;

    void CreateDeviceResource() override;
    void CreateDeviceResource(rhi::IRenderPass* renderPass,
                              uint32_t subpassCount,
                              rhi::IDescriptorSetLayout* skinLayout) override;
    void UpdateDeviceResource(rhi::IDevice* device, uint32_t frameSlot) override;

    void SetDataParameter(char const* name, void const* data, size_t size) override;
    void SetDataTexture(uint32_t binding, rhi::ITexture* texture) override;
    void SetDataTextureByName(char const* name, rhi::ITexture* texture) override;

    bool IsDeviceReady() const override;

    // === Configuration ===
    void SetShaderEntry(rendercore::IShaderEntry* entry);
    void SetPipelineStateDesc(rendercore::PipelineStateDesc const& desc);
    void SetDevice(rhi::IDevice* device);
    void SetName(char const* name);

private:
    // Create PSO if needed
    bool CreatePSO(rhi::IRenderPass* renderPass, uint32_t subpassCount);
    // Create uniform buffer
    bool CreateUniformBuffer();
    // Create descriptor set
    bool CreateDescriptorSet(rhi::IDescriptorSetLayout* skinLayout);
    // Upload parameters to uniform buffer
    void UploadParameters();
    // Update descriptor set textures
    void UpdateDescriptorTextures();

private:
    // Shader and pipeline state
    rendercore::IShaderEntry* shaderEntry_{nullptr};
    rendercore::PipelineStateDesc pipelineStateDesc_;
    rendercore::IRenderPipelineState* pipelineState_{nullptr};

    // CPU data
    std::string name_;
    std::map<std::string, std::vector<uint8_t>> cpuParameters_;
    std::map<uint32_t, rhi::ITexture*> cpuTextures_;
    std::map<std::string, uint32_t> textureBindingMap_;  // name -> binding

    // GPU resources
    rhi::IDevice* device_{nullptr};
    std::unique_ptr<rendercore::IUniformBuffer> uniformBuffer_;
    rhi::IDescriptorSet* descriptorSet_{nullptr};
    std::vector<rhi::IPSO*> psos_;  // Per subpass

    // Layout (created from shader reflection or external)
    rhi::IDescriptorSetLayout* descriptorSetLayout_{nullptr};

    // State
    bool deviceReady_{false};
    bool psoCreated_{false};
    bool descriptorSetCreated_{false};
};

// === Factory functions ===

/**
 * @brief Create a RenderMaterial with shader entry and pipeline state.
 */
RenderMaterial* CreateRenderMaterial(
    rendercore::IShaderEntry* shaderEntry,
    rendercore::PipelineStateDesc const& pipelineState);

/**
 * @brief Destroy a RenderMaterial.
 */
void DestroyRenderMaterial(RenderMaterial* material);

}  // namespace material
}  // namespace te
