#ifndef TE_INPUT_LOAD_CONFIG_HPP
#define TE_INPUT_LOAD_CONFIG_HPP

#include "te_input/BindingTable.hpp"
#include <cstddef>

namespace te_input {

/// Load binding config from path or memory into a BindingTable (contract-declared).
/// Config format: implementation-defined (e.g. JSON or simple text); documented in implementation.
class LoadConfig {
public:
    explicit LoadConfig(BindingTable& table);

    /// Load from file path. Returns true on success.
    bool load_config(char const* path);

    /// Load from memory. Returns true on success.
    bool load_config_from_memory(void const* data, std::size_t size);

private:
    BindingTable* table_{nullptr};
};

} // namespace te_input

#endif
