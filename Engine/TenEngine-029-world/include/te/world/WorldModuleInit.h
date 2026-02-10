/**
 * @file WorldModuleInit.h
 * @brief 029-World: module init (e.g. register ModelComponent with 005-Entity).
 */

#ifndef TE_WORLD_WORLD_MODULE_INIT_H
#define TE_WORLD_WORLD_MODULE_INIT_H

namespace te {
namespace world {

/** Call once to register 029 component types (ModelComponent) with Entity/002-Object. */
void RegisterWorldModule();

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_WORLD_MODULE_INIT_H
