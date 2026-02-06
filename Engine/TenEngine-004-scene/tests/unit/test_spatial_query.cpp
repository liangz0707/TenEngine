/**
 * @file test_spatial_query.cpp
 * @brief Unit tests for SpatialQuery
 */

#include <te/scene/SpatialQuery.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <te/core/engine.h>
#include <cassert>
#include <vector>

namespace te {
namespace scene {

void TestAABBIntersection() {
    te::core::AABB a;
    a.min = {0, 0, 0};
    a.max = {10, 10, 10};
    
    te::core::AABB b;
    b.min = {5, 5, 5};
    b.max = {15, 15, 15};
    
    // Should intersect
    assert(SpatialQuery::AABBIntersects(a, b));
    
    te::core::AABB c;
    c.min = {20, 20, 20};
    c.max = {30, 30, 30};
    
    // Should not intersect
    assert(!SpatialQuery::AABBIntersects(a, c));
}

void TestAABBContainment() {
    te::core::AABB outer;
    outer.min = {0, 0, 0};
    outer.max = {10, 10, 10};
    
    te::core::AABB inner;
    inner.min = {2, 2, 2};
    inner.max = {8, 8, 8};
    
    // Inner should be contained in outer
    assert(SpatialQuery::AABBContains(inner, outer));
    
    te::core::AABB partial;
    partial.min = {5, 5, 5};
    partial.max = {15, 15, 15};
    
    // Partial should not be contained
    assert(!SpatialQuery::AABBContains(partial, outer));
}

void RunTestSpatialQuery() {
    TestAABBIntersection();
    TestAABBContainment();
}

}  // namespace scene
}  // namespace te
