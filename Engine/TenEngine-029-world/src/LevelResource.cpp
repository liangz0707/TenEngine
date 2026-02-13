/**
 * @file LevelResource.cpp
 * @brief 029-World: Level resource implementation (IResource + ILevelResource).
 */

#include <te/world/LevelResource.h>
#include <te/world/LevelAssetDesc.h>
#include <te/resource/Resource.h>
#include <te/resource/Resource.inl>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceId.h>
#include <te/object/Guid.h>
#include <atomic>
#include <memory>

namespace te {
namespace resource {
template<>
struct AssetDescTypeName<te::world::LevelAssetDesc> {
    static const char* Get() { return "LevelAssetDesc"; }
};
}  // namespace resource
}  // namespace te

namespace te {
namespace world {

class LevelResource : public te::resource::IResource, public ILevelResource {
public:
    LevelResource() : m_refCount(1), m_guid(te::resource::ResourceId()) {}
    explicit LevelResource(LevelAssetDesc const& desc) : m_refCount(1), m_guid(te::resource::ResourceId()), m_desc(desc) {}

    te::resource::ResourceType GetResourceType() const override {
        return te::resource::ResourceType::Level;
    }
    te::resource::ResourceId GetResourceId() const override { return m_guid; }
    void Release() override {
        if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

    LevelAssetDesc const& GetLevelAssetDesc() const override { return m_desc; }

    bool Load(char const* path, te::resource::IResourceManager* manager) override {
        (void)manager;
        std::unique_ptr<LevelAssetDesc> loaded = LoadAssetDesc<LevelAssetDesc>(path);
        if (!loaded) return false;
        m_desc = std::move(*loaded);
        return true;
    }

    bool Save(char const* path, te::resource::IResourceManager* manager) override {
        (void)manager;
        return SaveAssetDesc<LevelAssetDesc>(path, &m_desc);
    }

    bool OnConvertSourceFile(char const*, void**, std::size_t*) override { return false; }
    void* OnCreateAssetDesc() override { return new LevelAssetDesc(); }

private:
    std::atomic<int> m_refCount;
    te::resource::ResourceId m_guid;
    LevelAssetDesc m_desc;
};

te::resource::IResource* CreateLevelResourceFromDesc(LevelAssetDesc const& desc) {
    return new LevelResource(desc);
}

}  // namespace world
}  // namespace te

namespace te {
namespace world {

te::resource::IResource* LevelResourceFactory::Create(te::resource::ResourceType type) {
    (void)type;
    return new LevelResource();
}

}  // namespace world
}  // namespace te
