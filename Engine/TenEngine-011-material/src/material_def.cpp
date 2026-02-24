/**
 * @file material_def.cpp
 * @brief MaterialSystemImpl implementation.
 */

#include "material_system_impl.hpp"
#include <te/material/MaterialResource.h>
#include <te/material/RenderMaterial.hpp>
#include <te/shader/ShaderCollection.h>
#include <te/shader/types.hpp>
#include <te/resource/ResourceManager.h>
#include <te/rendercore/uniform_layout.hpp>

#include <cstring>
#include <map>
#include <memory>
#include <vector>

namespace te {
namespace material {

// Internal material data structure
struct MaterialData {
    MaterialHandle handle;
    std::unique_ptr<MaterialResource> resource;
    std::unique_ptr<RenderMaterial> renderMaterial;

    struct SlotInfo {
        std::string name;
        uint32_t type;
        uint32_t count;
    };

    std::vector<SlotInfo> slotMapping;
    std::map<std::string, SlotInfo> nameToSlot;
};

struct MaterialInstanceData {
    MaterialInstanceHandle handle;
    MaterialHandle parentHandle;
    std::map<std::string, std::vector<uint8_t>> parameterOverrides;
    std::map<uint32_t, rhi::ITexture*> textureOverrides;
};

class MaterialSystemImpl::Impl {
public:
    std::map<uint64_t, std::unique_ptr<MaterialData>> materials;
    std::map<uint64_t, std::unique_ptr<MaterialInstanceData>> instances;
    uint64_t nextMaterialId{1};
    uint64_t nextInstanceId{1};
    rhi::IDevice* device{nullptr};

    MaterialData* GetMaterial(MaterialHandle h) {
        auto it = materials.find(h.id);
        return it != materials.end() ? it->second.get() : nullptr;
    }

