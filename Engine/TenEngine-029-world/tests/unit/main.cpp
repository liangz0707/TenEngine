/**
 * @file main.cpp
 * @brief Test runner for 029-World module
 */

#include <te/core/engine.h>
#include <cassert>

namespace te {
namespace world {
    int test_world_manager();
}  // namespace world
}  // namespace te

int main() {
    te::core::Init(nullptr);

    int result = te::world::test_world_manager();

    te::core::Shutdown();
    return result;
}
