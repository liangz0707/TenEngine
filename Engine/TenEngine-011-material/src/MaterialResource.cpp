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
#include <te/object/Guid.h>
#include <te/shader/factory.hpp>
#include <te/shader/ShaderAssetDesc.h>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/core/alloc.h>
#include <cstring>
#include <algorithm>

namespace te {
namespace material {

MaterialResource::MaterialResource()
    : resourceId_(), refCount_(1), shaderHandle_(nullptr), shaderResource_(nullptr), textureRefs_(), paramBuffer_(), jsonData_() {}

MaterialResource::~MaterialResource() {
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

}  // namespace material
}  // namespace te