    MaterialInstanceData* GetInstance(MaterialInstanceHandle h) {
        auto it = instances.find(h.id);
        return it != instances.end() ? it->second.get() : nullptr;
    }
};

MaterialSystemImpl::MaterialSystemImpl(te::shader::IShaderCompiler* compiler) 
    : compiler_(compiler), impl_(new Impl()) {}

MaterialSystemImpl::~MaterialSystemImpl() = default;

void MaterialSystemImpl::SetDevice(rhi::IDevice* device) {
    impl_->device = device;
}

MaterialHandle MaterialSystemImpl::Load(char const* path) {
    if (!path) return MaterialHandle{};

    auto* rm = resource::GetResourceManager();
    if (!rm) return MaterialHandle{};

    // Try to load as .material file
    auto* res = rm->LoadSync(path, resource::ResourceType::Material);
    if (!res) return MaterialHandle{};

    auto* matRes = dynamic_cast<MaterialResource*>(res);
    if (!matRes) {
        res->Release();
        return MaterialHandle{};
    }

    // Create material data
    auto data = std::make_unique<MaterialData>();
    data->handle.id = impl_->nextMaterialId;
    data->resource.reset(matRes);

    // Build slot mapping from resource parameters
    uint32_t slotIndex = 0;
    for (auto const& [name, param] : matRes->GetParams()) {
        MaterialData::SlotInfo slot{};
        slot.name = name;
        slot.type = static_cast<uint32_t>(param.type);
        slot.count = param.count;
        data->slotMapping.push_back(slot);
        data->nameToSlot[name] = slot;
        slotIndex++;
    }

    MaterialHandle h = data->handle;
    impl_->materials[impl_->nextMaterialId] = std::move(data);
    impl_->nextMaterialId++;

    return h;
}

uint32_t MaterialSystemImpl::GetParameters(MaterialHandle h, 
                                           te::rendercore::UniformMember* outParams, 
                                           uint32_t maxCount) {
    auto* data = impl_->GetMaterial(h);
    if (!data || !data->resource) return 0;

    auto const& params = data->resource->GetParams();
    uint32_t count = static_cast<uint32_t>(params.size());
    if (count > maxCount) count = maxCount;

    if (outParams) {
        uint32_t i = 0;
        for (auto const& [name, param] : params) {
            if (i >= count) break;
            std::strncpy(outParams[i].name, name.c_str(), sizeof(outParams[i].name) - 1);
            outParams[i].name[sizeof(outParams[i].name) - 1] = '\0';
            outParams[i].type = param.type;
            outParams[i].count = param.count;
            outParams[i].offset = 0;  // Will be filled from shader reflection
            outParams[i].size = param.GetTotalSize();
            i++;
        }
    }

    return count;
}

bool MaterialSystemImpl::GetDefaultValues(MaterialHandle h, void* outData, size_t maxSize) {
    auto* data = impl_->GetMaterial(h);
    if (!data || !data->resource || !outData) return false;

    auto const& params = data->resource->GetParams();
    size_t offset = 0;

    for (auto const& [name, param] : params) {
        size_t size = param.GetTotalSize();
        if (offset + size > maxSize) break;
        std::memcpy(static_cast<uint8_t*>(outData) + offset, param.data.data(), size);
        offset += size;
    }

    return true;
}

te::shader::IShaderHandle* MaterialSystemImpl::GetShaderRef(MaterialHandle h) {
    auto* data = impl_->GetMaterial(h);
    if (!data || !data->resource) return nullptr;

    auto shaderGuid = data->resource->GetShaderGuid();
    if (shaderGuid.IsNull()) return nullptr;

    auto* collection = shader::ShaderCollection::GetInstance();
    if (!collection) return nullptr;

    // ShaderCollectionEntry is IShaderHandle
    return const_cast<te::shader::IShaderHandle*>(
        reinterpret_cast<te::shader::IShaderHandle const*>(collection->Get(shaderGuid)));
}

uint32_t MaterialSystemImpl::GetTextureRefs(MaterialHandle h, 
                                            ParameterSlot* outSlots, 
                                            char const** outPaths, 
                                            uint32_t maxCount) {
    auto* data = impl_->GetMaterial(h);
    if (!data || !data->resource) return 0;

    auto const& textureSlots = data->resource->GetTextureSlots();
    uint32_t count = static_cast<uint32_t>(textureSlots.size());
    if (count > maxCount) count = maxCount;

    for (uint32_t i = 0; i < count; ++i) {
        if (outSlots) {
            outSlots[i].name = textureSlots[i].first.c_str();
            outSlots[i].type = static_cast<uint32_t>(te::rendercore::UniformMemberType::Int);  // Texture slot
            outSlots[i].count = 1;
        }
        if (outPaths) {
            outPaths[i] = textureSlots[i].second.c_str();
        }
    }

    return count;
}

te::rendercore::IUniformLayout* MaterialSystemImpl::GetUniformLayout(MaterialHandle h) {
    auto* shaderHandle = GetShaderRef(h);
    if (!shaderHandle) return nullptr;

    auto* collection = shader::ShaderCollection::GetInstance();
    if (!collection) return nullptr;

    // Get reflection from shader
    auto* entry = reinterpret_cast<shader::ShaderCollectionEntry const*>(shaderHandle);
    if (!entry || !entry->fragmentReflection.uniformBlock.members) return nullptr;

    te::rendercore::UniformLayoutDesc desc{};
    desc.members = entry->fragmentReflection.uniformBlock.members;
    desc.memberCount = entry->fragmentReflection.uniformBlock.memberCount;
    desc.totalSize = entry->fragmentReflection.uniformBlock.totalSize;

    return te::rendercore::CreateUniformLayout(desc);
}

void MaterialSystemImpl::SetScalar(MaterialInstanceHandle h, 
                                   ParameterSlot slot, 
                                   void const* data, 
                                   size_t size) {
    auto* instance = impl_->GetInstance(h);
    if (!instance || !data || size == 0) return;

    std::string name(slot.name ? slot.name : "");
    if (name.empty()) return;

    auto& param = instance->parameterOverrides[name];
    param.resize(size);
    std::memcpy(param.data(), data, size);
}

void MaterialSystemImpl::SetTexture(MaterialInstanceHandle h, 
                                    ParameterSlot slot, 
                                    void* texture) {
    auto* instance = impl_->GetInstance(h);
    if (!instance) return;

    uint32_t binding = slot.binding;
    instance->textureOverrides[binding] = reinterpret_cast<rhi::ITexture*>(texture);
}

void MaterialSystemImpl::SetBuffer(MaterialInstanceHandle h, 
                                   ParameterSlot slot, 
                                   void* buffer) {
    (void)h;
    (void)slot;
    (void)buffer;
    // Not implemented for now
}

uint32_t MaterialSystemImpl::GetSlotMapping(MaterialHandle h,
                                            ParameterSlot* outSlots,
                                            uint32_t maxCount) {
    auto* data = impl_->GetMaterial(h);
    if (!data) return 0;

    uint32_t count = static_cast<uint32_t>(data->slotMapping.size());
    if (count > maxCount) count = maxCount;

    if (outSlots) {
        for (uint32_t i = 0; i < count; ++i) {
            outSlots[i].name = data->slotMapping[i].name.c_str();
            outSlots[i].type = data->slotMapping[i].type;
            outSlots[i].count = data->slotMapping[i].count;
            outSlots[i].set = 0;
            outSlots[i].binding = i;
        }
    }

    return count;
}

MaterialInstanceHandle MaterialSystemImpl::CreateInstance(MaterialHandle h) {
    auto* data = impl_->GetMaterial(h);
    if (!data) return MaterialInstanceHandle{};

    auto instance = std::make_unique<MaterialInstanceData>();
    instance->handle.id = impl_->nextInstanceId;
    instance->parentHandle = h;

    MaterialInstanceHandle result = instance->handle;
    impl_->instances[impl_->nextInstanceId] = std::move(instance);
    impl_->nextInstanceId++;

    return result;
}

void MaterialSystemImpl::ReleaseInstance(MaterialInstanceHandle h) {
    auto it = impl_->instances.find(h.id);
    if (it != impl_->instances.end()) {
        impl_->instances.erase(it);
    }
}

te::shader::VariantKey MaterialSystemImpl::GetVariantKey(MaterialHandle h) {
    // For now, return empty variant key
    (void)h;
    return te::shader::VariantKey{};
}

void MaterialSystemImpl::SubmitToPipeline(MaterialInstanceHandle h, void* pipeline) {
    auto* instance = impl_->GetInstance(h);
    if (!instance || !pipeline || !impl_->device) return;

    auto* renderMaterial = reinterpret_cast<RenderMaterial*>(pipeline);
    if (!renderMaterial) return;

    // Apply instance parameter overrides to render material
    for (auto const& [name, data] : instance->parameterOverrides) {
        renderMaterial->SetDataParameter(name.c_str(), data.data(), data.size());
    }

    // Apply texture overrides
    for (auto const& [binding, texture] : instance->textureOverrides) {
        renderMaterial->SetDataTexture(binding, texture);
    }

    // Update GPU resources
    renderMaterial->UpdateDeviceResource(impl_->device, 0);
}

RenderMaterial* MaterialSystemImpl::CreateRenderMaterial(MaterialHandle h) {
    auto* data = impl_->GetMaterial(h);
    if (!data || !data->resource) return nullptr;

    auto shaderGuid = data->resource->GetShaderGuid();
    if (shaderGuid.IsNull()) return nullptr;

    auto* collection = shader::ShaderCollection::GetInstance();
    if (!collection) return nullptr;

    auto const* shaderEntry = collection->Get(shaderGuid);
    if (!shaderEntry) return nullptr;

    // Create render material
    // ShaderCollectionEntry implements IShaderEntry, so we can cast
    auto* renderMat = new RenderMaterial();
    renderMat->SetShaderEntry(const_cast<rendercore::IShaderEntry*>(
        static_cast<rendercore::IShaderEntry const*>(shaderEntry)));
    renderMat->SetPipelineStateDesc(data->resource->GetPipelineStateDesc());
    renderMat->SetDevice(impl_->device);

    // Set CPU parameters from material resource
    auto const& params = data->resource->GetParams();
    for (auto const& [name, param] : params) {
        renderMat->SetDataParameter(name.c_str(), param.data.data(), param.data.size());
    }

    // Create GPU resources if device is available
    if (impl_->device) {
        renderMat->CreateDeviceResource();
    }

    return renderMat;
}

}  // namespace material
}  // namespace te
