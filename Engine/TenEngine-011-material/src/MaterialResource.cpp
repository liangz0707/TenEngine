/**
 * @file MaterialResource.cpp
 * @brief MaterialResource: Load/Save .material JSON; shader by GUID only, params by name, textures by name->GUID.
 */
#include <te/material/MaterialResource.h>
#include <te/material/material_json.hpp>
#include <te/material/MaterialParam.hpp>
#include <te/resource/ResourceId.h>
#include <te/shader/ShaderCollection.h>
#include <te/rendercore/shader_reflection.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <cstring>
#include <algorithm>

namespace te {
namespace material {

MaterialResource::MaterialResource()
    : resourceId_(), refCount_(1), shaderGuid_(), params_(), textureSlots_(), pipelineStateDesc_(), jsonData_(), loaded_(false) {}

MaterialResource::~MaterialResource() = default;

resource::ResourceType MaterialResource::GetResourceType() const {
  return resource::ResourceType::Material;
}

resource::ResourceId MaterialResource::GetResourceId() const {
  return resourceId_;
}

void MaterialResource::Release() {
  if (refCount_ > 0) --refCount_;
}

static void FillParamFromValues(te::rendercore::UniformMemberType type, std::vector<double> const& v, MaterialParam& out) {
  out.type = type;
  out.count = 0;
  out.data.clear();
  std::size_t elemSize = MaterialParam::GetElementSize(type);
  if (elemSize == 0) return;
  std::size_t n = 0;
  switch (type) {
    case te::rendercore::UniformMemberType::Float:  n = 1; break;
    case te::rendercore::UniformMemberType::Float2: n = 2; break;
    case te::rendercore::UniformMemberType::Float3: n = 3; break;
    case te::rendercore::UniformMemberType::Float4: n = 4; break;
    case te::rendercore::UniformMemberType::Mat3:   n = 9; break;
    case te::rendercore::UniformMemberType::Mat4:   n = 16; break;
    case te::rendercore::UniformMemberType::Int:    n = 1; break;
    case te::rendercore::UniformMemberType::Int2:   n = 2; break;
    case te::rendercore::UniformMemberType::Int3:   n = 3; break;
    case te::rendercore::UniformMemberType::Int4:   n = 4; break;
    default: return;
  }
  if (v.size() < n) return;
  out.data.resize(elemSize);
  std::uint8_t* dst = out.data.data();
  if (type == te::rendercore::UniformMemberType::Float && n >= 1) {
    float f = static_cast<float>(v[0]);
    std::memcpy(dst, &f, 4);
  } else if (type == te::rendercore::UniformMemberType::Float4 && n >= 4) {
    float f4[4] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) };
    std::memcpy(dst, f4, 16);
  } else if (type == te::rendercore::UniformMemberType::Float2 && n >= 2) {
    float f2[2] = { static_cast<float>(v[0]), static_cast<float>(v[1]) };
    std::memcpy(dst, f2, 8);
  } else if (type == te::rendercore::UniformMemberType::Float3 && n >= 3) {
    float f3[3] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) };
    std::memcpy(dst, f3, 12);
  } else if (type == te::rendercore::UniformMemberType::Int && n >= 1) {
    int i = static_cast<int>(v[0]);
    std::memcpy(dst, &i, 4);
  } else if (type == te::rendercore::UniformMemberType::Mat4 && n >= 16) {
    float m4[16];
    for (int j = 0; j < 16; ++j) m4[j] = static_cast<float>(v[j]);
    std::memcpy(dst, m4, 64);
  } else if (type == te::rendercore::UniformMemberType::Mat3 && n >= 9) {
    float m3[12] = {};
    for (int j = 0; j < 9; ++j) m3[j] = static_cast<float>(v[j]);
    std::memcpy(dst, m3, 48);
  }
}

bool MaterialResource::Load(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) return false;

  MaterialJSONData data;
  if (!ParseMaterialJSON(path, data)) return false;

  resource::ResourceId shaderGuid = resource::ResourceId::FromString(data.shader.c_str());
  if (shaderGuid.IsNull()) return false;

  params_.clear();
  textureSlots_.clear();

  shaderGuid_ = shaderGuid;

  /* Use ShaderCollection for reflection so we can fill params by type. */
  shader::ShaderCollectionEntry const* entry = nullptr;
  if (shader::ShaderCollection* coll = shader::ShaderCollection::GetInstance())
    entry = coll->Get(shaderGuid);

  if (entry && entry->fragmentReflection.uniformBlock.members && entry->fragmentReflection.uniformBlock.memberCount > 0) {
    te::rendercore::ShaderReflectionDesc const& refl = entry->fragmentReflection;
    for (std::uint32_t i = 0; i < refl.uniformBlock.memberCount; ++i) {
      te::rendercore::UniformMember const& m = refl.uniformBlock.members[i];
      auto it = data.parameters.find(m.name);
      if (it == data.parameters.end()) continue;
      MaterialParam param;
      FillParamFromValues(m.type, it->second.values, param);
      if (!param.data.empty())
        params_[m.name] = std::move(param);
    }
  }

  for (auto const& kv : data.textures)
    textureSlots_.push_back({ kv.first, kv.second });

  if (!data.guid.empty())
    resourceId_ = resource::ResourceId::FromString(data.guid.c_str());
  if (resourceId_.IsNull()) {
    resourceId_ = resource::ResourceId::Generate();
    data.guid = resourceId_.ToString();
  }
  jsonData_ = std::move(data);
  loaded_ = true;
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
  return false;
}

