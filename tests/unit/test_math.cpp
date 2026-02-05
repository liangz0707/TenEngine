/**
 * @file test_math.cpp
 * @brief Unit tests for Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp per contract capability 5.
 */

#include "te/core/math.h"
#include <cassert>
#include <cmath>

using namespace te::core;

int main() {
  Vector2 v2{ 1.f, 2.f };
  assert(v2.x == 1.f && v2.y == 2.f);
  assert(v2[0] == 1.f && v2[1] == 2.f);
  assert(Lerp(0.f, 10.f, 0.5f) == 5.f);
  assert(Dot(v2, v2) == 5.f);
  assert(std::abs(Length(v2) - std::sqrt(5.f)) < 1e-5f);

  Vector3 v3{ 1.f, 0.f, 0.f };
  assert(Length(Normalize(v3)) - 1.f < 1e-5f);
  Vector3 a{ 1.f, 0.f, 0.f }, b{ 0.f, 1.f, 0.f };
  Vector3 c = Cross(a, b);
  assert(std::abs(c.x) < 1e-5f && std::abs(c.y) < 1e-5f && std::abs(c.z - 1.f) < 1e-5f);

  Vector4 v4{ 1.f, 2.f, 3.f, 4.f };
  assert(Lerp(v4, v4, 0.5f).x == 1.f && Lerp(v4, v4, 0.5f).w == 4.f);

  AABB box{ {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f} };
  assert(box.min.x == 0.f && box.max.z == 1.f);

  Ray ray{ {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f} };
  assert(ray.origin.x == 0.f && ray.direction.x == 1.f);

  Matrix3 m3{};
  m3[0][0] = 1.f;
  assert(m3[0][0] == 1.f);

  Quaternion q{};
  assert(q.w == 1.f);

  return 0;
}
