/**
 * @file BuiltinMaterials.cpp
 * @brief Implementation of BuiltinMaterials with actual material creation.
 */

#include <te/pipeline/BuiltinMaterials.h>
#include <te/material/RenderMaterial.hpp>

#include <te/rhi/device.hpp>
#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rendercore/IRenderPipelineState.hpp>

#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace te::pipeline {

// === Shader paths ===
static char const* kDefaultShaderPath = "builtin/shaders/";

// === Embedded shader bytecode (stubs - would be populated by shader compilation) ===
// For a real implementation, these would be generated from .hlsl/.frag files

struct BuiltinShaderData {
    char const* name;
    // Placeholder - in real implementation, these would contain compiled bytecode
    void const* vertexBytecode{nullptr};
    size_t vertexSize{0};
    void const* fragmentBytecode{nullptr};
    size_t fragmentSize{0};
    rendercore::PipelineStateDesc pipelineState;
};

// === Material names to IDs ===
static std::unordered_map<std::string, BuiltinMaterialId> sMaterialNames = {
  {"post_process/copy", BuiltinMaterialId::PostProcessCopy},
  {"post_process/tonemap", BuiltinMaterialId::PostProcessTonemap},
  {"post_process/bloom", BuiltinMaterialId::PostProcessBloom},
  {"lighting/point", BuiltinMaterialId::LightPoint},
  {"lighting/spot", BuiltinMaterialId::LightSpot},
  {"lighting/directional", BuiltinMaterialId::LightDirectional},
};

// === Builtin shader entry implementation ===

class BuiltinShaderEntry : public rendercore::IShaderEntry {
public:
    BuiltinShaderData const* data{nullptr};

    void const* GetVertexBytecode() const override {
        return data ? data->vertexBytecode : nullptr;
    }
    std::size_t GetVertexBytecodeSize() const override {
        return data ? data->vertexSize : 0;
    }
    void const* GetFragmentBytecode() const override {
        return data ? data->fragmentBytecode : nullptr;
    }
    std::size_t GetFragmentBytecodeSize() const override {
        return data ? data->fragmentSize : 0;
    }
    rendercore::VertexFormatDesc const* GetVertexInput() const override {
        // Default vertex format: position + uv
        static rendercore::VertexFormatDesc desc{};
        return &desc;
    }
    rendercore::ShaderReflectionDesc const* GetVertexReflection() const override {
        static rendercore::ShaderReflectionDesc desc{};
        return &desc;
    }
    rendercore::ShaderReflectionDesc const* GetFragmentReflection() const override {
        static rendercore::ShaderReflectionDesc desc{};
        return &desc;
    }
};

// === BuiltinMaterials::Impl ===

struct CachedMaterial {
    rendercore::IRenderMaterial* material{nullptr};
    BuiltinMaterialId id;
    size_t memoryUsed{0};
};

struct BuiltinMaterials::Impl {
    rhi::IDevice* device{nullptr};
    std::string shaderPath{kDefaultShaderPath};
    std::unordered_map<uint32_t, CachedMaterial> cache;
    std::mutex mutex;
    size_t totalMemory{0};

    // Shader entries
    std::vector<std::unique_ptr<BuiltinShaderEntry>> shaderEntries;

    // Default pipeline states
    std::unique_ptr<rendercore::IRenderPipelineState> opaquePipelineState;
    std::unique_ptr<rendercore::IRenderPipelineState> postProcessPipelineState;
    std::unique_ptr<rendercore::IRenderPipelineState> transparentPipelineState;

