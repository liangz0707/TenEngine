/**
 * @file math.cpp
 * @brief Implementation of Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp per contract (001-core-public-api.md).
 * No GPU dependency. Comments in English.
 */

#include "te/core/math.h"
#include <cmath>
#include <cassert>

namespace te {
namespace core {

static Scalar* V2(Vector2& v, int i) { return i == 0 ? &v.x : &v.y; }
static Scalar const* V2(Vector2 const& v, int i) { return i == 0 ? &v.x : &v.y; }
static Scalar* V3(Vector3& v, int i) { return i == 0 ? &v.x : (i == 1 ? &v.y : &v.z); }
static Scalar const* V3(Vector3 const& v, int i) { return i == 0 ? &v.x : (i == 1 ? &v.y : &v.z); }
static Scalar* V4(Vector4& v, int i) { return i == 0 ? &v.x : (i == 1 ? &v.y : (i == 2 ? &v.z : &v.w)); }
static Scalar const* V4(Vector4 const& v, int i) { return i == 0 ? &v.x : (i == 1 ? &v.y : (i == 2 ? &v.z : &v.w)); }

Scalar& Vector2::operator[](int i) { assert(i >= 0 && i < 2); return *V2(*this, i); }
Scalar Vector2::operator[](int i) const { assert(i >= 0 && i < 2); return *V2(*this, i); }
Scalar& Vector3::operator[](int i) { assert(i >= 0 && i < 3); return *V3(*this, i); }
Scalar Vector3::operator[](int i) const { assert(i >= 0 && i < 3); return *V3(*this, i); }
Scalar& Vector4::operator[](int i) { assert(i >= 0 && i < 4); return *V4(*this, i); }
Scalar Vector4::operator[](int i) const { assert(i >= 0 && i < 4); return *V4(*this, i); }

Scalar* Matrix3::operator[](int row) { assert(row >= 0 && row < 3); return m[row]; }
Scalar const* Matrix3::operator[](int row) const { assert(row >= 0 && row < 3); return m[row]; }
Scalar* Matrix4::operator[](int row) { assert(row >= 0 && row < 4); return m[row]; }
Scalar const* Matrix4::operator[](int row) const { assert(row >= 0 && row < 4); return m[row]; }

Scalar Lerp(Scalar a, Scalar b, Scalar t) { return a + t * (b - a); }
Vector2 Lerp(Vector2 const& a, Vector2 const& b, Scalar t) {
  return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
}
Vector3 Lerp(Vector3 const& a, Vector3 const& b, Scalar t) {
  return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
}
Vector4 Lerp(Vector4 const& a, Vector4 const& b, Scalar t) {
  return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t), Lerp(a.w, b.w, t) };
}

Scalar Dot(Vector2 const& a, Vector2 const& b) { return a.x * b.x + a.y * b.y; }
Scalar Dot(Vector3 const& a, Vector3 const& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Scalar Dot(Vector4 const& a, Vector4 const& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

Vector3 Cross(Vector3 const& a, Vector3 const& b) {
  return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

Scalar Length(Vector2 const& v) { return std::sqrt(Dot(v, v)); }
Scalar Length(Vector3 const& v) { return std::sqrt(Dot(v, v)); }
Scalar Length(Vector4 const& v) { return std::sqrt(Dot(v, v)); }

Vector2 Normalize(Vector2 const& v) {
  Scalar l = Length(v);
  return l > 0 ? Vector2{ v.x / l, v.y / l } : Vector2{};
}
Vector3 Normalize(Vector3 const& v) {
  Scalar l = Length(v);
  return l > 0 ? Vector3{ v.x / l, v.y / l, v.z / l } : Vector3{};
}
Vector4 Normalize(Vector4 const& v) {
  Scalar l = Length(v);
  return l > 0 ? Vector4{ v.x / l, v.y / l, v.z / l, v.w / l } : Vector4{};
}

}  // namespace core
}  // namespace te
