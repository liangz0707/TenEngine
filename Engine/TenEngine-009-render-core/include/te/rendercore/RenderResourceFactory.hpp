/** @file RenderResourceFactory.hpp
 *  009-RenderCore: Factory for render resources (no resource/payload refs).
 */
#pragma once

#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderPipelineState.hpp>
#include <te/rendercore/ShaderBytecodeDesc.hpp>
#include <te/rendercore/PipelineStateDesc.hpp>

namespace te {
namespace rhi {
struct IDevice;
}
namespace rendercore {

/** Create material; caller sets data via SetData* then CreateDeviceResource / UpdateDeviceResource. */
IRenderMaterial* CreateDeviceResourceMaterial(rhi::IDevice* device,
                                              ShaderBytecodeDesc const& bytecodeDesc,
                                              PipelineStateDesc const& pipelineStateDesc);
void DestroyDeviceResourceMaterial(IRenderMaterial* material);

/** Create mesh; caller sets data via SetData* then UpdateDeviceResource(device). */
IRenderMesh* CreateDeviceResourceMesh(rhi::IDevice* device);
void DestroyDeviceResourceMesh(IRenderMesh* mesh);

/** Create element (stores pointers to mesh and material; does not take ownership). */
IRenderElement* CreateDeviceResourceElement(IRenderMesh* mesh, IRenderMaterial* material);
void DestroyDeviceResourceElement(IRenderElement* element);

IRenderPipelineState* CreateRenderPipelineState(PipelineStateDesc const& desc);
void DestroyRenderPipelineState(IRenderPipelineState* state);

}  // namespace rendercore
}  // namespace te
