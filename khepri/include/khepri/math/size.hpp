#pragma once

namespace khepri {

/**
 * \brief Represents a discrete, two-dimensional size
 */
struct Size
{
    /// The width of the size
    unsigned long width;

    /// The height of the size
    unsigned long height;
};

inline constexpr bool operator==(const Size& s1, const Size& s2) noexcept
{
    return s1.width == s2.width && s1.height == s2.height;
}

inline constexpr bool operator!=(const Size& s1, const Size& s2) noexcept
{
    return !(s1 == s2);
}

} // namespace khepri