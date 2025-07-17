#pragma once

#include "math.hpp"
#include "point.hpp"

#include <functional>

namespace khepri {

/**
 * @brief Generic constraint type.
 *
 * Constraints provide an interface to allow values to be constrained to certain rules.
 * A constraint takes an old and new value and returns the new value constrained to its rules.
 * This could be the new value, the old value, or another value entirely (e.g. a value in between).
 *
 * @tparam T the value type to constrain.
 */
template <typename T>
using Constraint = std::function<T(const T& old_value, const T& new_value)>;

/**
 * @brief Returns a constraint that constrains a value to a range.
 *
 * @param bounds_min the minimum value of the range
 * @param bounds_max the maximum value of the bound
 */
template <typename T>
Constraint<T> RangeConstraint(const T& bounds_min, const T& bounds_max)
{
    return [=](const T& old_value, const T& new_value) -> T {
        return clamp(new_value, bounds_min, bounds_max);
    };
}

/**
 * @brief Returns a constraint that constrains a 2D point to a rectangle.
 *
 * @param bounds_min the minimum coordinates of the bounds
 * @param bounds_max the maximum coordinates of the bounds
 */
template <typename T>
Constraint<BasicPoint<T>> RectangleConstraint(const BasicPoint<T>& bounds_min,
                                              const BasicPoint<T>& bounds_max)
{
    return [=](const BasicPoint<T>& old_value, const BasicPoint<T>& new_value) -> BasicPoint<T> {
        return {clamp(new_value.x, bounds_min.x, bounds_max.x),
                clamp(new_value.y, bounds_min.y, bounds_max.y)};
    };
}

} // namespace khepri