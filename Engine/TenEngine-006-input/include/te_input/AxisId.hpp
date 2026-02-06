#ifndef TE_INPUT_AXIS_ID_HPP
#define TE_INPUT_AXIS_ID_HPP

#include <string>
#include <string_view>

namespace te_input {

/// Axis identifier: value type, comparable (contract-declared only).
struct AxisId {
    std::string name_;

    AxisId() = default;
    explicit AxisId(std::string name) : name_(std::move(name)) {}

    static AxisId from_name(std::string_view name) {
        return AxisId(std::string(name));
    }

    bool operator==(const AxisId& other) const { return name_ == other.name_; }
    bool operator!=(const AxisId& other) const { return !(*this == other); }
};

} // namespace te_input

#endif
