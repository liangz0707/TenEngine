/**
 * Integration test: InitializeTextureModule with stub manager, and verify importer registry.
 */
#include <te/texture/TextureModuleInit.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceId.h>
#include <cassert>

namespace {

struct StubResourceManager : te::resource::IResourceManager {
  void RegisterResourceFactory(te::resource::ResourceType, te::resource::ResourceFactory) override {}
  te::resource::LoadRequestId RequestLoadAsync(char const*, te::resource::ResourceType,
                                              te::resource::LoadCompleteCallback, void*) override {
    return nullptr;
  }
  te::resource::LoadStatus GetLoadStatus(te::resource::LoadRequestId) const override {
    return te::resource::LoadStatus::Pending;
  }
  float GetLoadProgress(te::resource::LoadRequestId) const override { return 0.0f; }
  void CancelLoad(te::resource::LoadRequestId) override {}
  te::resource::IResource* GetCached(te::resource::ResourceId) const override { return nullptr; }
  te::resource::IResource* LoadSync(char const*, te::resource::ResourceType) override {
    return nullptr;
  }
  void Unload(te::resource::IResource*) override {}
  te::resource::StreamingHandle RequestStreaming(te::resource::ResourceId, int) override {
    return nullptr;
  }
  void SetStreamingPriority(te::resource::StreamingHandle, int) override {}
  bool Import(char const*, te::resource::ResourceType, void*) override { return false; }
  bool Save(te::resource::IResource*, char const*) override { return false; }
  char const* ResolvePath(te::resource::ResourceId) const override { return nullptr; }
};

}  // namespace

int main() {
  StubResourceManager stub;
  te::texture::InitializeTextureModule(&stub);

  te::texture::import::ImageFormat fmt =
      te::texture::import::ImageImporterRegistry::GetInstance().DetectFormat("test.png");
  assert(fmt == te::texture::import::ImageFormat::PNG);

  fmt = te::texture::import::ImageImporterRegistry::GetInstance().DetectFormat("test.jpg");
  assert(fmt == te::texture::import::ImageFormat::JPEG);

  return 0;
}
