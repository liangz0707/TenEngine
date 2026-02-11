/** @file RenderMaterialImpl.hpp
 *  009-RenderCore: IRenderMaterial; SetData* for CPU data, CreateDeviceResource / UpdateDeviceResource.
 */
#pragma once

#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>
#include <te/rendercore/ShaderBytecodeDesc.hpp>
#include <te/rendercore/PipelineStateDesc.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <map>
#include <string>
#include <vector>

namespace te {
namespace rendercore {

struct IShadingState;
struct IUniformBuffer;
struct IUniformLayout;

class RenderMaterialImpl : public IRenderMaterial {
 public:
  void SetDataShaderBytecode(ShaderBytecodeDesc const& desc);
  void SetDataPipelineState(PipelineStateDesc const& desc);

  RenderMaterialImpl(rhi::IDevice* device);
  ~RenderMaterialImpl() override;

  IUniformBuffer* GetUniformBuffer() override;
  IUniformBuffer const* GetUniformBuffer() const override;
  rhi::IDescriptorSet* GetDescriptorSet() override;
  rhi::IDescriptorSet const* GetDescriptorSet() const override;
  rhi::IPSO* GetGraphicsPSO(std::uint32_t subpassIndex) override;
  rhi::IPSO const* GetGraphicsPSO(std::uint32_t subpassIndex) const override;
  void CreateDeviceResource() override;
  void CreateDeviceResource(rhi::IRenderPass* renderPass, std::uint32_t subpassCount,
                            rhi::IDescriptorSetLayout* skinLayout) override;
  void UpdateDeviceResource(rhi::IDevice* device, std::uint32_t frameSlot) override;
  void SetDataParameter(char const* name, void const* data, std::size_t size) override;
  void SetDataTexture(std::uint32_t binding, rhi::ITexture* texture) override;
  void SetDataTextureByName(char const* name, rhi::ITexture* texture) override;
  bool IsDeviceReady() const override;

  rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const override;
  IRenderPipelineState const* GetPipelineState() const override;
  IShaderEntry const* GetShaderEntry() const override;

 private:
  void BuildBindingNameToIndex();

  rhi::IDevice* device_;
  IShadingState* shadingState_ = nullptr;
  std::vector<std::uint8_t> vertexBytecode_;
  std::vector<std::uint8_t> fragmentBytecode_;
  std::vector<VertexAttribute> vertexInputAttributes_;
  VertexFormatDesc vertexInput_{};
  std::vector<UniformMember> fragmentUniformMembers_;
  std::vector<ShaderResourceBinding> fragmentResourceBindings_;
  ShaderReflectionDesc fragmentReflection_{};
  ShaderEntryImpl* shaderEntryImpl_ = nullptr;
  PipelineStateDesc pipelineStateDesc_;
  IRenderPipelineState* pipelineStateImpl_ = nullptr;
  std::vector<std::uint8_t> paramBuffer_;
  IUniformLayout* uniformLayout_ = nullptr;
  IUniformBuffer* uniformBuffer_ = nullptr;
  rhi::IDescriptorSetLayout* descriptorSetLayout_ = nullptr;
  rhi::IDescriptorSet* descriptorSet_ = nullptr;
  rhi::IPSO* graphicsPSO_ = nullptr;
  std::vector<rhi::IPSO*> graphicsPSOs_;
  rhi::ISampler* defaultSampler_ = nullptr;
  std::map<std::uint32_t, rhi::ITexture*> textureBindings_;
  std::map<std::string, std::uint32_t> bindingNameToIndex_;
  bool deviceResourcesReady_ = false;
};

}  // namespace rendercore
}  // namespace te
