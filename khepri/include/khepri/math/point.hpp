#pragma once

#include "vector2.hpp"

#include <cmath>
#include <type_traits>

namespace khepri {

/**
 * \brief A 2-component point.
 *
 * A point is a position in a Euclidian space. Unlike a \see vector, a point has no magnitude
 * or direction. As such, a point supports very few operations on its own. It can, however, be
 * combined with vectors (e.g. point + vector = point)
 *
 * \tparam ComponentT specifies the type the point's components (default = double).
 */
template <typename ComponentT = double>
class BasicPoint final
{
public:
    /// The type of the point's components
    using ComponentType = ComponentT;

    /// The x coordinate of this point
    ComponentType x;

    /// The y coordinate of this point
    ComponentType y;

    /// Constructs an uninitialized point
    BasicPoint() noexcept = default;

    /// Constructs the point \a x and \a y.
    BasicPoint(ComponentType x, ComponentType y) noexcept : x(x), y(y) {}

    constexpr BasicPoint& operator+=(const BasicVector2<ComponentType>& v) noexcept
    {
        x += v.x;
        y += v.y;
        return *this;
    }
};

/// Point of doubles
using Point = BasicPoint<double>;

/// Point of floats
using Pointf = BasicPoint<float>;

/// Point of (long) integers
using Pointi = BasicPoint<long>;

template <typename T, typename U>
constexpr bool operator==(const BasicPoint<T>& p1, const BasicPoint<U>& p2) noexcept
{
    return p1.x == p2.x && p1.y == p2.y;
}

template <typename T, typename U>
constexpr bool operator!=(const BasicPoint<T>& p1, const BasicPoint<U>& p2) noexcept
{
    return !(p1 == p2);
}

template <typename T, typename U>
inline auto operator-(const BasicPoint<T>& p1, const BasicPoint<U>& p2) noexcept
{
    return BasicVector2<long>(p1.x - p2.x, p1.y - p2.y);
}

template <typename T, typename U>
inline auto operator+(const BasicPoint<T>& p, const BasicVector2<U>& v) noexcept
{
    return BasicPoint<std::common_type_t<T, U>>{p.x + v.x, p.y + v.y};
}

} // namespace khepri
