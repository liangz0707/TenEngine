#ifndef TE_INPUT_MAP_DEVICE_HPP
#define TE_INPUT_MAP_DEVICE_HPP

#include "te_input/DeviceId.hpp"
#include "te_input/DeviceKind.hpp"
#include <cstddef>
#include <unordered_map>

namespace te_input {

/// Maps physical devices to logical DeviceId/DeviceKind (contract-declared).
/// Implementation-defined: platform_handle_or_index identifies the physical device;
/// after mapping, logical id and kind are used by input queries.
class MapDevice {
public:
    /// Map a physical device to a logical DeviceId and DeviceKind.
    /// platform_handle_or_index: implementation-defined (e.g. platform handle or connection index).
    void map_physical_device(void* platform_handle_or_index, DeviceKind kind, DeviceId id);

private:
    struct Key {
        void* handle{nullptr};
        bool operator==(const Key& o) const { return handle == o.handle; }
    };
    struct KeyHash {
        std::size_t operator()(const Key& k) const { return std::hash<void*>{}(k.handle); }
    };
    struct Entry { DeviceKind kind{DeviceKind::Keyboard}; DeviceId id{0}; };
    std::unordered_map<Key, Entry, KeyHash> map_;
};

} // namespace te_input

#endif
