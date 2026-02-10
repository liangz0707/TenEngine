/**
 * @file TextureResource.cpp
 * @brief TextureResource implementation.
 */
#include <te/texture/TextureResource.h>
#include <te/texture/TextureFactory.h>
#include <te/texture/TextureAssetDesc.h>
#include <te/texture/TextureDevice.h>
#include <te/texture/TextureSerialize.h>
#include <te/texture/detail/texture_data.hpp>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/resource/Resource.h>
#include <te/resource/Resource.inl>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceId.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <te/core/platform.h>
#include <cstring>
#include <string>
#include <algorithm>

namespace te {
namespace resource {

template <>
struct AssetDescTypeName<texture::TextureAssetDesc> {
  static const char* Get() { return "TextureAssetDesc"; }
};

}  // namespace resource

namespace texture {

namespace {

rendercore::TextureFormat ImageFormatToTextureFormat(import::ImageFormat fmt, bool isHDR) {
  if (isHDR) return rendercore::TextureFormat::RGBA32_Float;
  switch (fmt) {
    case import::ImageFormat::PNG:
    case import::ImageFormat::JPEG:
    case import::ImageFormat::BMP:
    case import::ImageFormat::TGA:
    case import::ImageFormat::GIF:
    default:
      return rendercore::TextureFormat::RGBA8_UNorm;
    case import::ImageFormat::HDR:
      return rendercore::TextureFormat::RGBA32_Float;
  }
}

/** Replace .texture extension with .texdata for data file path. */
std::string MakeTexdataPath(char const* path) {
  if (!path) return std::string();
  std::string s(path);
  std::string const ext = ".texture";
  if (s.size() >= ext.size() &&
      std::equal(ext.rbegin(), ext.rend(), s.rbegin())) {
    s.resize(s.size() - ext.size());
  }
  s += ".texdata";
  return s;
}

}  // namespace

TextureResource::TextureResource() {
  m_resourceId = resource::ResourceId::Generate();
}

TextureResource::~TextureResource() {
  CleanupGPUResources();
  if (m_textureHandle) {
    ReleaseTexture(m_textureHandle);
    m_textureHandle = nullptr;
  }
}

resource::ResourceType TextureResource::GetResourceType() const {
  return resource::ResourceType::Texture;
}

resource::ResourceId TextureResource::GetResourceId() const {
  return m_resourceId;
}

void TextureResource::Release() {
  if (m_refCount > 0) {
    --m_refCount;
  }
}

bool TextureResource::Load(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) return false;

  std::unique_ptr<TextureAssetDesc> desc = LoadAssetDesc<TextureAssetDesc>(path);
  if (!desc) return false;

  std::string dataPath = MakeTexdataPath(path);
  void* dataFileBuffer = nullptr;
  size_t dataFileSize = 0;
  if (!LoadDataFile(dataPath.c_str(), &dataFileBuffer, &dataFileSize)) {
    return false;
  }

  if (dataFileSize < desc->pixelDataSize) {
    te::core::Free(dataFileBuffer);
    return false;
  }

  desc->pixelData = dataFileBuffer;
  m_textureHandle = CreateTexture(desc.get());
  te::core::Free(dataFileBuffer);
  if (!m_textureHandle) return false;

  OnLoadComplete();
  return true;
}

bool TextureResource::LoadAsync(char const* path, resource::IResourceManager* manager,
                                resource::LoadCompleteCallback on_done, void* user_data) {
  return IResource::LoadAsync(path, manager, on_done, user_data);
}

bool TextureResource::Save(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager || !m_textureHandle) return false;

  OnPrepareSave();

  detail::TextureData* data = static_cast<detail::TextureData*>(m_textureHandle);
  TextureAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Saved texture";
  desc.format = data->format;
  desc.width = data->width;
  desc.height = data->height;
  desc.depth = data->depth;
  desc.mipLevels = data->mipLevels;
  desc.pixelData = nullptr;
  desc.pixelDataSize = data->pixelDataSize;
  desc.isHDR = data->isHDR;

  if (!SaveAssetDesc<TextureAssetDesc>(path, &desc)) {
    return false;
  }

  std::string dataPath = MakeTexdataPath(path);
  size_t size = texture::GetPixelDataSize(m_textureHandle);
  if (size > 0) {
    void* buffer = te::core::Alloc(size, 16);
    if (!buffer) return false;
    size_t written = size;
    bool ok = SerializeTextureToBuffer(m_textureHandle, buffer, &written);
    if (ok) {
      ok = SaveDataFile(dataPath.c_str(), buffer, written);
    }
    te::core::Free(buffer);
    if (!ok) return false;
  }

