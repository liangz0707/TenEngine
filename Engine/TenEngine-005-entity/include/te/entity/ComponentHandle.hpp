#ifndef TE_ENTITY_COMPONENT_HANDLE_HPP
#define TE_ENTITY_COMPONENT_HANDLE_HPP

#include <functional>

namespace te::entity {

/// Opaque handle: component instance (contract).
struct ComponentHandle {
  void* handle = nullptr;
};
inline bool operator==(ComponentHandle a, ComponentHandle b) { return a.handle == b.handle; }
inline bool operator!=(ComponentHandle a, ComponentHandle b) { return !(a == b); }
inline bool operator<(ComponentHandle a, ComponentHandle b) { return std::less<void*>{}(a.handle, b.handle); }

}

#endif
