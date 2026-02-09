/**
 * @file test_resource.cpp
 * @brief Unit tests for IResource base class (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * Note: IResource is abstract, so we test through a minimal concrete implementation.
 */

#include <te/resource/Resource.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/core/engine.h>
#include <cassert>
#include <string>

using namespace te::resource;
using namespace te::core;

// Minimal test resource implementation
class TestResource : public IResource {
public:
    TestResource() : resourceId_(ResourceId::Generate()), refcount_(0) {}
    
    ResourceType GetResourceType() const override {
        return ResourceType::Custom;
    }
    
    ResourceId GetResourceId() const override {
        return resourceId_;
    }
    
    void Release() override {
        refcount_--;
    }
    
    int GetRefCount() const { return refcount_; }
    void IncrementRefCount() { refcount_++; }
    
    // Override Import pure virtual methods
    bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override {
        (void)sourcePath;
        *outData = nullptr;
        *outSize = 0;
        return false;  // Not implemented for test
    }
    
    void* OnCreateAssetDesc() override {
        return nullptr;  // Not implemented for test
    }
    
private:
    ResourceId resourceId_;
    int refcount_;
};

int main() {
    // Initialize core
    assert(Init(nullptr) == true);
    
    // Test resource creation
    TestResource resource;
    assert(resource.GetResourceType() == ResourceType::Custom);
    assert(!resource.GetResourceId().IsNull());
    
    // Test Release
    resource.IncrementRefCount();
    assert(resource.GetRefCount() == 1);
    resource.Release();
    assert(resource.GetRefCount() == 0);
    
    // Test Load with invalid manager (should not crash)
    bool loadResult = resource.Load("test.mesh", nullptr);
    assert(loadResult == false);  // Should fail with null manager
    
    // Test Save with invalid manager (should not crash)
    bool saveResult = resource.Save("test.mesh", nullptr);
    assert(saveResult == false);  // Should fail with null manager
    
    Shutdown();
    return 0;
}
