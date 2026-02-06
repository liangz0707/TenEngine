/**
 * @file ComponentRegistration.cpp
 * @brief Component type registration at engine startup
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#include <te/entity/ComponentRegistration.h>
#include <te/entity/ComponentRegistry.h>
#include <te/entity/Component.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>

namespace te {
namespace entity {

/**
 * @brief Register built-in component types at engine startup
 * 
 * This function should be called during engine initialization
 * to register all built-in component types with the Object module.
 * 
 * Note: Entity module does not provide any built-in component implementations.
 * All component types (TransformComponent, ModelComponent, ScriptComponent, etc.)
 * should be implemented and registered by their respective modules:
 * - TransformComponent: Can be implemented by Scene module or game code
 * - ModelComponent: Should be implemented by World module
 * - ScriptComponent: Should be implemented by Script module
 * - EffectComponent: Should be implemented by Effects module
 * - etc.
 * 
 * See docs/ComponentUsageGuide.md for how to implement and use components.
 */
void RegisterBuiltinComponentTypes() {
    // Entity module does not provide any built-in component implementations
    // All components should be implemented by their respective modules
    // See docs/ComponentUsageGuide.md for implementation guide
}

}  // namespace entity
}  // namespace te
