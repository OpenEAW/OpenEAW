#pragma once

#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>

#include <ostream>

namespace khepri {
inline std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
    return os << "{" << v.x << "," << v.y << "}";
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    return os << "{" << v.x << "," << v.y << "," << v.z << "}";
}
} // namespace khepri
