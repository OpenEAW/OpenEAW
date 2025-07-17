#pragma once

namespace khepri {

/**
 * \brief A one-dimensional range
 *
 * A range is defined by a minimum and maximum value, that define the (inclusive) extends of the
 * range.
 *
 * \tparam ComponentT specifies the type of the range's components (default = double).
 */
template <typename ComponentT = double>
struct BasicRange
{
    /// Lower bound of the range (inclusive)
    ComponentT min;

    /// Upper bound of the range (inclusive)
    ComponentT max;
};

/// Point of doubles
using Range = BasicRange<double>;

/// Range of floats
using Rangef = BasicRange<float>;

/// Range of (long) integers
using Rangei = BasicRange<long>;

template <typename T, typename U>
constexpr bool operator==(const BasicRange<T>& p1, const BasicRange<U>& p2) noexcept
{
    return p1.min == p2.min && p1.max == p2.max;
}

template <typename T, typename U>
constexpr bool operator!=(const BasicRange<T>& p1, const BasicRange<U>& p2) noexcept
{
    return !(p1 == p2);
}

} // namespace khepri
