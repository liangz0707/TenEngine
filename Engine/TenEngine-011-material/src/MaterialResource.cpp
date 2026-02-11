/**
 * @file MaterialResource.cpp
 * @brief MaterialResource: Load/Save .material JSON, resolve shader/textures by GUID, GetTextureRefs outputs GUID strings.
 */
#include <te/material/MaterialResource.h>
#include <te/material/material_json.hpp>
#include <te/resource/ResourceManager.h>
#include <te/resource/Resource.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ShaderResource.h>
#include <te/resource/TextureResource.h>
#include <te/resource/ResourceId.h>
#include <te/texture/TextureResource.h>
#include <te/object/Guid.h>
#include <te/shader/factory.hpp>
#include <te/shader/ShaderAssetDesc.h>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/uniform_buffer.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/resources.hpp>
#include <te/texture/TextureDevice.h>
#include <te/core/alloc.h>
#include <cstring>
#include <algorithm>

namespace te {
namespace material {

MaterialResource::MaterialResource()
    : resourceId_(), refCount_(1), shaderHandle_(nullptr), shaderResource_(nullptr), textureRefs_(), paramBuffer_(), jsonData_() {}

MaterialResource::~MaterialResource() {
  if (device_ && graphicsPSO_) {
    device_->DestroyPSO(graphicsPSO_);
    graphicsPSO_ = nullptr;
  }
  if (device_) {
    for (te::rhi::IPSO* p : graphicsPSOs_) { if (p) device_->DestroyPSO(p); }
    graphicsPSOs_.clear();
  }
  if (device_ && descriptorSet_) {
    device_->DestroyDescriptorSet(descriptorSet_);
    descriptorSet_ = nullptr;
  }
  if (device_ && descriptorSetLayout_) {
    device_->DestroyDescriptorSetLayout(descriptorSetLayout_);
    descriptorSetLayout_ = nullptr;
  }
  if (device_ && defaultSampler_) {
    device_->DestroySampler(defaultSampler_);
    defaultSampler_ = nullptr;
  }
  if (uniformBuffer_) {
    te::rendercore::ReleaseUniformBuffer(uniformBuffer_);
    uniformBuffer_ = nullptr;
  }
  if (uniformLayout_) {
    te::rendercore::ReleaseUniformLayout(uniformLayout_);
    uniformLayout_ = nullptr;
  }
  if (shaderResource_) {
    shaderResource_->Release();
    shaderResource_ = nullptr;
  }
  shaderHandle_ = nullptr;
  for (TextureEntry& e : textureRefs_) {
    if (e.textureResource) {
      e.textureResource->Release();
      e.textureResource = nullptr;
    }
  }
  textureRefs_.clear();
}

resource::ResourceType MaterialResource::GetResourceType() const {
  return resource::ResourceType::Material;
}

resource::ResourceId MaterialResource::GetResourceId() const {
  return resourceId_;
}

void MaterialResource::Release() {
  if (refCount_ > 0) --refCount_;
}

void MaterialResource::OnLoadComplete() {}
void MaterialResource::OnPrepareSave() {}

bool MaterialResource::Load(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) return false;

  MaterialJSONData data;
  if (!ParseMaterialJSON(path, data)) return false;

  resource::ResourceId shaderGuid = resource::ResourceId::FromString(data.shader.c_str());
  if (shaderGuid.IsNull()) return false;
  resource::IResource* shaderRes = manager->GetCached(shaderGuid);
  if (!shaderRes) return false;
  resource::IShaderResource* iShaderRes = dynamic_cast<resource::IShaderResource*>(shaderRes);
  if (!iShaderRes) return false;
  te::shader::IShaderHandle* handle = static_cast<te::shader::IShaderHandle*>(iShaderRes->GetShaderHandle());
  if (!handle) return false;

  te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
  if (!compiler) return false;
  te::rendercore::ShaderReflectionDesc refl = {};
  bool hasRefl = compiler->GetShaderReflection(handle, &refl);
  te::shader::DestroyShaderCompiler(compiler);
  if (!hasRefl) {
    /* no reflection: still accept, texture refs will be empty or we use (0,i) */
  }

  if (shaderResource_) {
    shaderResource_->Release();
    shaderResource_ = nullptr;
  }
  for (TextureEntry& e : textureRefs_) {
    if (e.textureResource) e.textureResource->Release();
  }
  textureRefs_.clear();
  paramBuffer_.clear();

  shaderResource_ = shaderRes;  /* GetCached already incremented refcount; we Release in dtor */
  shaderHandle_ = handle;

