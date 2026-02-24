/**
 * @file WorldModuleInit.cpp
 * @brief 029-World: register ModelComponent, LightComponent, CameraComponent with reflection.
 */

#include <te/world/WorldModuleInit.h>
#include <te/world/ModelComponent.h>
#include <te/world/LightComponent.h>
#include <te/world/CameraComponent.h>
#include <te/world/LevelAssetDesc.h>
#include <te/world/ModelAssetDesc.h>
#include <te/world/LevelResource.h>
#include <te/entity/ComponentRegistry.h>
#include <te/entity/PropertyReflection.h>
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

// Enum names for LightType
static const char* g_lightTypeNames[] = {
    "Point",
    "Directional",
    "Spot"
};

/**
 * @brief Register property metadata for all world components.
 */
static void RegisterComponentProperties() {
    te::entity::IPropertyRegistry* propReg = te::entity::GetPropertyRegistry();
    if (!propReg) return;

    // Register ModelComponent properties (handled specially in PropertyPanel)
    {
        te::entity::ComponentMeta meta{};
        meta.typeName = "ModelComponent";
        meta.typeSize = sizeof(ModelComponent);
        meta.properties = nullptr;  // ModelResourceId handled specially
        meta.propertyCount = 0;

        propReg->RegisterComponentMeta(meta);
    }

    // Register LightComponent properties
    {
        using namespace te::entity;

        static PropertyMeta lightProps[] = {
            // Type (enum)
            {
                "Type",
                PropertyValueType::Enum,
                offsetof(LightComponent, type),
                sizeof(LightType),
                {},
                g_lightTypeNames,
                3,
                0
            },
            // Color (float[3])
            {
                "Color",
                PropertyValueType::Float3,
                offsetof(LightComponent, color),
                sizeof(float) * 3,
                {0.0f, 0.0f, 0.01f, 1.0f, true}  // isColor = true
            },
            // Intensity
            {
                "Intensity",
                PropertyValueType::Float32,
                offsetof(LightComponent, intensity),
                sizeof(float),
                {0.0f, 100.0f, 0.01f, 1.0f, false}
            },
            // Range
            {
                "Range",
                PropertyValueType::Float32,
                offsetof(LightComponent, range),
                sizeof(float),
                {0.1f, 1000.0f, 0.1f, 1.0f, false}
            },
            // Direction (float[3])
            {
                "Direction",
                PropertyValueType::Float3,
                offsetof(LightComponent, direction),
                sizeof(float) * 3,
                {-1.0f, 1.0f, 0.01f, 1.0f, false}
            },
            // Spot Angle
            {
                "Spot Angle",
                PropertyValueType::Float32,
                offsetof(LightComponent, spotAngle),
                sizeof(float),
                {0.01f, 1.57f, 0.01f, 1.0f, false, true}  // isRadians = true
            }
        };

        ComponentMeta meta{};
        meta.typeName = "LightComponent";
        meta.typeSize = sizeof(LightComponent);
        meta.properties = lightProps;
        meta.propertyCount = sizeof(lightProps) / sizeof(lightProps[0]);

        propReg->RegisterComponentMeta(meta);
    }

    // Register CameraComponent properties
    {
        using namespace te::entity;

        static PropertyMeta cameraProps[] = {
            // FOV (radians, display as degrees)
            {
                "FOV",
                PropertyValueType::Float32,
                offsetof(CameraComponent, fovY),
                sizeof(float),
                {0.174f, 3.14f, 0.01f, 1.0f, false, true}  // isRadians = true
            },
            // Near Plane
            {
                "Near Plane",
                PropertyValueType::Float32,
                offsetof(CameraComponent, nearZ),
                sizeof(float),
                {0.001f, 100.0f, 0.01f, 1.0f, false}
            },
            // Far Plane
            {
                "Far Plane",
                PropertyValueType::Float32,
                offsetof(CameraComponent, farZ),
                sizeof(float),
                {1.0f, 100000.0f, 1.0f, 1.0f, false}
            },
            // Is Active
            {
                "Active",
                PropertyValueType::Bool,
                offsetof(CameraComponent, isActive),
                sizeof(bool),
                {}
            }
        };

        ComponentMeta meta{};
        meta.typeName = "CameraComponent";
        meta.typeSize = sizeof(CameraComponent);
        meta.properties = cameraProps;
        meta.propertyCount = sizeof(cameraProps) / sizeof(cameraProps[0]);

        propReg->RegisterComponentMeta(meta);
    }
}

void RegisterWorldModule() {
    // Register component types in component registry
    te::entity::IComponentRegistry* reg = te::entity::GetComponentRegistry();
    if (reg) {
        reg->RegisterComponentType<ModelComponent>("ModelComponent");
        reg->RegisterComponentType<LightComponent>("LightComponent");
        reg->RegisterComponentType<CameraComponent>("CameraComponent");
    }

    // Register component property metadata
    RegisterComponentProperties();

    // Register type descriptors
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

    // Register resource factory
    te::resource::IResourceManager* resMgr = te::resource::GetResourceManager();
    if (resMgr) {
        resMgr->RegisterResourceFactory(te::resource::ResourceType::Level, &LevelResourceFactory::Create);
    }
}

}  // namespace world
}  // namespace te
