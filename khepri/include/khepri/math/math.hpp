#pragma once

#include <cmath>
#include <type_traits>

namespace khepri {

static constexpr double PI = 3.1415926535897932384626433832795;

/**
 * \brief Linearly interpolates between \a v0 and \a v1 based on factor \a t.
 *
 * \a t is assumed to be between 0 and 1. If \a t is 0, \a v0 is returned. If \a t is 1, \a v1 is
 * returned. It is valid for \a t to be outside of this range, in which case the result is an
 * extrapolation.
 */
template <typename T>
inline T lerp(const T& v0, const T& v1, double t) noexcept
{
    return T(v1 * t + v0 * (1.0 - t));
}

/**
 * \brief Clamps a value between two extremes
 *
 * Returns \a min if \a val < \a min.
 * Returns \a max if \a val > \a max.
 * Otherwise, returns \a val.
 *
 * \param[in] val the value to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val clamped between \a min and \a max.
 */
template <typename T, typename U>
constexpr T clamp(const T& val, const U& min, const U& max) noexcept
{
    if (val <= min) {
        return T{min};
    }
    if (val >= max) {
        return T{max};
    }
    return val;
}

/**
 * \brief Clamps a value between 0 and 1
 *
 * Returns 0 if \a val < 0.
 * Returns 1 if \a val > 1.
 * Otherwise, returns \a val.
 *
 * \param[in] val the value to clamp
 *
 * \return \a val clamped between 0 and 1.
 */
template <typename T>
constexpr T saturate(const T& val) noexcept
{
    return clamp<T>(val, T{0}, T{1});
}

/// Converts degrees to radians
template <typename T>
constexpr T to_radians(const T& degrees) noexcept
{
    constexpr auto DEGREES_PER_PI_RADIANS = 180;
    static_assert(std::is_floating_point_v<T>);
    return degrees / DEGREES_PER_PI_RADIANS * T{PI};
}

/// Converts radians to degrees
template <typename T>
constexpr T to_degrees(const T& radians) noexcept
{
    constexpr auto DEGREES_PER_PI_RADIANS = 180;
    static_assert(std::is_floating_point_v<T>);
    return radians / T{PI} * DEGREES_PER_PI_RADIANS;
}

/// Returns whether the difference between \a lhs and \a rhs is at most \a abs_error
[[nodiscard]] inline bool near(float lhs, float rhs, float abs_error) noexcept
{
    return std::abs(lhs - rhs) <= abs_error;
}

} // namespace khepri