  for (auto const& kv : data.textures) {
    resource::ResourceId texGuid = resource::ResourceId::FromString(kv.second.c_str());
    if (texGuid.IsNull()) continue;
    resource::IResource* texRes = manager->GetCached(texGuid);
    if (!texRes) continue;
    resource::MaterialTextureSlot slot = {0, 0};
    if (hasRefl && refl.resourceBindings) {
      for (std::uint32_t i = 0; i < refl.resourceBindingCount; ++i) {
        if (refl.resourceBindings[i].kind == te::rendercore::ShaderResourceKind::SampledImage &&
            std::strcmp(refl.resourceBindings[i].name, kv.first.c_str()) == 0) {
          slot.set = refl.resourceBindings[i].set;
          slot.binding = refl.resourceBindings[i].binding;
          break;
        }
      }
    } else {
      slot.binding = static_cast<std::uint32_t>(textureRefs_.size());
    }
    TextureEntry e;
    e.slot = slot;
    e.guidString = kv.second;
    e.textureResource = texRes;
    textureRefs_.push_back(e);
  }

  if (hasRefl && refl.uniformBlock.members && refl.uniformBlock.memberCount > 0 && refl.uniformBlock.totalSize > 0) {
    paramBuffer_.resize(refl.uniformBlock.totalSize, 0);
    for (std::uint32_t i = 0; i < refl.uniformBlock.memberCount; ++i) {
      te::rendercore::UniformMember const& m = refl.uniformBlock.members[i];
      auto it = data.parameters.find(m.name);
      if (it == data.parameters.end()) continue;
      std::vector<double> const& v = it->second.values;
      if (v.empty()) continue;
      if (m.offset + m.size > paramBuffer_.size()) continue;
      std::uint8_t* dst = paramBuffer_.data() + m.offset;
      if (m.type == te::rendercore::UniformMemberType::Float && v.size() >= 1) {
        float f = static_cast<float>(v[0]);
        std::memcpy(dst, &f, 4);
      } else if (m.type == te::rendercore::UniformMemberType::Float4 && v.size() >= 4) {
        float f4[4] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) };
        std::memcpy(dst, f4, 16);
      } else if (m.type == te::rendercore::UniformMemberType::Float2 && v.size() >= 2) {
        float f2[2] = { static_cast<float>(v[0]), static_cast<float>(v[1]) };
        std::memcpy(dst, f2, 8);
      } else if (m.type == te::rendercore::UniformMemberType::Float3 && v.size() >= 3) {
        float f3[3] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) };
        std::memcpy(dst, f3, 12);
      } else if (m.type == te::rendercore::UniformMemberType::Int && v.size() >= 1) {
        int n = static_cast<int>(v[0]);
        std::memcpy(dst, &n, 4);
      }
      /* extend for Mat3/Mat4 etc. if needed */
    }
  }

  jsonData_ = std::move(data);
  if (!jsonData_.guid.empty()) {
    resourceId_ = resource::ResourceId::FromString(jsonData_.guid.c_str());
  }
  if (resourceId_.IsNull()) {
    resourceId_ = resource::ResourceId::Generate();
    jsonData_.guid = resourceId_.ToString();
  }
  OnLoadComplete();
  return true;
}

bool MaterialResource::Save(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) return false;
  OnPrepareSave();
  return SerializeMaterialJSON(path, jsonData_);
}

bool MaterialResource::Import(char const* sourcePath, resource::IResourceManager* manager) {
  (void)sourcePath;
  (void)manager;
  return false;  /* Import 可空实现 */
}

std::uint32_t MaterialResource::GetTextureRefs(resource::MaterialTextureSlot* outSlots, char const** outPaths,
                                               std::uint32_t maxCount) const {
  if (!outSlots || !outPaths || maxCount == 0) return 0;
  std::uint32_t n = static_cast<std::uint32_t>(textureRefs_.size());
  if (n > maxCount) n = maxCount;
  for (std::uint32_t i = 0; i < n; ++i) {
    outSlots[i] = textureRefs_[i].slot;
    outPaths[i] = textureRefs_[i].guidString.c_str();
  }
  return n;
}

bool MaterialResource::OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) {
  (void)sourcePath;
  (void)outData;
  (void)outSize;
  return false;
}

void* MaterialResource::OnCreateAssetDesc() {
  return nullptr;  /* Material uses JSON only, no 002 AssetDesc */
}

void MaterialResource::EnsureDeviceResources() {
  EnsureDeviceResources(nullptr, 0);
}

