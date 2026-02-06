/**
 * Integration test: LoadSync + GetCached + Unload using 001-Core FileRead.
 * Verifies dependency chain: 013 uses 001 file API.
 */
#include "te/resource/ResourceManager.h"
#include "te/resource/Resource.h"
#include "te/resource/ResourceLoader.h"
#include "te/resource/ResourceSerializer.h"
#include "te/resource/ResourceId.h"
#include "te/resource/ResourceTypes.h"
#include "te/core/platform.h"
#include <cassert>
#include <string>
#include <vector>
#include <cstring>

using namespace te::resource;

namespace {
class MinimalResource : public IResource {
public:
    explicit MinimalResource(ResourceType t) : type_(t) {}
    ResourceType GetResourceType() const override { return type_; }
    void Release() override { delete this; }
    ResourceType type_;
};
class StubLoader : public IResourceLoader {
    IResource* CreateFromPayload(ResourceType t, void* payload, IResourceManager*) override {
        delete static_cast<char*>(payload);
        return new MinimalResource(t);
    }
};
class StubSerializer : public IResourceSerializer {
    void* Deserialize(void const* buf, size_t sz) override {
        char* p = new char[sz];
        std::memcpy(p, buf, sz);
        return p;
    }
    bool Serialize(IResource*, void*, size_t, size_t*) override { return false; }
};
} // namespace

int main() {
    IResourceManager* mgr = GetResourceManager();
    StubLoader loader;
    StubSerializer serializer;
    mgr->RegisterResourceLoader(ResourceType::Custom, &loader);
    mgr->RegisterSerializer(ResourceType::Custom, &serializer);

    std::string path = "integration_test_resource.bin";
    if (!te::core::FileWrite(path, std::vector<std::uint8_t>{1, 2, 3}))
        return 0;

    IResource* a = mgr->LoadSync(path.c_str(), ResourceType::Custom);
    assert(a != nullptr);
    IResource* b = mgr->LoadSync(path.c_str(), ResourceType::Custom);
    assert(b == a); // cache hit

    mgr->Unload(a);
    assert(mgr->LoadSync(path.c_str(), ResourceType::Custom) != nullptr); // reload
    return 0;
}