  return true;
}

bool TextureResource::Import(char const* sourcePath, resource::IResourceManager* manager) {
  if (!sourcePath || !manager) return false;

  import::ImageImportResult result;
  if (!import::ImageImporterRegistry::GetInstance().ImportImage(sourcePath, result)) {
    return false;
  }

  if (!result.pixelData || result.pixelDataSize == 0 || result.width == 0 || result.height == 0) {
    if (result.pixelData) te::core::Free(result.pixelData);
    return false;
  }

  if (m_resourceId.IsNull()) {
    m_resourceId = GenerateGUID();
  }

  std::string outputPath = sourcePath;
  size_t dotPos = outputPath.find_last_of('.');
  if (dotPos != std::string::npos) {
    outputPath = outputPath.substr(0, dotPos);
  }
  outputPath += ".texture";

  TextureAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Imported texture";
  desc.format = ImageFormatToTextureFormat(result.format, result.isHDR);
  desc.width = result.width;
  desc.height = result.height;
  desc.depth = result.depth;
  desc.mipLevels = 1;
  desc.pixelData = result.pixelData;
  desc.pixelDataSize = result.pixelDataSize;
  desc.isHDR = result.isHDR;

  TextureAssetDesc descForSave = desc;
  descForSave.pixelData = nullptr;

  if (!SaveAssetDesc<TextureAssetDesc>(outputPath.c_str(), &descForSave)) {
    te::core::Free(result.pixelData);
    return false;
  }

  std::string dataPath = MakeTexdataPath(outputPath.c_str());
  if (!SaveDataFile(dataPath.c_str(), result.pixelData, result.pixelDataSize)) {
    te::core::Free(result.pixelData);
    return false;
  }

  m_textureHandle = CreateTexture(&desc);
  te::core::Free(result.pixelData);
  return m_textureHandle != nullptr;
}

void TextureResource::EnsureDeviceResources() {
  if (!m_textureHandle || !m_device) return;
  if (::te::texture::GetTextureHandle(m_textureHandle)) return;
  texture::EnsureDeviceResources(m_textureHandle, m_device);
}

void TextureResource::EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) {
  if (!m_textureHandle || !m_device) {
    if (on_done) on_done(user_data);
    return;
  }
  if (::te::texture::GetTextureHandle(m_textureHandle)) {
    if (on_done) on_done(user_data);
    return;
  }
  texture::EnsureDeviceResourcesAsync(m_textureHandle, m_device, on_done, user_data);
}

void TextureResource::OnLoadComplete() {}

void TextureResource::OnPrepareSave() {}

bool TextureResource::OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) {
  if (!sourcePath || !outData || !outSize) return false;
  import::ImageImportResult result;
  if (!import::ImageImporterRegistry::GetInstance().ImportImage(sourcePath, result)) {
    return false;
  }
  *outData = result.pixelData;
  *outSize = result.pixelDataSize;
  return true;
}

void* TextureResource::OnCreateAssetDesc() {
  void* p = te::core::Alloc(sizeof(TextureAssetDesc), alignof(TextureAssetDesc));
  if (p) {
    new (p) TextureAssetDesc();
  }
  return p;
}

void const* TextureResource::GetPixelData() const {
  return texture::GetPixelData(m_textureHandle);
}

size_t TextureResource::GetPixelDataSize() const {
  return texture::GetPixelDataSize(m_textureHandle);
}

uint32_t TextureResource::GetWidth() const {
  return texture::GetWidth(m_textureHandle);
}

uint32_t TextureResource::GetHeight() const {
  return texture::GetHeight(m_textureHandle);
}

uint32_t TextureResource::GetMipCount() const {
  return texture::GetMipCount(m_textureHandle);
}

te::rendercore::TextureFormat TextureResource::GetFormat() const {
  return texture::GetFormat(m_textureHandle);
}

rhi::ITexture* TextureResource::GetDeviceTexture() const {
  return ::te::texture::GetTextureHandle(m_textureHandle);
}

void TextureResource::CleanupGPUResources() {
  if (m_textureHandle && m_device) {
    DestroyDeviceTexture(m_textureHandle, m_device);
  }
}

}  // namespace texture
}  // namespace te
