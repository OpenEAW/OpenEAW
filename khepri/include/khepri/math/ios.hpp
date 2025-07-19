#pragma once

#include "quaternion.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#include <iostream>

namespace khepri {

template <typename T>
std::ostream& operator<<(std::ostream& os, const BasicVector2<T>& v)
{
    return os << "{" << v.x << "," << v.y << "}";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const BasicVector3<T>& v)
{
    return os << "{" << v.x << "," << v.y << "," << v.z << "}";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const BasicVector4<T>& v)
{
    return os << "{" << v.x << "," << v.y << "," << v.z << "," << v.w << "}";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const BasicQuaternion<T>& q)
{
    return os << "{" << q.x << "," << q.y << "," << q.z << "," << q.w << "}";
}

} // namespace khepri
