/**
 * @file math.h
 * @brief Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp (contract: 001-core-public-api.md).
 * Only contract-declared types and API; no GPU dependency.
 */
#ifndef TE_CORE_MATH_H
#define TE_CORE_MATH_H

namespace te {
namespace core {

using Scalar = float;

struct Vector2 {
  Scalar x = 0, y = 0;
  Scalar& operator[](int i);
  Scalar operator[](int i) const;
};

struct Vector3 {
  Scalar x = 0, y = 0, z = 0;
  Scalar& operator[](int i);
  Scalar operator[](int i) const;
};

struct Vector4 {
  Scalar x = 0, y = 0, z = 0, w = 0;
  Scalar& operator[](int i);
  Scalar operator[](int i) const;
};

struct Matrix3 {
  Scalar m[3][3] = {};
  Scalar* operator[](int row);
  Scalar const* operator[](int row) const;
};

struct Matrix4 {
  Scalar m[4][4] = {};
  Scalar* operator[](int row);
  Scalar const* operator[](int row) const;
};

struct Quaternion {
  Scalar x = 0, y = 0, z = 0, w = 1;
};

struct AABB {
  Vector3 min{}, max{};
};

struct Ray {
  Vector3 origin{}, direction{};
};

/** Linear interpolation: a + t * (b - a). */
Scalar Lerp(Scalar a, Scalar b, Scalar t);
Vector2 Lerp(Vector2 const& a, Vector2 const& b, Scalar t);
Vector3 Lerp(Vector3 const& a, Vector3 const& b, Scalar t);
Vector4 Lerp(Vector4 const& a, Vector4 const& b, Scalar t);

/** Dot product. */
Scalar Dot(Vector2 const& a, Vector2 const& b);
Scalar Dot(Vector3 const& a, Vector3 const& b);
Scalar Dot(Vector4 const& a, Vector4 const& b);

/** Cross product (3D). */
Vector3 Cross(Vector3 const& a, Vector3 const& b);

/** Length. */
Scalar Length(Vector2 const& v);
Scalar Length(Vector3 const& v);
Scalar Length(Vector4 const& v);

/** Normalize; returns zero vector if length is zero. */
Vector2 Normalize(Vector2 const& v);
Vector3 Normalize(Vector3 const& v);
Vector4 Normalize(Vector4 const& v);

}  // namespace core
}  // namespace te

#endif  // TE_CORE_MATH_H
