/** TypeRegistry (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_TYPE_REGISTRY_HPP
#define TE_OBJECT_TYPE_REGISTRY_HPP

#include "TypeDescriptor.hpp"
#include "TypeId.hpp"

namespace te::object {

/** Central type registry: RegisterType, GetTypeByName/ById, CreateInstance (contract). */
class TypeRegistry {
public:
    /** Register type; duplicate TypeId rejected; duplicate name per implementation. Returns true on success. */
    static bool RegisterType(TypeDescriptor const& desc);

    /** Lookup by name; returns nullptr if not found. */
    static TypeDescriptor const* GetTypeByName(char const* name);

    /** Lookup by id; returns nullptr if not found. */
    static TypeDescriptor const* GetTypeById(TypeId id);

    /** Create instance for type; allocation semantics per contract (caller or implementation). */
    static void* CreateInstance(TypeId id);
};

}  // namespace te::object

#endif