std::uint32_t MaterialResource::GetTextureRefs(resource::MaterialTextureSlot* outSlots, char const** outPaths,
                                               std::uint32_t maxCount) const {
  if (!outSlots || !outPaths || maxCount == 0) return 0;
  std::uint32_t n = static_cast<std::uint32_t>(textureSlots_.size());
  if (n > maxCount) n = maxCount;
  for (std::uint32_t i = 0; i < n; ++i) {
    outSlots[i].set = 0;
    outSlots[i].binding = i;
    outPaths[i] = textureSlots_[i].second.c_str();
  }
  return n;
}

bool MaterialResource::SetParameter(char const* name, te::rendercore::UniformMemberType type, void const* data, std::uint32_t count) {
  if (!name || !data) return false;
  std::size_t elemSize = MaterialParam::GetElementSize(type);
  if (elemSize == 0) return false;
  std::size_t n = (count == 0) ? 1u : count;
  std::size_t total = elemSize * n;
  MaterialParam p;
  p.type = type;
  p.count = (count == 0) ? 0u : count;
  p.data.resize(total);
  std::memcpy(p.data.data(), data, total);
  params_[name] = std::move(p);
  return true;
}

bool MaterialResource::GetParameter(char const* name, void* outData, std::size_t maxSize) const {
  if (!name || !outData) return false;
  auto it = params_.find(name);
  if (it == params_.end()) return false;
  MaterialParam const& p = it->second;
  std::size_t need = p.GetTotalSize();
  if (need > maxSize) return false;
  std::memcpy(outData, p.data.data(), need);
  return true;
}

void MaterialResource::SetTextureGuid(char const* name, char const* guid) {
  if (!name) return;
  std::string key(name);
  for (auto& slot : textureSlots_) {
    if (slot.first == key) {
      slot.second = guid ? guid : "";
      return;
    }
  }
  textureSlots_.push_back({ key, guid ? guid : "" });
}

bool MaterialResource::IsDeviceReady() const {
  return loaded_ || !shaderGuid_.IsNull();
}

bool MaterialResource::OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) {
  (void)sourcePath;
  (void)outData;
  (void)outSize;
  return false;
}

void* MaterialResource::OnCreateAssetDesc() {
  return nullptr;
}

void MaterialResource::OnPrepareSave() {
  jsonData_.guid = resourceId_.ToString();
  jsonData_.shader = shaderGuid_.ToString();
  jsonData_.textures.clear();
  for (auto const& p : textureSlots_)
    jsonData_.textures[p.first] = p.second;
  jsonData_.parameters.clear();
  for (auto const& kv : params_) {
    MaterialParamValue val;
    MaterialParam const& p = kv.second;
    if (p.type == te::rendercore::UniformMemberType::Float && p.data.size() >= 4) {
      float f;
      std::memcpy(&f, p.data.data(), 4);
      val.values.push_back(static_cast<double>(f));
    } else if (p.type == te::rendercore::UniformMemberType::Float4 && p.data.size() >= 16) {
      float f4[4];
      std::memcpy(f4, p.data.data(), 16);
      for (int i = 0; i < 4; ++i) val.values.push_back(static_cast<double>(f4[i]));
    } else if (p.type == te::rendercore::UniformMemberType::Float2 && p.data.size() >= 8) {
      float f2[2];
      std::memcpy(f2, p.data.data(), 8);
      val.values.push_back(static_cast<double>(f2[0]));
      val.values.push_back(static_cast<double>(f2[1]));
    } else if (p.type == te::rendercore::UniformMemberType::Float3 && p.data.size() >= 12) {
      float f3[3];
      std::memcpy(f3, p.data.data(), 12);
      for (int i = 0; i < 3; ++i) val.values.push_back(static_cast<double>(f3[i]));
    } else if (p.type == te::rendercore::UniformMemberType::Int && p.data.size() >= 4) {
      int i;
      std::memcpy(&i, p.data.data(), 4);
      val.values.push_back(static_cast<double>(i));
    } else if (p.type == te::rendercore::UniformMemberType::Mat4 && p.data.size() >= 64) {
      float m4[16];
      std::memcpy(m4, p.data.data(), 64);
      for (int i = 0; i < 16; ++i) val.values.push_back(static_cast<double>(m4[i]));
    } else if (p.type == te::rendercore::UniformMemberType::Mat3 && p.data.size() >= 48) {
      float m3[12];
      std::memcpy(m3, p.data.data(), 48);
      for (int i = 0; i < 9; ++i) val.values.push_back(static_cast<double>(m3[i]));
    }
    if (!val.values.empty())
      jsonData_.parameters[kv.first] = std::move(val);
  }
}

MaterialResource* CreateMaterialResourceFromShader(resource::ResourceId shaderGuid, PipelineStateDesc const& pipelineState) {
  MaterialResource* r = new MaterialResource();
  r->SetResourceId(resource::ResourceId::Generate());
  r->SetShaderGuid(shaderGuid);
  r->SetPipelineStateDesc(pipelineState);
  return r;
}

}  // namespace material
}  // namespace te
