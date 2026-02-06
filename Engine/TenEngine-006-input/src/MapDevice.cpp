#include "te_input/MapDevice.hpp"

namespace te_input {

void MapDevice::map_physical_device(void* platform_handle_or_index, DeviceKind kind, DeviceId id) {
    Key k{platform_handle_or_index};
    map_[k] = Entry{kind, id};
}

} // namespace te_input
