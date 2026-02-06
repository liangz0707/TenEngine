#ifndef TE_ENTITY_ENTITY_ID_HPP
#define TE_ENTITY_ENTITY_ID_HPP

#include <functional>

namespace te::entity {

/// Opaque handle: entity unique id, 1:1 with Scene NodeId (contract).
struct EntityId {
  void* handle = nullptr;
};
inline bool operator==(EntityId a, EntityId b) { return a.handle == b.handle; }
inline bool operator!=(EntityId a, EntityId b) { return !(a == b); }
inline bool operator<(EntityId a, EntityId b) { return std::less<void*>{}(a.handle, b.handle); }

}

#endif
