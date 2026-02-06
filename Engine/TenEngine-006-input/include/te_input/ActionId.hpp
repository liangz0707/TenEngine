#ifndef TE_INPUT_ACTION_ID_HPP
#define TE_INPUT_ACTION_ID_HPP

#include <string>
#include <string_view>

namespace te_input {

/// Action identifier: value type, comparable (contract-declared only).
struct ActionId {
    std::string name_;

    ActionId() = default;
    explicit ActionId(std::string name) : name_(std::move(name)) {}

    static ActionId from_name(std::string_view name) {
        return ActionId(std::string(name));
    }

    bool operator==(const ActionId& other) const { return name_ == other.name_; }
    bool operator!=(const ActionId& other) const { return !(*this == other); }
};

} // namespace te_input

#endif
