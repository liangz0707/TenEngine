/**
 * @file test_resource_manager.cpp
 * @brief Unit tests for ResourceManager (contract: specs/_contracts/013-resource-ABI.md).
 */

#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/core/engine.h>
#include <cassert>

using namespace te::resource;
using namespace te::core;

int main() {
    // Initialize core
    assert(Init(nullptr) == true);
    
    // Get ResourceManager
    IResourceManager* manager = GetResourceManager();
    assert(manager != nullptr);
    
    // Test GetCached with null ID
    ResourceId nullId{};
    IResource* cached = manager->GetCached(nullId);
    assert(cached == nullptr);
    
    // Test LoadSync with invalid path
    IResource* resource = manager->LoadSync("nonexistent_resource.mesh", ResourceType::Mesh);
    assert(resource == nullptr);  // Should fail
    
    // Test GetLoadStatus with invalid ID
    LoadStatus status = manager->GetLoadStatus(nullptr);
    assert(status == LoadStatus::Failed);
    
    // Test GetLoadProgress with invalid ID
    float progress = manager->GetLoadProgress(nullptr);
    assert(progress == 0.0f);
    
    // Test ResolvePath with null ID
    char const* path = manager->ResolvePath(nullId);
    assert(path == nullptr);
    
    // Test RegisterResourceFactory
    ResourceFactory factory = [](ResourceType) -> IResource* { return nullptr; };
    manager->RegisterResourceFactory(ResourceType::Mesh, factory);
    // No assertion - just verify it doesn't crash
    
    Shutdown();
    return 0;
}
