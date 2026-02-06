/**
 * @file main.cpp
 * @brief Unified test runner for all Scene module tests
 */

#include <te/core/engine.h>
#include <cassert>

// Forward declarations of test functions
namespace te {
namespace scene {
    void RunTestSceneManager();
    void RunTestSceneWorld();
    void RunTestSpatialQuery();
    void RunTestNodeManagers();
    void RunTestOctree();
    void RunTestQuadtree();
}  // namespace scene
}  // namespace te

int main() {
    te::core::Init(nullptr);
    
    // Run all test suites
    te::scene::RunTestSceneManager();
    te::scene::RunTestSceneWorld();
    te::scene::RunTestSpatialQuery();
    te::scene::RunTestNodeManagers();
    te::scene::RunTestOctree();
    te::scene::RunTestQuadtree();
    
    te::core::Shutdown();
    return 0;
}