void MaterialResource::EnsureDeviceResources(te::rhi::IRenderPass* renderPass, uint32_t subpassCount) {
  if (!device_) return;
  for (TextureEntry& e : textureRefs_) {
    if (!e.textureResource) continue;
    te::texture::TextureResource* texRes = dynamic_cast<te::texture::TextureResource*>(e.textureResource);
    if (texRes) {
      texRes->SetDevice(device_);
      texRes->EnsureDeviceResources();
    }
  }
  te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
  if (!compiler) { deviceResourcesReady_ = true; return; }
  te::rendercore::ShaderReflectionDesc refl = {};
  bool hasRefl = shaderHandle_ && compiler->GetShaderReflection(shaderHandle_, &refl);
  if (!uniformBuffer_ && shaderHandle_ && !paramBuffer_.empty() && hasRefl &&
      refl.uniformBlock.members && refl.uniformBlock.memberCount > 0 &&
      refl.uniformBlock.totalSize > 0 &&
      paramBuffer_.size() >= refl.uniformBlock.totalSize) {
    uniformLayout_ = te::rendercore::CreateUniformLayout(refl.uniformBlock);
    if (uniformLayout_)
      uniformBuffer_ = te::rendercore::CreateUniformBuffer(uniformLayout_, device_);
    if (uniformBuffer_)
      uniformBuffer_->Update(paramBuffer_.data(), refl.uniformBlock.totalSize);
  }
  /* Build descriptor set layout from reflection: binding 0 = UB, then texture/sampler bindings */
  if (!descriptorSetLayout_ && !descriptorSet_ && hasRefl && device_) {
    te::rhi::DescriptorSetLayoutDesc layoutDesc = {};
    layoutDesc.bindingCount = 0;
    if (uniformBuffer_) {
      layoutDesc.bindings[layoutDesc.bindingCount].binding = 0;
      layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<std::uint32_t>(te::rhi::DescriptorType::UniformBuffer);
      layoutDesc.bindings[layoutDesc.bindingCount].descriptorCount = 1;
      layoutDesc.bindingCount++;
    }
    if (refl.resourceBindings) {
      for (std::uint32_t i = 0; i < refl.resourceBindingCount && layoutDesc.bindingCount < te::rhi::DescriptorSetLayoutDesc::kMaxBindings; ++i) {
        te::rendercore::ShaderResourceKind k = refl.resourceBindings[i].kind;
        if (k == te::rendercore::ShaderResourceKind::SampledImage || k == te::rendercore::ShaderResourceKind::Sampler) {
          layoutDesc.bindings[layoutDesc.bindingCount].binding = refl.resourceBindings[i].binding;
          layoutDesc.bindings[layoutDesc.bindingCount].descriptorType = static_cast<std::uint32_t>(te::rhi::DescriptorType::CombinedImageSampler);
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
    te::rhi::SamplerDesc sd = {};
    sd.filter = 1;  /* linear */
    defaultSampler_ = device_->CreateSampler(sd);
  }
  /* PSO: get bytecode for vertex and fragment, create graphics PSO with layout (single or per-subpass) */
  bool wantMulti = (renderPass != nullptr && subpassCount > 0u);
  bool needPSO = shaderHandle_ && descriptorSetLayout_ && device_ &&
      (wantMulti ? (graphicsPSOs_.size() != subpassCount) : (graphicsPSO_ == nullptr));
  if (needPSO) {
    te::rhi::Backend backend = device_->GetBackend();
    te::shader::BackendType shaderBackend = te::shader::BackendType::SPIRV;
    if (backend == te::rhi::Backend::Vulkan)
      shaderBackend = te::shader::BackendType::SPIRV;
    else if (backend == te::rhi::Backend::D3D11)
      shaderBackend = te::shader::BackendType::DXBC;
    te::shader::CompileOptions compileOpts = {};
    compileOpts.targetBackend = shaderBackend;
    compileOpts.stage = te::shader::ShaderStage::Vertex;
    if (!compiler->Compile(shaderHandle_, compileOpts)) { te::shader::DestroyShaderCompiler(compiler); deviceResourcesReady_ = true; return; }
    size_t vsSize = 0, fsSize = 0;
    void const* vsPtr = compiler->GetBytecodeForStage(shaderHandle_, te::shader::ShaderStage::Vertex, &vsSize);
    if (!vsPtr || vsSize == 0) { te::shader::DestroyShaderCompiler(compiler); deviceResourcesReady_ = true; return; }
    std::vector<std::uint8_t> vsCopy(static_cast<size_t>(vsSize));
    std::memcpy(vsCopy.data(), vsPtr, vsSize);
    void const* fsPtr = compiler->GetBytecodeForStage(shaderHandle_, te::shader::ShaderStage::Fragment, &fsSize);
    if (!fsPtr || fsSize == 0) { te::shader::DestroyShaderCompiler(compiler); deviceResourcesReady_ = true; return; }
    te::rhi::GraphicsPSODesc psoDesc = {};
    psoDesc.vertex_shader = vsCopy.data();
    psoDesc.vertex_shader_size = vsSize;
    psoDesc.fragment_shader = fsPtr;
    psoDesc.fragment_shader_size = fsSize;
    if (renderPass && subpassCount > 0u) {
      for (te::rhi::IPSO* p : graphicsPSOs_) { if (p) device_->DestroyPSO(p); }
      graphicsPSOs_.clear();
      if (graphicsPSO_) { device_->DestroyPSO(graphicsPSO_); graphicsPSO_ = nullptr; }
      for (uint32_t i = 0; i < subpassCount; ++i) {
        te::rhi::IPSO* p = device_->CreateGraphicsPSO(psoDesc, descriptorSetLayout_, renderPass, i);
        if (p) graphicsPSOs_.push_back(p);
      }
    } else {
      for (te::rhi::IPSO* p : graphicsPSOs_) { if (p) device_->DestroyPSO(p); }
      graphicsPSOs_.clear();
      graphicsPSO_ = device_->CreateGraphicsPSO(psoDesc, descriptorSetLayout_);
    }
  }
  te::shader::DestroyShaderCompiler(compiler);
  deviceResourcesReady_ = true;
}

te::rhi::IPSO* MaterialResource::GetGraphicsPSO(std::uint32_t subpassIndex) const {
  if (!graphicsPSOs_.empty())
    return (subpassIndex < graphicsPSOs_.size()) ? graphicsPSOs_[subpassIndex] : nullptr;
  return (subpassIndex == 0) ? graphicsPSO_ : nullptr;
}

bool MaterialResource::IsDeviceReady() const {
  if (!device_ || !deviceResourcesReady_) return false;
  for (TextureEntry const& e : textureRefs_) {
    if (e.textureResource && !e.textureResource->IsDeviceReady())
      return false;
  }
  return true;
}

void MaterialResource::UpdateDescriptorSetForFrame(te::rhi::IDevice* device, uint32_t frameSlot) {
  if (!device || !descriptorSet_) return;
  if (uniformBuffer_) {
    uniformBuffer_->SetCurrentFrameSlot(static_cast<te::rendercore::FrameSlotId>(frameSlot));
    // Upload current params to this frame's slot so the ring buffer region is valid for the descriptor.
    size_t totalSize = uniformLayout_ ? uniformLayout_->GetTotalSize() : paramBuffer_.size();
    if (totalSize == 0 && !paramBuffer_.empty()) totalSize = paramBuffer_.size();
    if (totalSize > 0 && !paramBuffer_.empty())
      uniformBuffer_->Update(paramBuffer_.data(), totalSize);
  }
  std::vector<te::rhi::DescriptorWrite> writes;
  if (uniformBuffer_ && uniformBuffer_->GetBuffer()) {
    te::rhi::DescriptorWrite w = {};
    w.dstSet = descriptorSet_;
    w.binding = 0;
    w.type = static_cast<std::uint32_t>(te::rhi::DescriptorType::UniformBuffer);
    w.buffer = uniformBuffer_->GetBuffer();
    w.bufferOffset = uniformBuffer_->GetRingBufferOffset(static_cast<te::rendercore::FrameSlotId>(frameSlot));
    w.texture = nullptr;
    w.sampler = nullptr;
    writes.push_back(w);
  }
  for (TextureEntry const& e : textureRefs_) {
    te::texture::TextureResource* texRes = dynamic_cast<te::texture::TextureResource*>(e.textureResource);
    if (!texRes) continue;
    te::rhi::ITexture* tex = texRes->GetDeviceTexture();
    if (!tex) continue;
    te::rhi::DescriptorWrite w = {};
    w.dstSet = descriptorSet_;
    w.binding = e.slot.binding;
    w.type = static_cast<std::uint32_t>(te::rhi::DescriptorType::CombinedImageSampler);
    w.buffer = nullptr;
    w.bufferOffset = 0;
    w.texture = tex;
    w.sampler = defaultSampler_;
    writes.push_back(w);
  }
  if (!writes.empty())
    device->UpdateDescriptorSet(descriptorSet_, writes.data(), static_cast<std::uint32_t>(writes.size()));
}

}  // namespace material
}  // namespace te
