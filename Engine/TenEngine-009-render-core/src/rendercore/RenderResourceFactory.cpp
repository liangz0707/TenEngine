/** @file RenderResourceFactory.cpp */
#include <te/rendercore/RenderResourceFactory.hpp>
#include <te/rendercore/impl/RenderMaterialImpl.hpp>
#include <te/rendercore/impl/RenderMeshImpl.hpp>
#include <te/rendercore/impl/RenderElementImpl.hpp>
#include <te/rendercore/impl/RenderPipelineStateImpl.hpp>
#include <te/rendercore/PipelineStateDesc.hpp>

namespace te {
namespace rendercore {

IRenderPipelineState* CreateRenderPipelineState(PipelineStateDesc const& desc) {
  rhi::GraphicsPipelineStateDesc rhiDesc = {};
  ConvertToRHI(desc, rhiDesc);
  return new RenderPipelineStateImpl(rhiDesc);
}

void DestroyRenderPipelineState(IRenderPipelineState* state) {
  delete state;
}

IRenderMaterial* CreateDeviceResourceMaterial(rhi::IDevice* device,
                                              ShaderBytecodeDesc const& bytecodeDesc,
                                              PipelineStateDesc const& pipelineStateDesc) {
  if (!device) return nullptr;
  RenderMaterialImpl* impl = new RenderMaterialImpl(device);
  impl->SetDataShaderBytecode(bytecodeDesc);
  impl->SetDataPipelineState(pipelineStateDesc);
  return impl;
}

void DestroyDeviceResourceMaterial(IRenderMaterial* material) {
  delete material;
}

IRenderMesh* CreateDeviceResourceMesh(rhi::IDevice* device) {
  if (!device) return nullptr;
  return new RenderMeshImpl(device);
}

void DestroyDeviceResourceMesh(IRenderMesh* mesh) {
  delete mesh;
}

IRenderElement* CreateDeviceResourceElement(IRenderMesh* mesh, IRenderMaterial* material) {
  return new RenderElementImpl(mesh, material);
}

void DestroyDeviceResourceElement(IRenderElement* element) {
  delete element;
}

}  // namespace rendercore
}  // namespace te
