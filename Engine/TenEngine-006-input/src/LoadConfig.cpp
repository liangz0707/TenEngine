#include "te_input/LoadConfig.hpp"
#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/DeviceKind.hpp"
#include "te_input/KeyCode.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace te_input {

LoadConfig::LoadConfig(BindingTable& table) : table_(&table) {}

bool LoadConfig::load_config(char const* path) {
    if (!path || !table_) return false;
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream buf;
    buf << f.rdbuf();
    std::string s = buf.str();
    return load_config_from_memory(s.data(), s.size());
}

bool LoadConfig::load_config_from_memory(void const* data, std::size_t size) {
    if (!data || !table_) return size == 0;
    // Simple format: lines "action <name> <kind> <code>" or "axis <name> <kind> <code>"
    // kind: 0=Keyboard, 1=Mouse, 2=Gamepad, 3=Touch. code = uint32.
    char const* p = static_cast<char const*>(data);
    char const* end = p + size;
    while (p < end) {
        while (p < end && (*p == ' ' || *p == '\r' || *p == '\n')) ++p;
        if (p >= end) break;
        if (std::strncmp(p, "action ", 7) == 0) {
            p += 7;
            char const* name_start = p;
            while (p < end && *p != ' ' && *p != '\n') ++p;
            if (p >= end) break;
            std::string name(name_start, p);
            while (p < end && *p == ' ') ++p;
            if (p >= end) break;
            int k = 0, c = 0;
            if (std::sscanf(p, "%d %u", &k, &c) != 2) break;
            table_->add_binding(ActionId::from_name(name), static_cast<DeviceKind>(k), static_cast<KeyCode>(c));
            while (p < end && *p != '\n') ++p;
        } else if (std::strncmp(p, "axis ", 5) == 0) {
            p += 5;
            char const* name_start = p;
            while (p < end && *p != ' ' && *p != '\n') ++p;
            if (p >= end) break;
            std::string name(name_start, p);
            while (p < end && *p == ' ') ++p;
            if (p >= end) break;
            int k = 0; unsigned c = 0;
            if (std::sscanf(p, "%d %u", &k, &c) != 2) break;
            table_->add_axis_binding(AxisId::from_name(name), static_cast<DeviceKind>(k), static_cast<KeyCode>(c));
            while (p < end && *p != '\n') ++p;
        } else
            while (p < end && *p != '\n') ++p;
    }
    return true;
}

} // namespace te_input