    uint32_t MakeKey(BuiltinMaterialId id, uint32_t param = 0) {
        return (static_cast<uint32_t>(id) << 8) | (param & 0xFF);
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex);
        // Clean up materials
        for (auto& [key, cached] : cache) {
            if (cached.material) {
                auto* renderMat = dynamic_cast<material::RenderMaterial*>(cached.material);
                if (renderMat) {
                    material::DestroyRenderMaterial(renderMat);
                }
            }
        }
        cache.clear();
        totalMemory = 0;
        shaderEntries.clear();
    }

    void InitPipelineStates() {
        if (!device) return;

        // Create default pipeline state for opaque materials
        // (This would normally come from shader reflection or preset)
    }

    // Get or create shader entry for builtin material
    BuiltinShaderEntry* GetOrCreateShaderEntry(BuiltinMaterialId id) {
        auto entry = std::make_unique<BuiltinShaderEntry>();

        // Get shader data for this material ID
        BuiltinShaderData* shaderData = GetBuiltinShaderData(id);
        if (shaderData) {
            entry->data = shaderData;
        }

        BuiltinShaderEntry* ptr = entry.get();
        shaderEntries.push_back(std::move(entry));
        return ptr;
    }

    // Get shader data for builtin material ID
    BuiltinShaderData* GetBuiltinShaderData(BuiltinMaterialId id) {
        static std::unordered_map<BuiltinMaterialId, BuiltinShaderData> shaderDataMap = {
            // Post-process materials
            {BuiltinMaterialId::PostProcessCopy, {"post_process_copy"}},
            {BuiltinMaterialId::PostProcessTonemap, {"post_process_tonemap"}},
            {BuiltinMaterialId::PostProcessBloom, {"post_process_bloom"}},
            // Lighting materials
            {BuiltinMaterialId::LightPoint, {"light_point"}},
            {BuiltinMaterialId::LightSpot, {"light_spot"}},
            {BuiltinMaterialId::LightDirectional, {"light_directional"}},
        };

        auto it = shaderDataMap.find(id);
        return it != shaderDataMap.end() ? &it->second : nullptr;
    }

    // Get default pipeline state for material category
    rendercore::PipelineStateDesc GetDefaultPipelineState(BuiltinMaterialId id) {
        rendercore::PipelineStateDesc desc{};

        // Configure based on material type
        uint32_t idValue = static_cast<uint32_t>(id);

        // Post-process materials (no depth, additive blend)
        if (idValue >= 0 && idValue < 10) {
            desc.blendState = rhi::BlendStateDesc{};
            desc.depthStencilState = rhi::DepthStencilStateDesc{};
            desc.depthStencilState->depthEnable = false;
            desc.rasterizerState = rhi::RasterizerStateDesc{};
            desc.rasterizerState->cullMode = rhi::CullMode::None;
        }
        // Lighting materials
        else if (idValue >= 20 && idValue < 30) {
            desc.blendState = rhi::BlendStateDesc{};
            desc.blendState->enable = true;
            desc.blendState->srcBlend = rhi::Blend::One;
            desc.blendState->dstBlend = rhi::Blend::One;
            desc.depthStencilState = rhi::DepthStencilStateDesc{};
            desc.depthStencilState->depthEnable = true;
            desc.depthStencilState->depthWriteEnable = false;
            desc.rasterizerState = rhi::RasterizerStateDesc{};
        }
        // Shadow materials
        else if (idValue >= 30 && idValue < 40) {
            desc.blendState = rhi::BlendStateDesc{};
            desc.blendState->enable = false;
            desc.depthStencilState = rhi::DepthStencilStateDesc{};
            desc.depthStencilState->depthEnable = true;
            desc.depthStencilState->depthWriteEnable = true;
            desc.rasterizerState = rhi::RasterizerStateDesc{};
            desc.rasterizerState->cullMode = rhi::CullMode::Front;  // Peter panning mitigation
        }
        // Default
        else {
            desc.blendState = rhi::BlendStateDesc{};
            desc.depthStencilState = rhi::DepthStencilStateDesc{};
            desc.rasterizerState = rhi::RasterizerStateDesc{};
        }

        return desc;
    }

    // Create actual material
    rendercore::IRenderMaterial* CreateMaterial(BuiltinMaterialId id) {
        if (!device) return nullptr;

        // Get shader entry
        BuiltinShaderEntry* shaderEntry = GetOrCreateShaderEntry(id);
        if (!shaderEntry) return nullptr;

        // Get pipeline state
        rendercore::PipelineStateDesc pipelineState = GetDefaultPipelineState(id);

        // Create RenderMaterial
        auto* renderMat = material::CreateRenderMaterial(shaderEntry, pipelineState);
        if (!renderMat) return nullptr;

        renderMat->SetDevice(device);

        // Create GPU resources
        renderMat->CreateDeviceResource();

        return renderMat;
    }
};

// === BuiltinMaterials ===

BuiltinMaterials::BuiltinMaterials()
  : impl_(std::make_unique<Impl>()) {
}

BuiltinMaterials::~BuiltinMaterials() {
  impl_->Clear();
}

