/**
 * @file ShaderModuleInit.cpp
 * @brief Shader module initialization: register ShaderAssetDesc with 002, Shader factory with 013, LoadAllShaders.
 */
#include <te/shader/ShaderModuleInit.h>
#include <te/shader/ShaderResource.h>
#include <te/shader/ShaderAssetDesc.h>
#include <te/shader/ShaderCollection.h>
#include <te/shader/factory.hpp>
#include <te/shader/types.hpp>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ShaderResource.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>
#include <te/core/platform.h>
#include <te/core/alloc.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

namespace te {
namespace shader {

namespace {

constexpr object::TypeId kCompileOptionsTypeId = 0x01000001u;
constexpr object::TypeId kShaderAssetDescTypeId = 0x01000002u;

resource::IResource* CreateShaderResource(resource::ResourceType type) {
  if (type == resource::ResourceType::Shader) {
    return new ShaderResource();
  }
  return nullptr;
}

void* CreateCompileOptions() {
  void* ptr = te::core::Alloc(sizeof(CompileOptions), alignof(CompileOptions));
  if (ptr) {
    new (ptr) CompileOptions();
  }
  return ptr;
}

void* CreateShaderAssetDesc() {
  void* ptr = te::core::Alloc(sizeof(ShaderAssetDesc), alignof(ShaderAssetDesc));
  if (ptr) {
    new (ptr) ShaderAssetDesc();
  }
  return ptr;
}

object::PropertyDescriptor g_compileOptionsProperties[] = {
    {"targetBackend", 0, offsetof(CompileOptions, targetBackend), sizeof(BackendType), nullptr},
    {"optimizationLevel", 0, offsetof(CompileOptions, optimizationLevel), sizeof(uint32_t), nullptr},
    {"generateDebugInfo", 0, offsetof(CompileOptions, generateDebugInfo), sizeof(bool), nullptr},
    {"stage", 0, offsetof(CompileOptions, stage), sizeof(ShaderStage), nullptr},
    {"entryPoint", 0, offsetof(CompileOptions, entryPoint), sizeof(CompileOptions::entryPoint), nullptr},
};

object::PropertyDescriptor g_shaderAssetDescProperties[] = {
    {"guid", 0, offsetof(ShaderAssetDesc, guid), sizeof(resource::ResourceId), nullptr},
    {"sourceFileName", 0, offsetof(ShaderAssetDesc, sourceFileName), sizeof(ShaderAssetDesc::sourceFileName), nullptr},
    {"sourceFormat", 0, offsetof(ShaderAssetDesc, sourceFormat), sizeof(ShaderSourceFormat), nullptr},
    {"compileOptions", kCompileOptionsTypeId, offsetof(ShaderAssetDesc, compileOptions), sizeof(CompileOptions), nullptr},
};

}  // namespace

void InitializeShaderModule(resource::IResourceManager* manager) {
  if (!manager) {
    return;
  }

  object::TypeDescriptor compileOptionsType;
  compileOptionsType.id = kCompileOptionsTypeId;
  compileOptionsType.name = "CompileOptions";
  compileOptionsType.size = sizeof(CompileOptions);
  compileOptionsType.properties = g_compileOptionsProperties;
  compileOptionsType.propertyCount = sizeof(g_compileOptionsProperties) / sizeof(g_compileOptionsProperties[0]);
  compileOptionsType.baseTypeId = object::kInvalidTypeId;
  compileOptionsType.createInstance = CreateCompileOptions;
  object::TypeRegistry::RegisterType(compileOptionsType);

  object::TypeDescriptor shaderAssetDescType;
  shaderAssetDescType.id = kShaderAssetDescTypeId;
  shaderAssetDescType.name = "ShaderAssetDesc";
  shaderAssetDescType.size = sizeof(ShaderAssetDesc);
  shaderAssetDescType.properties = g_shaderAssetDescProperties;
  shaderAssetDescType.propertyCount = sizeof(g_shaderAssetDescProperties) / sizeof(g_shaderAssetDescProperties[0]);
  shaderAssetDescType.baseTypeId = object::kInvalidTypeId;
  shaderAssetDescType.createInstance = CreateShaderAssetDesc;
  object::TypeRegistry::RegisterType(shaderAssetDescType);

  manager->RegisterResourceFactory(resource::ResourceType::Shader, CreateShaderResource);
}

bool LoadAllShaders(resource::IResourceManager* manager, char const* manifestPath) {
  if (!manager || !manifestPath) {
    return false;
  }
  auto dataOpt = te::core::FileRead(manifestPath);
  if (!dataOpt.has_value() || dataOpt->empty()) {
    return false;
  }
  std::string content(dataOpt->begin(), dataOpt->end());
  std::vector<std::string> lines;
  std::string line;
  for (char c : content) {
    if (c == '\r') continue;
    if (c == '\n') {
      if (!line.empty()) {
        lines.push_back(line);
      }
      line.clear();
      continue;
    }
    line.push_back(c);
  }
  if (!line.empty()) {
    lines.push_back(line);
  }
  ShaderCollection* collection = ShaderCollection::GetInstance();
  collection->Clear();

  for (std::string const& path : lines) {
    resource::IResource* res = manager->LoadSync(path.c_str(), resource::ResourceType::Shader);
    if (!res) {
      return false;
    }
    resource::IShaderResource* shaderRes = dynamic_cast<resource::IShaderResource*>(res);
    IShaderHandle* handle = shaderRes ? static_cast<IShaderHandle*>(shaderRes->GetShaderHandle()) : nullptr;
    if (!handle) {
      res->Release();
      return false;
    }

    IShaderCompiler* compiler = CreateShaderCompiler();
    if (!compiler) {
      res->Release();
      return false;
    }

    CompileOptions opts{};
    opts.targetBackend = BackendType::SPIRV;

    ShaderCollectionEntry entry{};
    opts.stage = ShaderStage::Vertex;
    if (compiler->Compile(handle, opts)) {
      size_t vsSize = 0;
      void const* vsPtr = compiler->GetBytecodeForStage(handle, ShaderStage::Vertex, &vsSize);
      if (vsPtr && vsSize > 0) {
        entry.vertexBytecode.assign(static_cast<uint8_t const*>(vsPtr), static_cast<uint8_t const*>(vsPtr) + vsSize);
      }
      rendercore::VertexFormatDesc vertexInputDesc{};
      if (compiler->GetVertexInputReflection(handle, &vertexInputDesc)) {
        CopyVertexInputInto(vertexInputDesc, entry);
      }
      rendercore::ShaderReflectionDesc vertexRefl{};
      if (compiler->GetShaderReflection(handle, &vertexRefl)) {
        CopyVertexReflectionInto(vertexRefl, entry);
      }
    }
    opts.stage = ShaderStage::Fragment;
    if (compiler->Compile(handle, opts)) {
      size_t fsSize = 0;
      void const* fsPtr = compiler->GetBytecodeForStage(handle, ShaderStage::Fragment, &fsSize);
      if (fsPtr && fsSize > 0) {
        entry.fragmentBytecode.assign(static_cast<uint8_t const*>(fsPtr), static_cast<uint8_t const*>(fsPtr) + fsSize);
      }
      rendercore::ShaderReflectionDesc fragmentRefl{};
      if (compiler->GetShaderReflection(handle, &fragmentRefl)) {
        CopyFragmentReflectionInto(fragmentRefl, entry);
      }
    }

    if (entry.vertexBytecode.empty() && entry.fragmentBytecode.empty()) {
      DestroyShaderCompiler(compiler);
      res->Release();
      return false;
    }

    collection->Add(res->GetResourceId(), std::move(entry));
    DestroyShaderCompiler(compiler);
    res->Release();
  }
  return true;
}

void ShutdownShaderModule() {
  /* 002-Object TypeRegistry does not expose UnregisterType; types remain registered. */
}

}  // namespace shader
}  // namespace te
