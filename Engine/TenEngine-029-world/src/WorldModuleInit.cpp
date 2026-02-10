/**
 * @file WorldModuleInit.cpp
 * @brief 029-World: register ModelComponent, LevelAssetDesc/SceneNodeDesc/ModelAssetDesc with 002, Level factory with 013.
 */

#include <te/world/WorldModuleInit.h>
#include <te/world/ModelComponent.h>
#include <te/world/LevelAssetDesc.h>
#include <te/world/ModelAssetDesc.h>
#include <te/world/LevelResource.h>
#include <te/entity/ComponentRegistry.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>

namespace te {
namespace world {

// TypeId range for 029-owned types (avoid collision with 002/013)
static constexpr te::object::TypeId kLevelAssetDescTypeId = 29001u;
static constexpr te::object::TypeId kSceneNodeDescTypeId = 29002u;
static constexpr te::object::TypeId kModelAssetDescTypeId = 29003u;

void RegisterWorldModule() {
    te::entity::IComponentRegistry* reg = te::entity::GetComponentRegistry();
    if (reg) {
        reg->RegisterComponentType<ModelComponent>("ModelComponent");
    }

    te::object::TypeDescriptor levelDesc = {};
    levelDesc.id = kLevelAssetDescTypeId;
    levelDesc.name = "LevelAssetDesc";
    levelDesc.size = sizeof(LevelAssetDesc);
    levelDesc.properties = nullptr;
    levelDesc.propertyCount = 0;
    levelDesc.baseTypeId = te::object::kInvalidTypeId;
    levelDesc.createInstance = nullptr;
    te::object::TypeRegistry::RegisterType(levelDesc);

    te::object::TypeDescriptor nodeDesc = {};
    nodeDesc.id = kSceneNodeDescTypeId;
    nodeDesc.name = "SceneNodeDesc";
    nodeDesc.size = sizeof(SceneNodeDesc);
    nodeDesc.properties = nullptr;
    nodeDesc.propertyCount = 0;
    nodeDesc.baseTypeId = te::object::kInvalidTypeId;
    nodeDesc.createInstance = nullptr;
    te::object::TypeRegistry::RegisterType(nodeDesc);

    te::object::TypeDescriptor modelDesc = {};
    modelDesc.id = kModelAssetDescTypeId;
    modelDesc.name = "ModelAssetDesc";
    modelDesc.size = sizeof(ModelAssetDesc);
    modelDesc.properties = nullptr;
    modelDesc.propertyCount = 0;
    modelDesc.baseTypeId = te::object::kInvalidTypeId;
    modelDesc.createInstance = nullptr;
    te::object::TypeRegistry::RegisterType(modelDesc);

    te::resource::IResourceManager* resMgr = te::resource::GetResourceManager();
    if (resMgr) {
        resMgr->RegisterResourceFactory(te::resource::ResourceType::Level, &LevelResourceFactory::Create);
    }
}

}  // namespace world
}  // namespace te
