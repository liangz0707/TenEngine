/**
 * @file ShaderResource.cpp
 * @brief ShaderResource: Load/Save/Import with .shader desc + same-dir source file.
 */
#include <te/shader/ShaderResource.h>
#include <te/shader/ShaderAssetDesc.h>
#include <te/shader/factory.hpp>
#include <te/resource/Resource.h>
#include <te/resource/Resource.inl>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceId.h>
#include <te/object/Guid.h>
#include <te/core/platform.h>
#include <te/core/alloc.h>
#include <cstring>
#include <string>
#include <algorithm>

namespace te {
namespace resource {

template <>
struct AssetDescTypeName<shader::ShaderAssetDesc> {
  static const char* Get() { return "ShaderAssetDesc"; }
};

}  // namespace resource
}  // namespace te

namespace te {
namespace shader {

namespace {

std::string DirOf(char const* path) {
  if (!path) return std::string();
  std::string s(path);
  std::string::size_type p = s.find_last_of("/\\");
  if (p == std::string::npos) return std::string();
  return s.substr(0, p + 1);
}

ShaderSourceFormat SourceFormatFromExtension(char const* path) {
  std::string ext = te::core::PathGetExtension(path ? path : "");
  if (ext == ".hlsl") return ShaderSourceFormat::HLSL;
  if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".comp") return ShaderSourceFormat::GLSL;
  return ShaderSourceFormat::HLSL;
}

std::string ReplaceExtensionToShader(std::string const& sourcePath) {
  std::string dir = te::core::PathGetDirectory(sourcePath);
  std::string fn = te::core::PathGetFileName(sourcePath);
  std::string::size_type d = fn.find_last_of('.');
  if (d != std::string::npos) fn = fn.substr(0, d);
  return te::core::PathJoin(dir, fn + ".shader");
}

}  // namespace

ShaderResource::ShaderResource()
    : resourceId_(), refCount_(1), compiler_(CreateShaderCompiler()), handle_(nullptr), desc_(), sourceBlob_() {}

ShaderResource::~ShaderResource() {
  if (compiler_ && handle_) {
    compiler_->ReleaseHandle(handle_);
    handle_ = nullptr;
  }
  if (compiler_) {
    DestroyShaderCompiler(compiler_);
    compiler_ = nullptr;
  }
}

resource::ResourceType ShaderResource::GetResourceType() const {
  return resource::ResourceType::Shader;
}

resource::ResourceId ShaderResource::GetResourceId() const {
  return resourceId_;
}

void ShaderResource::Release() {
  if (refCount_ > 0) --refCount_;
}

void* ShaderResource::GetShaderHandle() const {
  return handle_;
}

void ShaderResource::OnLoadComplete() {}

void ShaderResource::OnPrepareSave() {}

bool ShaderResource::Load(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager || !compiler_) return false;

  std::unique_ptr<ShaderAssetDesc> desc = LoadAssetDesc<ShaderAssetDesc>(path);
  if (!desc || !desc->IsValid()) return false;

  std::string sourcePath = DirOf(path) + desc->sourceFileName;
  auto dataOpt = te::core::FileRead(sourcePath);
  if (!dataOpt.has_value() || dataOpt->empty()) return false;

  if (handle_) {
    compiler_->ReleaseHandle(handle_);
    handle_ = nullptr;
  }
  handle_ = compiler_->LoadSourceFromMemory(dataOpt->data(), dataOpt->size(), desc->sourceFormat);
  if (!handle_) return false;
  if (!compiler_->Compile(handle_, desc->compileOptions)) {
    compiler_->ReleaseHandle(handle_);
    handle_ = nullptr;
    return false;
  }

  resourceId_ = desc->guid;
  desc_ = *desc;
  sourceBlob_.assign(dataOpt->begin(), dataOpt->end());
  OnLoadComplete();
  return true;
}

bool ShaderResource::Save(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) return false;

  OnPrepareSave();
  if (!SaveAssetDesc<ShaderAssetDesc>(path, &desc_)) return false;

  std::string sourcePath = DirOf(path) + desc_.sourceFileName;
  if (!sourceBlob_.empty()) {
    if (!te::core::FileWrite(sourcePath, std::vector<std::uint8_t>(sourceBlob_.begin(), sourceBlob_.end()))) {
      return false;
    }
  }
  return true;
}

bool ShaderResource::Import(char const* sourcePath, resource::IResourceManager* manager) {
  if (!sourcePath || !manager || !compiler_) return false;

  auto dataOpt = te::core::FileRead(sourcePath);
  if (!dataOpt.has_value() || dataOpt->empty()) return false;

  if (handle_) {
    compiler_->ReleaseHandle(handle_);
    handle_ = nullptr;
  }
  ShaderSourceFormat fmt = SourceFormatFromExtension(sourcePath);
  handle_ = compiler_->LoadSourceFromMemory(dataOpt->data(), dataOpt->size(), fmt);
  if (!handle_) return false;
  CompileOptions opts;
  opts.stage = ShaderStage::Unknown;
  if (!compiler_->Compile(handle_, opts)) {
    compiler_->ReleaseHandle(handle_);
    handle_ = nullptr;
    return false;
  }

  resourceId_ = resource::ResourceId::Generate();
  desc_.guid = resourceId_;
  std::string fn = te::core::PathGetFileName(sourcePath);
  std::strncpy(desc_.sourceFileName, fn.c_str(), kShaderSourceFileNameMaxLen - 1);
  desc_.sourceFileName[kShaderSourceFileNameMaxLen - 1] = '\0';
  desc_.sourceFormat = fmt;
  desc_.compileOptions = opts;
  sourceBlob_.assign(dataOpt->begin(), dataOpt->end());

  std::string outPath = ReplaceExtensionToShader(sourcePath);
  if (!SaveAssetDesc<ShaderAssetDesc>(outPath.c_str(), &desc_)) return false;
  std::string sourceOutPath = te::core::PathJoin(te::core::PathGetDirectory(outPath), desc_.sourceFileName);
  if (!te::core::FileWrite(sourceOutPath, std::vector<std::uint8_t>(sourceBlob_.begin(), sourceBlob_.end()))) {
    return false;
  }
  return true;
}

bool ShaderResource::OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) {
  if (!sourcePath || !outData || !outSize) return false;
  auto dataOpt = te::core::FileRead(sourcePath);
  if (!dataOpt.has_value() || dataOpt->empty()) return false;
  std::size_t n = dataOpt->size();
  void* buf = te::core::Alloc(n, 1);
  if (!buf) return false;
  std::memcpy(buf, dataOpt->data(), n);
  *outData = buf;
  *outSize = n;
  return true;
}

void* ShaderResource::OnCreateAssetDesc() {
  void* ptr = te::core::Alloc(sizeof(ShaderAssetDesc), alignof(ShaderAssetDesc));
  if (ptr) new (ptr) ShaderAssetDesc();
  return ptr;
}

}  // namespace shader
}  // namespace te