void BuiltinMaterials::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
  impl_->InitPipelineStates();
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessCopy() {
  return GetMaterial(BuiltinMaterialId::PostProcessCopy);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessTonemap() {
  return GetMaterial(BuiltinMaterialId::PostProcessTonemap);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloom() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloom);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloomDownsample() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloomDownsample);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloomUpsample() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloomUpsample);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessColorGrading() {
  return GetMaterial(BuiltinMaterialId::PostProcessColorGrading);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessVignette() {
  return GetMaterial(BuiltinMaterialId::PostProcessVignette);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessChromaticAberration() {
  return GetMaterial(BuiltinMaterialId::PostProcessChromaticAberration);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessFXAA() {
  return GetMaterial(BuiltinMaterialId::PostProcessFXAA);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessTAA() {
  return GetMaterial(BuiltinMaterialId::PostProcessTAA);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightPoint() {
  return GetMaterial(BuiltinMaterialId::LightPoint);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightSpot() {
  return GetMaterial(BuiltinMaterialId::LightSpot);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightDirectional() {
  return GetMaterial(BuiltinMaterialId::LightDirectional);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightAmbient() {
  return GetMaterial(BuiltinMaterialId::LightAmbient);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightIBL() {
  return GetMaterial(BuiltinMaterialId::LightIBL);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthOpaque() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthOpaque);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthAlphaTest() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthAlphaTest);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthVSM() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthVSM);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugWireframe() {
  return GetMaterial(BuiltinMaterialId::DebugWireframe);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugNormals() {
  return GetMaterial(BuiltinMaterialId::DebugNormals);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugUVs() {
  return GetMaterial(BuiltinMaterialId::DebugUVs);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugDepth() {
  return GetMaterial(BuiltinMaterialId::DebugDepth);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugLightComplexity() {
  return GetMaterial(BuiltinMaterialId::DebugLightComplexity);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetClearColor() {
  return GetMaterial(BuiltinMaterialId::ClearColor);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetClearDepth() {
  return GetMaterial(BuiltinMaterialId::ClearDepth);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetBlit() {
  return GetMaterial(BuiltinMaterialId::Blit);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetMaterial(
    BuiltinMaterialId id,
    BuiltinMaterialParams const* params) {

  uint32_t key = impl_->MakeKey(id, params ? params->renderPassIndex : 0);

  std::lock_guard<std::mutex> lock(impl_->mutex);

  auto it = impl_->cache.find(key);
  if (it != impl_->cache.end()) {
    return it->second.material;
  }

  // Create new material
  CachedMaterial cached;
  cached.id = id;
  cached.material = impl_->CreateMaterial(id);
  cached.memoryUsed = sizeof(material::RenderMaterial);  // Approximate

  impl_->cache[key] = cached;
  impl_->totalMemory += cached.memoryUsed;

  return cached.material;
}

rendercore::IRenderMaterial* BuiltinMaterials::GetMaterialByName(char const* name) {
  if (!name) return nullptr;

  auto it = sMaterialNames.find(name);
  if (it != sMaterialNames.end()) {
    return GetMaterial(it->second);
  }
  return nullptr;
}

void BuiltinMaterials::WarmupCache() {
  // Create commonly used materials upfront
  GetPostProcessCopy();
  GetPostProcessTonemap();
  GetLightPoint();
  GetLightSpot();
  GetLightDirectional();
  GetShadowDepthOpaque();
}

void BuiltinMaterials::SetShaderSearchPath(char const* path) {
  if (path) {
    impl_->shaderPath = path;
  }
}

bool BuiltinMaterials::IsCached(BuiltinMaterialId id) const {
  std::lock_guard<std::mutex> lock(impl_->mutex);
  return impl_->cache.find(static_cast<uint32_t>(id)) != impl_->cache.end();
}

void BuiltinMaterials::ClearCache() {
  impl_->Clear();
}

size_t BuiltinMaterials::GetMemoryUsed() const {
  return impl_->totalMemory;
}

// === Global Instance ===

static std::unique_ptr<BuiltinMaterials> g_builtinMaterials;
static std::mutex g_builtinMutex;

BuiltinMaterials* GetBuiltinMaterials() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMaterials) {
    g_builtinMaterials = std::make_unique<BuiltinMaterials>();
  }
  return g_builtinMaterials.get();
}

void InitializeBuiltinMaterials(rhi::IDevice* device) {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMaterials) {
    g_builtinMaterials = std::make_unique<BuiltinMaterials>();
  }
  g_builtinMaterials->SetDevice(device);
}

void ShutdownBuiltinMaterials() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  g_builtinMaterials.reset();
}

// === Free Functions ===

BuiltinMaterials* CreateBuiltinMaterials() {
  return new BuiltinMaterials();
}

void DestroyBuiltinMaterials(BuiltinMaterials* materials) {
  delete materials;
}

// === Shader Path Helpers ===

char const* GetBuiltinShaderPath(char const* shaderName) {
  static std::string path;
  path = kDefaultShaderPath;
  path += shaderName;
  return path.c_str();
}

bool BuiltinShaderExists(char const* shaderName) {
  // Would check if shader file exists
  // For now, assume all builtin shaders exist
  (void)shaderName;
  return true;
}

}  // namespace te::pipeline
