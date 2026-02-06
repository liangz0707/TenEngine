/**
 * @file ComponentRegistration.h
 * @brief Component type registration functions
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_COMPONENT_REGISTRATION_H
#define TE_ENTITY_COMPONENT_REGISTRATION_H

namespace te {
namespace entity {

/**
 * @brief Register built-in component types at engine startup
 * 
 * This function should be called during engine initialization
 * to register component types with the Object module (002-Object) for reflection support.
 * 
 * Note: Entity module does not provide any built-in component implementations.
 * All component types should be implemented and registered by their respective modules.
 * 
 * See docs/ComponentUsageGuide.md for how to implement and use components.
 * 
 * Call this function once at engine startup, typically in the engine initialization code.
 */
void RegisterBuiltinComponentTypes();

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_COMPONENT_REGISTRATION_H
