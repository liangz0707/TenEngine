/**
 * Unit test: ResourceId, GetCached, LoadSync with stub loader/deserializer.
 * Uses 001-Core FileWrite to create a temp file, then LoadSync/GetCached/Unload.
 */
#include "te/resource/ResourceManager.h"
#include "te/resource/Resource.h"
#include "te/resource/ResourceLoader.h"
#include "te/resource/ResourceSerializer.h"
#include "te/resource/ResourceId.h"
#include "te/resource/ResourceTypes.h"
#include "te/core/platform.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <functional>

using namespace te::resource;

namespace {

class MinimalResource : public IResource {
public:
    explicit MinimalResource(ResourceType type) : type_(type) {}
    ResourceType GetResourceType() const override { return type_; }
    void Release() override { delete this; }
private:
    ResourceType type_;
};

void* StubDeserialize(void const* buffer, size_t size) {
    size_t* p = new size_t(size);
    (void)buffer;
    return p;
}

class StubLoader : public IResourceLoader {
public:
    IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager*) override {
        size_t* p = static_cast<size_t*>(payload);
        delete p;
        return new MinimalResource(type);
    }
};

class StubSerializerImpl : public IResourceSerializer {
public:
    void* Deserialize(void const* b, size_t s) override { return StubDeserialize(b, s); }
    bool Serialize(IResource*, void*, size_t, size_t*) override { return false; }
};

// Match ResourceManager's ResourceIdFromPath (16-byte deterministic)
ResourceId ResourceIdFromPath(char const* path) {
    ResourceId id{};
    std::hash<std::string> H;
    std::string s(path ? path : "");
    size_t h0 = H(s);
    size_t h1 = H(s + "__te_013");
    for (size_t i = 0; i < 8; ++i) {
        id.data[i] = static_cast<uint8_t>((h0 >> (i * 8)) & 0xff);
        id.data[8 + i] = static_cast<uint8_t>((h1 >> (i * 8)) & 0xff);
    }
    return id;
}

} // namespace

int main() {
    IResourceManager* mgr = GetResourceManager();
    StubLoader loader;
    StubSerializerImpl serializer;

    mgr->RegisterSerializer(ResourceType::Custom, &serializer);
    mgr->RegisterResourceLoader(ResourceType::Custom, &loader);

    std::string path = "test_custom_resource.bin";
    std::vector<std::uint8_t> bytes = { 0x01, 0x02 };
    bool written = te::core::FileWrite(path, bytes);
    if (!written) return 0; // skip if no write permission (e.g. read-only tree)

    IResource* r = mgr->LoadSync(path.c_str(), ResourceType::Custom);
    assert(r != nullptr);
    assert(r->GetResourceType() == ResourceType::Custom);

    ResourceId id = ResourceIdFromPath(path.c_str());

    IResource* cached = mgr->GetCached(id);
    assert(cached == r);

    char const* resolved = mgr->ResolvePath(id);
    assert(resolved != nullptr);
    assert(std::strcmp(resolved, path.c_str()) == 0);

    char path_buf[256];
    assert(mgr->ResolvePathCopy(id, path_buf, sizeof(path_buf)));
    assert(std::strcmp(path_buf, path.c_str()) == 0);

    mgr->Unload(r);
    assert(mgr->GetCached(id) == nullptr);

    return 0;
}
